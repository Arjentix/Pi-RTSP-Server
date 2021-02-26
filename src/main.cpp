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

#include <cstdlib>
#include <csignal>

#include <iostream>

#include "processing/request_dispatcher.h"
#include "sock/server_socket.h"

namespace {

volatile bool stop_flag = false;

void SignalHandler(int) {
  stop_flag = true;
}

processing::RequestDispatcher BuildRequestDispatcher() {
  return processing::RequestDispatcher();
}

} // namespace

int main(int /*argc*/, char **/*argv*/) {
  try {
    signal(SIGINT, SignalHandler);

    processing::RequestDispatcher dispatcher = BuildRequestDispatcher();

    constexpr int kRtspPortNumber = 554;

    sock::ServerSocket server_socket(sock::Type::kTcp, kRtspPortNumber);
    std::cout << "Server started" << std::endl;
    while (!stop_flag) {
      sock::Socket socket = server_socket.Accept().first;
      rtsp::Request request;
      socket >> request;
      rtsp::Response response = dispatcher.Dispatch(request);
      socket << response << std::endl;
    }
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unknown error occurred" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
