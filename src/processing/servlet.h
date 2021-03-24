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

#include <unordered_set>

#include "rtsp/request.h"
#include "rtsp/response.h"

namespace processing {

/**
 * @brief Base Servlet class. Returns "405 Method Not Allowed" from every Serve-method
 */
class Servlet {
 public:
  virtual ~Servlet();

  /**
   * @brief Get all methods, that this Servlet can properly handle
   *
   * @return Set of acceptable RTSP methods
   */
  std::unordered_set<rtsp::Method> GetAcceptableMethods() const;

  /**
   * @brief Serve DESCRIBE RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServeDescribe(const rtsp::Request &request);

  /**
   * @brief Serve ANNOUNCE RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServeAnnounce(const rtsp::Request &request);

  /**
   * @brief Serve GET_PARAMETER RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServeGetParameter(const rtsp::Request &request);

  /**
   * @brief Serve PAUSE RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServePause(const rtsp::Request &request);

  /**
   * @brief Serve PLAY RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServePlay(const rtsp::Request &request);

  /**
   * @brief Serve RECORD RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServeRecord(const rtsp::Request &request);

  /**
   * @brief Serve SETUP RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServeSetup(const rtsp::Request &request);

  /**
   * @brief Serve SET_PARAMETER RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServeSetParameter(const rtsp::Request &request);

  /**
   * @brief Serve TEARDOWN RTSP request
   *
   * @return RTSP response to the request
   */
  virtual rtsp::Response ServeTeardown(const rtsp::Request &request);

 protected:
  /**
   * @brief Add RTSP method to acceptable ones
   *
   * @param method RTSP method, that can be properly handled
   */
  void AddMethod(rtsp::Method method);

 private:
  std::unordered_set<rtsp::Method> acceptable_methods_; //!< All acceptable methods
};

} // namespace processing
