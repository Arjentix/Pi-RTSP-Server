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
#include <regex>
#include <iostream>

#include "handlers/options.h"

namespace {

using ServletMethod = rtsp::Response (processing::Servlet::*)(const rtsp::Request &);

const char kCSeq[] = "CSeq";

/**
 * @brief Exception, that indicates that invalid url was received
 */
class BadUrl : public std::runtime_error {
 public:
  BadUrl(std::string_view message) :
  std::runtime_error(message.data()) {}
};

/**
 * @brief Extract path from url
 *
 * @param full_url A full url with protocol, hostname and etc.
 * @return Path from the url
 */
std::string ExtractPath(const std::string &full_url) {
  const uint kGroupsNumber = 7;
  const std::string scheme = R"(^(\S+)://)";
  const std::string login = R"(([^:\s]+)?)";
  const std::string password = R"((\S+))";
  const std::string login_password = R"((?:)" + login + R"((?::)" + password + R"()?@)?)";
  const std::string hostname = R"(([^:]+))";
  const std::string port = R"((?::([0-9]+))?)";
  const std::string path = R"(((?:/[^/\s]+)+))";
  const std::regex reg_expr(scheme + login_password + hostname + port + path);
  std::smatch matches;

  if (!std::regex_search(full_url, matches, reg_expr) ||
      (matches.size() < kGroupsNumber)) {
    throw BadUrl("Bad url");
  }

  std::string path_value = matches[kGroupsNumber - 1].str();
  if (*(path_value.rend()) == '/') {
    path_value = path_value.substr(0, path_value.size() - 1);
  }

  return path_value;
}

ServletMethod ChooseServletMethod(rtsp::Method rtsp_method) {
  ServletMethod servlet_method = nullptr;

  switch (rtsp_method) {
    case rtsp::Method::kDescribe:
      servlet_method = &processing::Servlet::ServeDescribe;
      break;
    case rtsp::Method::kAnnounce:
      servlet_method = &processing::Servlet::ServeAnnounce;
      break;
    case rtsp::Method::kGetParameter:
      servlet_method = &processing::Servlet::ServeGetParameter;
      break;
    case rtsp::Method::kPause:
      servlet_method = &processing::Servlet::ServePause;
      break;
    case rtsp::Method::kPlay:
      servlet_method = &processing::Servlet::ServePlay;
      break;
    case rtsp::Method::kRecord:
      servlet_method = &processing::Servlet::ServeRecord;
      break;
    case rtsp::Method::kSetup:
      servlet_method = &processing::Servlet::ServeSetup;
      break;
    case rtsp::Method::kSetParameter:
      servlet_method = &processing::Servlet::ServeSetParameter;
      break;
    case rtsp::Method::kTeardown:
      servlet_method = &processing::Servlet::ServeTeardown;
      break;
    default:
      throw std::out_of_range("Unknown method");
  }

  return servlet_method;
}

std::string MethodsToString(const processing::handlers::Options::Methods &methods) {
  std::ostringstream oss;

  bool first = true;
  for (rtsp::Method method : methods) {
    if (!first) {
      oss << ", ";
    }
    oss << rtsp::MethodToString(method);

    first = false;
  }

  return oss.str();
}

} // namespace

namespace processing {

RequestDispatcher::RequestDispatcher() :
url_to_servlet_(),
acceptable_methods_({rtsp::Method::kOptions}),
acceptable_urls_() {}

RequestDispatcher &RequestDispatcher::RegisterServlet(
  const std::string &url, std::shared_ptr<Servlet> servlet_ptr) {
  auto res = url_to_servlet_.insert({std::move(url), servlet_ptr});
  if (res.second) {
    acceptable_urls_.insert(res.first->first);
    acceptable_methods_.merge(servlet_ptr->GetAcceptableMethods());
  }

  return *this;
}

rtsp::Response RequestDispatcher::Dispatch(rtsp::Request request) {
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
  if (!acceptable_methods_.count(request.method)) {
    response.code = 501;
    response.description = "Not Implemented";
    return response;
  }

  try {
    rtsp::Response::Headers old_response_headers = response.headers;

    if (request.method == rtsp::Method::kOptions) {
      response = GetOptions();
    } else {
      ServletMethod method = ChooseServletMethod(request.method);
      auto [path, servlet_ptr] = *ChooseServlet(request.url);
      request.url = ExtractPath(request.url).substr(path.size());
      response = (servlet_ptr.get()->*method)(request);
    }

    response.headers.merge(std::move(old_response_headers));
  }
  catch (const BadUrl &ex) {
    response.code = 400;
    response.description = "Bad Request";
  }
  catch (const std::out_of_range &ex) {
      response.code = 404;
      response.description = "Not Found";
  }
  catch (const std::runtime_error &ex) {
    response.code = 500;
    response.description = "Internal Server Error";
  }

  return response;
}

rtsp::Response RequestDispatcher::GetOptions() const {
  return {200, "OK",
    {
      {"Public", MethodsToString(acceptable_methods_)}
    }
  };
}

RequestDispatcher::UrlToServletMap::const_iterator RequestDispatcher::ChooseServlet(
  const std::string &url) const {
  if (url_to_servlet_.size() == 0) {
    throw std::out_of_range("There aren't any servlets at all");
  }

  const std::string path = ExtractPath(url);
  auto it = url_to_servlet_.lower_bound(path);
  if ((it != url_to_servlet_.end()) &&
      (it->first == path)) {
    return it;
  }

  --it;
  if (path.find(it->first) == 0) {
    return it;
  }

  throw std::out_of_range("Can't find suitable servlet");
}

} // namespace processing
