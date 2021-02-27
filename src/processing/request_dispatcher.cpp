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

#include "request_dispatcher.h"

#include <utility>
#include <functional>

#include "handlers/options.h"

namespace {

const char kCSeq[] = "CSeq";

} // namespace

namespace processing {

bool operator==(const RequestParams &lhs, const RequestParams &rhs) {
  return (lhs.method == rhs.method) && (lhs.url == rhs.url);
}

std::size_t RequestParamsHash::operator()(const RequestParams &params) const {
  const std::size_t a = std::hash<int>()(static_cast<int>(params.method));
  const std::size_t b = std::hash<std::string>()(params.url);
  const std::size_t x = 31;

  return a*x + b;
}


RequestDispatcher::RequestDispatcher() :
params_to_handler_(),
acceptable_methods_(),
acceptable_urls_() {
  options_handler_ptr_ =
      std::make_shared<processing::handlers::Options>(acceptable_methods_);
  RegisterHandler({rtsp::Method::kOptions, "*"},
                  options_handler_ptr_);
}

void RequestDispatcher::RegisterHandler(const RequestParams &params,
                                        std::shared_ptr<Handler> handler_ptr) {
  auto res = params_to_handler_.insert({params, handler_ptr});
  acceptable_methods_.insert(params.method);
  if (res.second) {
    acceptable_urls_.insert(res.first->first.url);
  }
}

rtsp::Response RequestDispatcher::Dispatch(const rtsp::Request &request) {
  if (!request.headers.count(kCSeq)) {
    return {400, "Bad Request"};
  }

  rtsp::Response response = {
    0, "",
    {
      {kCSeq, request.headers.at(kCSeq)}
    }
  };

  if (request.version != 1.0) {
    response.code = 505;
    response.description = "RTSP Version not supported";
    return response;
  }

  try {
    rtsp::Response::Headers old_response_headers = response.headers;

    if (request.method == rtsp::Method::kOptions) {
      response = options_handler_ptr_->handle(request);
    } else {
      RequestParams params = {request.method, request.url};
      response = params_to_handler_.at(std::move(params))->handle(request);
    }

    response.headers.merge(std::move(old_response_headers));
  }
  catch (const std::out_of_range &ex) {
    if (!acceptable_methods_.count(request.method)) {
      response.code = 501;
      response.description = "Not Implemented";
    } else if (!acceptable_urls_.count(request.url)){
      response.code = 404;
      response.description = "Not Found";
    } else {
      response.code = 405;
      response.description = "Method Not Allowed";
    }
  }
  catch (const std::runtime_error &ex) {
    response.code = 500;
    response.description = "Internal Server Error";
  }

  return response;
}

} // namespace processing
