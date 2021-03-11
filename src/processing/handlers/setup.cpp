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

#include "setup.h"

#include <utility>
#include <sstream>

namespace {

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

namespace processing::handlers {

rtsp::Response Setup::Handle(const rtsp::Request &request) {
  using namespace std::string_literals;

  const char kSessionHeader[] = "Session";
  const char kTransportHeader[] = "Transport";
  rtsp::Response response;

  if (request.headers.count(kSessionHeader) &&
      std::stoi(request.headers.at(kSessionHeader)) == kSessionId) {
    response.code = 459;
    response.description = "Aggregate Operation Not Allowed";
    return response;
  }

  response.code = 200;
  response.description = "OK";
  response.headers[kSessionHeader] = std::to_string(kSessionId);
  auto client_ports = ExtractClientPorts(request.headers.at(kTransportHeader));
  response.headers[kTransportHeader] = "RTP/AVP;unicast;"s + "client_port=" +
    std::to_string(client_ports.first) + "-" +
    std::to_string(client_ports.second) + ";server_port=" +
    std::to_string(kServerPort) + "-" + std::to_string(kServerPort + 1);

  return response;
}

} // namespace processing::handlers
