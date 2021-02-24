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
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "handler.h"
#include "rtsp/request.h"
#include "rtsp/response.h"

namespace processing {

/**
 * @brief Request parameters that distinguish requests from handlers point of view
 */
struct RequestParams {
  rtsp::Method method;
  std::string url;
};

bool operator==(const RequestParams &lhs, const RequestParams &rhs);

struct RequestParamsHash {
  std::size_t operator()(const RequestParams &params) const;
};


/**
 * @brief RequestDispatcher allows to register handlers and then dispatch RTSP
 * request to the qualified handler returning RTSP response
 */
class RequestDispatcher {
 public:
  /**
   * @brief Register new handler, that will process request with specified params
   *
   * @param params Unique request parameters
   * @param handler_ptr Pointer to the Handler inheritor
   */
  void RegisterHandler(const RequestParams &params,
                       std::shared_ptr<Handler> handler_ptr);

  /**
   * @brief Dispatch request to the specified handler and get a response
   *
   * @param request Request to dispatch
   * @return Response from handler if handler was found
   * @return Response with error in other way
   */
  rtsp::Response Dispatch(const rtsp::Request &request) const;

 private:
  //! Request params -> Handler inheritor
  std::unordered_map<RequestParams, std::shared_ptr<Handler>,
                     RequestParamsHash> params_to_handler_;
  std::unordered_set<rtsp::Method> acceptable_methods_; //!< All acceptable methods
  std::unordered_set<std::string_view> acceptable_urls_; //!< All acceptable urls
};

} // namespace processing
