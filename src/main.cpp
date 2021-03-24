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
#include <vector>
#include <future>

#include "camera.h"
#include "sock/server_socket.h"
#include "sock/exception.h"
#include "processing/request_dispatcher.h"
#include "processing/servlets/jpeg.h"

namespace {

volatile bool stop_flag = false;

void SignalHandler(int) {
  stop_flag = true;
}

processing::RequestDispatcher BuildRequestDispatcher() {
  processing::RequestDispatcher request_dispatcher;

  request_dispatcher.RegisterServlet(
      "/jpeg",
      std::make_shared<processing::servlets::Jpeg>()
  );

  return request_dispatcher;
}

void HandleClient(const processing::RequestDispatcher &dispatcher,
                  std::shared_ptr<sock::Socket> socket_ptr) {
  try {
    bool close_connection = false;
    while (!close_connection) {
      rtsp::Response response;
      rtsp::Request request;

      try {
        *socket_ptr >> request;
        std::cout << "Request:\n" << request << std::endl;
        response = dispatcher.Dispatch(request);
      } catch (const rtsp::ParseError &ex) {
        std::cout << "Can't parse request: " << ex.what() << std::endl;
        response = {400, "Bad Request"};
      }

      *socket_ptr << response << std::endl;
      std::cout << "\nResponse:\n" << response << std::endl;
      if (request.method == rtsp::Method::kTeardown) {
        close_connection = true;
      }
    }
  } catch (const sock::ReadError &ex) {
    std::cout << "Client on socket " << socket_ptr->GetDescriptor()
              << " disconnected" << std::endl;
  }
  std::cout << "Socket " << socket_ptr->GetDescriptor()
            << " closed" << std::endl;
}

} // namespace

int main(int /*argc*/, char **/*argv*/) {
  try {
    constexpr int kRtspPortNumber = 5544;
    constexpr int kAcceptTimeout = 2;

    signal(SIGINT, SignalHandler);

    Camera::GetInstance(); // Initializing camera

    processing::RequestDispatcher dispatcher = BuildRequestDispatcher();
    std::vector<std::future<void>> futures;

    sock::ServerSocket server_socket(sock::Type::kTcp, kRtspPortNumber);
    std::cout << "Server started" << std::endl;
    while (!stop_flag) {
      std::optional<sock::Socket> socket_opt = server_socket.TryAccept(kAcceptTimeout);
      if (socket_opt.has_value()) {
        std::cout << "Connected client on socket "
                  << socket_opt.value().GetDescriptor() << std::endl;
        auto client_socket_ptr = std::make_shared<sock::Socket>(std::move(socket_opt.value()));
        futures.push_back(std::async(HandleClient, dispatcher, client_socket_ptr));
      }
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
