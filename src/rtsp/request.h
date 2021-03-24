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

#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <ostream>

#include "sock/socket.h"

namespace rtsp {

/**
 * @brief Exception, indicating that an error occurred during request parsing
 */
class ParseError : public std::runtime_error {
 public:
  ParseError(std::string_view message);
};

/**
 * @brief All possible Client -> Server methods
 */
enum class Method {
  kDescribe,
  kAnnounce,
  kGetParameter,
  kOptions,
  kPause,
  kPlay,
  kRecord,
  kSetup,
  kSetParameter,
  kTeardown
};

/**
 * @brief Converts method to string
 *
 * @param method Method to be converted
 * @return str with method name
 */
std::string MethodToString(Method method);

/**
 * @brief Specific hasher for header names to provide case-insensitivity
 */
struct HeaderNameHash {
  std::size_t operator()(std::string header_name) const;
};

/**
 * @brief Specific equal for header names to provide case-insensitivity
 */
struct HeaderNameEqual {
  bool operator()(std::string lhs, std::string rhs) const;
};

/**
 * @brief Request from Client to Server
 */
struct Request {
  using Headers = std::unordered_map<std::string, std::string,
                                     HeaderNameHash, HeaderNameEqual>;

  Method method;
  std::string url;
  float version;
  Headers headers;
  std::string body;
};

std::ostream &operator<<(std::ostream &os, const Request::Headers &headers);

std::ostream &operator<<(std::ostream &os, const Request &request);

sock::Socket &operator>>(sock::Socket &socket, Request &request);

} // namespace rtsp
