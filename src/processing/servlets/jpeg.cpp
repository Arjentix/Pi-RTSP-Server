/*
MIT License

Copyright (c) 2021 Polyakov Daniil Alexandrovich

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "jpeg.h"

#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <chrono>

#include "sdp/session_description.h"
#include "camera.h"

namespace {

using namespace std::literals::string_literals;

/**
 * @brief Get IP address of this machine
 *
 * @return IP address on success
 * @return 0.0.0.0 in other way
 */
std::string GetIpAddr() {
  ifaddrs *if_addr_ptr = nullptr;
  getifaddrs(&if_addr_ptr);

  for (ifaddrs *ifa_iter = if_addr_ptr;
       ifa_iter != NULL;
       ifa_iter = ifa_iter->ifa_next) {
    if ((ifa_iter->ifa_addr != nullptr) &&
        (std::string(ifa_iter->ifa_name) == "wlan0") &&
        (ifa_iter->ifa_addr->sa_family == AF_INET)) {
      in_addr *in_addr_ptr = &(reinterpret_cast<struct sockaddr_in *>(ifa_iter->ifa_addr)->sin_addr);
      char address_buffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, in_addr_ptr, address_buffer, INET_ADDRSTRLEN);

      freeifaddrs(if_addr_ptr);
      return address_buffer;
    }
  }

  return "0.0.0.0";
}

/**
 * @brief Build SDP video media description with jpeg-encoding
 *
 * @param ip_address IP address of this machine to not to call expensive GetIpAddr()
 * @param track_name Name of the video tack
 * @return Video media description
 */
sdp::MediaDescription BuildMediaDescription(const std::string &ip_address, const std::string track_name) {
  const int kMediaFormatCode = 26; // Jpeg code

  sdp::MediaDescription media_descr;

  media_descr.name = "video 0 RTP/AVP "s + std::to_string(kMediaFormatCode);
  media_descr.connection = "IN IP4 "s + ip_address;

  media_descr.attributes.push_back({"control", track_name});

  const uint height = Camera::GetInstance().getHeight();
  const uint width = Camera::GetInstance().getWidth();
  media_descr.attributes.push_back({"cliprect",
                                    "0,0,"s + std::to_string(height) + "," + std::to_string(width)});

  media_descr.attributes.push_back({"framerate",
                                    std::to_string(Camera::GetInstance().getFrameRate())});

  return media_descr;
}

/**
 * @brief Build SDP session description, i.e. body of DESCRIBE rtsp response
 *
 * @param track_name Name of the video tack
 * @return Session description
 */
sdp::SessionDescription BuildSessionDescription(const std::string track_name) {
  const auto now = std::chrono::system_clock::now();
  const uint64_t kSessionId = std::chrono::duration_cast<std::chrono::seconds>(
      now.time_since_epoch()).count();
  const int kSessionVersion = 0;
  const std::string kIp = GetIpAddr();

  sdp::SessionDescription descr;
  descr.version = 0;
  descr.originator_and_session_id = std::string(getlogin()) + " " +
      std::to_string(kSessionId) + " " + std::to_string(kSessionVersion) +
      " IN IP4 " + kIp;
  descr.session_name = "Session streamed by Pi RTSP Server";
  descr.info = "jpeg";
  descr.time_descriptions.push_back(sdp::TimeDescription{{0, 0}, std::nullopt});

  sdp::MediaDescription media_descr = BuildMediaDescription(kIp, track_name);
  descr.media_descriptions.push_back(std::move(media_descr));

  return descr;
}

/**
 * @brief Extract client RTP and RTCP ports
 *
 * @param transport Value of Transport header
 * @return pair of RTP and RTCP ports in success
 * @return {0, 0} in other way
 */
std::pair<int, int> ExtractClientPorts(const std::string &transport) {
  const std::string kClientPortStr = "client_port=";
  std::istringstream iss(transport);
  std::string param;

  while (std::getline(iss, param, ';')) {
    uint pos = param.find(kClientPortStr);
    if (pos != param.npos) {
      std::istringstream ports_iss(param.substr(pos + kClientPortStr.size()));
      std::pair<int, int> ports;
      ports_iss >> ports.first;
      ports_iss.ignore(1);
      ports_iss >> ports.second;

      return ports;
    }
  }

  return {0, 0};
}

} // namespace

namespace processing::servlets {

Jpeg::Jpeg():
client_ports_(0, 0),
play_queue_(),
play_worker_(&Jpeg::PlayWorkerThread, this),
play_worker_stop_(false),
play_worker_mutex_(),
play_worker_notifier_() {
  AddMethod(rtsp::Method::kDescribe);
  AddMethod(rtsp::Method::kSetup);
  AddMethod(rtsp::Method::kPlay);
}

Jpeg::~Jpeg() {
  {
    std::lock_guard guard(play_worker_mutex_);
    play_worker_stop_ = true;
  }
  play_worker_notifier_.notify_one();
  play_worker_.join();
}

rtsp::Response Jpeg::ServeDescribe(const rtsp::Request &) {
  std::ostringstream oss;
  oss << BuildSessionDescription(kVideoTrackName);
  std::string descr_str = oss.str();

  return {200, "OK",
    {
        {"Content-Type", "application/sdp"},
        {"Content-Length", std::to_string(descr_str.length())}
    },
    std::move(descr_str)
  };
}

rtsp::Response Jpeg::ServeSetup(const rtsp::Request &request) {
  using namespace std::string_literals;

  const char kSessionHeader[] = "Session";
  const char kTransportHeader[] = "Transport";
  rtsp::Response response;

  if (request.url != "/"s + kVideoTrackName) {
    response.code = 404;
    response.description = "Not Found";
    return response;
  }

  if (request.headers.count(kSessionHeader) &&
      std::stoi(request.headers.at(kSessionHeader)) == kSessionId) {
    response.code = 459;
    response.description = "Aggregate Operation Not Allowed";
    return response;
  }

  response.code = 200;
  response.description = "OK";
  response.headers[kSessionHeader] = std::to_string(kSessionId);
  client_ports_ = ExtractClientPorts(request.headers.at(kTransportHeader));
  response.headers[kTransportHeader] = "RTP/AVP;unicast;"s + "client_port=" +
    std::to_string(client_ports_.first) + "-" +
    std::to_string(client_ports_.second) + ";server_port=" +
    std::to_string(kServerPorts.first) + "-" + std::to_string(kServerPorts.second);

  return response;
}

rtsp::Response Jpeg::ServePlay(const rtsp::Request &request) {
  {
    std::lock_guard guard(play_worker_mutex_);
    play_queue_.push(request);
  }
  play_worker_notifier_.notify_one();

  return {200, "OK",
    {
      {"Range", "0.000-"}
    }
  };
}

void Jpeg::PlayWorkerThread() {
  for (;;) {
    std::unique_lock lock(play_worker_mutex_);
    play_worker_notifier_.wait(lock,
      [&play_queue = play_queue_, &play_worker_stop = play_worker_stop_] {
        return (!play_queue.empty() || play_worker_stop);
      }
    );

    if (play_worker_stop_) {
      return;
    }

    rtsp::Request play_request = play_queue_.front();
    play_queue_.pop();
    lock.unlock();

    std::cout << "Processing PLAY request..." << std::endl;
    // ...
  }
}

} // namespace processing::servlets
