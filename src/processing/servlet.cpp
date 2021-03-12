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

#include "servlet.h"

namespace {

const rtsp::Response kMethodNotAllowed = {405, "Method Not Allowed"};

} // namespace

namespace processing {

Servlet::~Servlet() {}

std::unordered_set<rtsp::Method> Servlet::GetAcceptableMethods() const {
  return acceptable_methods_;
}

rtsp::Response Servlet::ServeDescribe(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServeAnnounce(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServeGetParameter(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServePause(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServePlay(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServeRecord(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServeSetup(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServeSetParameter(const rtsp::Request &) {
  return kMethodNotAllowed;
}

rtsp::Response Servlet::ServeTeardown(const rtsp::Request &) {
  return kMethodNotAllowed;
}

void Servlet::AddMethod(rtsp::Method method) {
  acceptable_methods_.insert(method);
}

} // namespace processing
