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

#include <iostream>
#include <chrono>
#include <random>
#include <jpeglib.h>

#include "sdp/session_description.h"
#include "camera.h"
#include "sock/exception.h"
#include "sock/client_socket.h"
#include "rtp/byte.h"
#include "rtp/mjpeg/packet.h"
#include "rtp/packet.h"

namespace {

using namespace std::literals::string_literals;

/**
 * @brief Build SDP video media description with jpeg-encoding
 *
 * @param ip_address IP address of this machine
 * @param track_name Name of the video tack
 * @return Video media description
 */
sdp::MediaDescription BuildMediaDescription(const std::string &ip_address,
                                            const std::string &track_name) {
  const int kMediaFormatCode = 26; // Jpeg code

  sdp::MediaDescription media_descr;

  media_descr.name = "video 0 RTP/AVP "s + std::to_string(kMediaFormatCode);
  media_descr.connection = "IN IP4 "s + ip_address;

  media_descr.attributes.emplace_back("control", track_name);

  const uint height = Camera::GetInstance().getHeight();
  const uint width = Camera::GetInstance().getWidth();
  media_descr.attributes.emplace_back(
      "cliprect",
      "0,0,"s + std::to_string(height) + "," + std::to_string(width));

  media_descr.attributes.emplace_back(
      "framerate",
      std::to_string(Camera::GetInstance().getFrameRate()));

  return media_descr;
}

/**
 * @brief Build SDP session description, i.e. body of DESCRIBE rtsp response
 *
 * @param track_name Name of the video tack
 * @return Session description
 */
sdp::SessionDescription BuildSessionDescription(const std::string &track_name) {
  const auto now = std::chrono::system_clock::now();
  const uint64_t kSessionId = std::chrono::duration_cast<std::chrono::seconds>(
      now.time_since_epoch()).count();
  const int kSessionVersion = 1;
  const std::string kIp = "0.0.0.0";

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
    if (pos != std::string::npos) {
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

/**
 * @brief Convert raw image data to jpeg data
 *
 * @param raw_image Pointer to raw image data
 * @param width Image width
 * @param height Image height
 * @param quality Quality of resulting image in [0, 100] range
 * @return Jpeg image in bytes
 */
rtp::Bytes ConvertToJpeg(JSAMPLE *raw_image, const int width, const int height,
                         const int quality) {
  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;
  unsigned char *buffer = nullptr;
  unsigned long buffer_size = 0;

  JSAMPROW row_pointer[1];	// pointer to JSAMPLE row[s]
  int row_stride;		    // physical row width in image buffer

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  jpeg_mem_dest(&cinfo, &buffer, &buffer_size);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;		// # of color components per pixel
  cinfo.in_color_space = JCS_RGB; 	// colorspace of input image

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = width * 3;	// JSAMPLEs per row in image_buffer

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = & raw_image[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  rtp::Bytes res(buffer, buffer + buffer_size);
  free(buffer);
  return res;
}

/**
 * @brief Grab image from camera in jpeg format
 *
 * @param quality Quality of resulting image in [0, 100] range
 * @return Jpeg image in bytes
 */
rtp::Bytes GrabImage(const int quality) {
  raspicam::RaspiCam &camera = Camera::GetInstance();
  camera.grab();
  auto raw_image_ptr = std::make_unique<unsigned char[]>(
      camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB));
  camera.retrieve(raw_image_ptr.get());
  return ConvertToJpeg(raw_image_ptr.get(), camera.getWidth(),
                       camera.getHeight(), quality);
}

} // namespace

namespace processing::servlets {

Jpeg::Jpeg() :
client_connected_(false),
teardown_(false),
session_id_(0),
client_ports_(0, 0),
play_queue_(),
play_worker_(&Jpeg::PlayWorkerThread, this),
play_worker_stop_(false),
play_worker_mutex_(),
play_worker_notifier_() {
  AddMethod(rtsp::Method::kDescribe);
  AddMethod(rtsp::Method::kSetup);
  AddMethod(rtsp::Method::kPlay);
  AddMethod(rtsp::Method::kTeardown);
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
          descr_str
  };
}

rtsp::Response Jpeg::ServeSetup(const rtsp::Request &request) {
  using namespace std::string_literals;

  const char kSessionHeader[] = "Session";
  const char kTransportHeader[] = "Transport";
  rtsp::Response response;

  if (request.url != "/"s + kVideoTrackName) {
    return {404, "Not Found"};
  }

  if (request.headers.count(kSessionHeader) &&
      std::stoul(request.headers.at(kSessionHeader)) == session_id_) {
    return {459, "Aggregate Operation Not Allowed"};
  }

  if (client_connected_) {
    return {423, "Locked"};
  }

  std::random_device rd;
  std::mt19937 mersenne(rd());
  session_id_ = mersenne();

  response.code = 200;
  response.description = "OK";
  response.headers[kSessionHeader] = std::to_string(session_id_);
  client_ports_ = ExtractClientPorts(request.headers.at(kTransportHeader));
  response.headers[kTransportHeader] = "RTP/AVP;unicast;"s + "client_port=" +
      std::to_string(client_ports_.first) + "-" +
      std::to_string(client_ports_.second) + ";server_port=" +
      std::to_string(kServerPorts.first) + "-" + std::to_string(kServerPorts.second);

  return response;
}

rtsp::Response Jpeg::ServePlay(const rtsp::Request &request) {
  if (!CheckSession(request)) {
    return {454, "Session Not Found"};
  }

  client_connected_ = true;
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

rtsp::Response Jpeg::ServeTeardown(const rtsp::Request &request) {
  if (!CheckSession(request)) {
    return {454, "Session Not Found"};
  }

  {
    std::lock_guard guard(play_worker_mutex_);
    teardown_ = true;
    client_connected_ = false;
  }
  play_worker_notifier_.notify_one();

  return {200, "OK"};
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

    HandlePlayRequest(play_request);
  }
}

void Jpeg::HandlePlayRequest(const rtsp::Request &request) {
  std::cout << "Processing PLAY request..." << std::endl;

  const std::string kClientAddr = request.client_ip + ":" +
      std::to_string(client_ports_.first);
  sock::ClientSocket socket(sock::Type::kUdp);
  if (!socket.Connect(request.client_ip, client_ports_.first)) {
    std::cout << "Can't connect to the RTP client " << kClientAddr << std::endl;
    return;
  }
  std::cout << "Connected with the RTP client " << kClientAddr << std::endl;

  try {
    for (;;) {
      {
        std::lock_guard guard(play_worker_mutex_);
        if (teardown_) {
          teardown_ = false;
          break;
        }
      }

      const int kQuality = 50; // 0 - 100 %
      rtp::Bytes jpeg_image = GrabImage(kQuality);
      std::cout << "Jpeg image size: " << jpeg_image.size() << std::endl;

      std::vector<rtp::mjpeg::Packet> mjpeg_packets =
          rtp::mjpeg::PackJpeg(jpeg_image, kQuality);
      std::cout << "Packed in " << mjpeg_packets.size() << " MJPEG packets" << std::endl;
//
//    for (auto it = mjpeg_packets.begin(); it != mjpeg_packets.end(); ++it) {
//      const bool final = (it == std::prev(mjpeg_packets.end()));
//      rtp::Packet rtp_packet = rtp::mjpeg::PackToRtpPacket(*it, final);
//      socket << rtp_packet << std::endl;
//    }
    }
  } catch (sock::SocketException &ex) {
    std::cout << "Some error occurred during RTP packets translating: "
              << ex.what() << std::endl;
  }

  std::cout << "Disconnecting RTP client " << kClientAddr << std::endl;
}

bool Jpeg::CheckSession(const rtsp::Request &request) const {
  const char kSessionHeader[] = "Session";
  return (request.headers.count(kSessionHeader) &&
          (std::stoul(request.headers.at(kSessionHeader)) == session_id_));
}

} // namespace processing::servlets
