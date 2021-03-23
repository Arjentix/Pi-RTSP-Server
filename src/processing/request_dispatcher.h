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
#include <string_view>
#include <map>
#include <unordered_set>
#include <memory>

#include "servlet.h"

namespace processing {

/**
 * @brief RequestDispatcher allows to register handlers and then dispatch RTSP
 * request to the qualified handler returning RTSP response
 */
class RequestDispatcher {
 public:
  RequestDispatcher();

  /**
   * @brief Register new servlet, that will process request on specified url
   *
   * @param url Unique url
   * @param servlet_ptr Pointer to the Servlet inheritor
   * @return Reference to this for chaining
   */
  RequestDispatcher &RegisterServlet(const std::string &url,
                                     std::shared_ptr<Servlet> servlet_ptr);

  /**
   * @brief Dispatch request to the specified servlet and get a response
   * @details Must specific servlet will be chosen according to the url
   *
   * @param request Request to dispatch
   * @return Response from servlet if such was found
   * @return Response with error in other way
   */
  rtsp::Response Dispatch(rtsp::Request request) const;

 private:
  using UrlToServletMap = std::map<std::string, std::shared_ptr<Servlet>>;

  //! Url -> Servlet inheritor
  UrlToServletMap url_to_servlet_;
  std::unordered_set<rtsp::Method> acceptable_methods_; //!< All acceptable methods
  std::unordered_set<std::string_view> acceptable_urls_; //!< All acceptable urls

  /**
   * @brief Get response on OPTIONS request
   *
   * @return Response with acceptable methods
   */
  rtsp::Response GetOptions() const;

  /**
   * @brief Choose proper Servlet to serve request on the given url
   *
   * @param url Request url
   * @return Iterator, pointing on {Servlet url, Servlet} pair
   */
  UrlToServletMap::const_iterator ChooseServlet(const std::string &url) const;
};

} // namespace processing
