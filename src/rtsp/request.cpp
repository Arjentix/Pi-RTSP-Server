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

#include "request.h"

#include <cctype>

#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace {

/**
 * @brief Transform string to lower case
 *
 * @param str String to be transformed
 */
void TransformToLowerCase(std::string &str) {
  std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
}

/**
 * @brief Parse RTSP method from string
 * @throws rtsp::ParseError exception if can't recognize RTSP method
 *
 * @param method_str String with method
 * @return Extracted method
 */
rtsp::Method ParseMethod(std::string &&method_str) {
  rtsp::Method method;

  if (method_str == "DESCRIBE") {
    method = rtsp::Method::kDescribe;
  } else if (method_str == "ANNOUNCE") {
    method = rtsp::Method::kAnnounce;
  } else if (method_str == "GET_PARAMETER") {
    method = rtsp::Method::kGetParameter;
  } else if (method_str == "OPTIONS") {
    method = rtsp::Method::kOptions;
  } else if (method_str == "PAUSE") {
    method = rtsp::Method::kPause;
  } else if (method_str == "PLAY") {
    method = rtsp::Method::kPlay;
  } else if (method_str == "RECORD") {
    method = rtsp::Method::kRecord;
  } else if (method_str == "SETUP") {
    method = rtsp::Method::kSetup;
  } else if (method_str == "SET_PARAMETER") {
    method = rtsp::Method::kSetParameter;
  } else if (method_str == "TEARDOWN") {
    method = rtsp::Method::kTeardown;
  } else {
      throw rtsp::ParseError("Unknown method " + method_str);
  }

  return method;
}

/**
 * @brief Parse RTSP header from string
 *
 * @param header_str String with header
 * @return pair with header name and header value
 */
std::pair<std::string, std::string> ParseHeader(std::string &&header_str) {
  std::istringstream header_iss(header_str);
  std::string header_name;
  std::string header_value;

  std::getline(header_iss, header_name, ':');
  header_iss.ignore(1);
  header_iss >> header_value;

  return {header_name, header_value};
}

/**
 * @brief Parse RTSP request from string
 * @throws rtsp::ParseError if some error occurred during parsing
 *
 * @param request_str String with request
 * @return Extracted request
 */
rtsp::Request ParseRequest(std::string &&request_str) {
  rtsp::Request request;
  std::istringstream iss(std::move(request_str));

  std::string method_str;
  iss >> method_str;
  request.method = ParseMethod(std::move(method_str));

  iss >> request.url;

  iss.ignore(1);
  std::string protocol;
  std::getline(iss, protocol, '/');
  if (protocol != "RTSP") {
    throw rtsp::ParseError("Expected RTSP protocol, but got " + protocol);
  }

  iss >> request.version;

  iss.ignore(2, '\n');
  std::string line;
  while (std::getline(iss, line) && line != "\r") {
    request.headers.insert(ParseHeader(std::move(line)));
  }

  request.body = iss.str().substr(iss.tellg());

  return request;
}

/**
 * @brief Extract value of "Content-Length" header
 *
 * @param request Request where header will be searched
 * @return Extracted value if header exists
 * @return 0 in other case
 */
int ExtractContentLength(const rtsp::Request &request) {
  int content_length = 0;

  const std::string kContentLengthHeader = "Content-Length";
  if (request.headers.count(kContentLengthHeader)) {
    content_length = stoi(request.headers.at(kContentLengthHeader));
  }

  return content_length;
}

} // namespace

namespace rtsp {

ParseError::ParseError(std::string_view message) :
std::runtime_error(message.data()) {}


std::string MethodToString(Method method) {
  std::string method_str;

  switch (method) {
    case Method::kDescribe:
      method_str = "DESCRIBE";
      break;
    case Method::kAnnounce:
      method_str = "ANNOUNCE";
      break;
    case Method::kGetParameter:
      method_str = "GET_PARAMETER";
      break;
    case Method::kOptions:
      method_str = "OPTIONS";
      break;
    case Method::kPause:
      method_str = "PAUSE";
      break;
    case Method::kPlay:
      method_str = "PLAY";
      break;
    case Method::kRecord:
      method_str = "RECORD";
      break;
    case Method::kSetup:
      method_str = "SETUP";
      break;
    case Method::kSetParameter:
      method_str = "SET_PARAMETER";
      break;
    case Method::kTeardown:
      method_str = "TEARDOWN";
      break;
    default:
      method_str = "UNKNOWN METHOD";
  }

  return method_str;
}


std::size_t HeaderNameHash::operator()(std::string header_name) const {
  TransformToLowerCase(header_name);
  return std::hash<std::string>()(header_name);
}

bool HeaderNameEqual::operator()(std::string lhs, std::string rhs) const {
  TransformToLowerCase(lhs);
  TransformToLowerCase(rhs);
  return (lhs == rhs);
}

std::ostream &operator<<(std::ostream &os, const Request::Headers &headers) {
  for (const auto &[key, value] : headers) {
    os << key << ": " << value << "\r\n";
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const Request &request) {
  os << MethodToString(request.method) << " " << request.url << " RTSP/"
     << std::fixed << std::setprecision(1) << request.version << "\r\n"
     << request.headers << "\r\n\r\n"
     << request.body;

  return os;
}

sock::Socket &operator>>(sock::Socket &socket, Request &request) {
  std::string request_str;
  while (request_str.rfind("\r\n\r\n") == std::string::npos) {
    constexpr int kBuffSize = 1024;
    request_str = socket.Read(kBuffSize);
  }

  request = ParseRequest(std::move(request_str));

  const int content_length = ExtractContentLength(request);
  const int diff = content_length - request.body.size();
  if (diff > 0) {
    request.body += socket.Read(diff);
  }

  return socket;
}

} // namespace rtsp
