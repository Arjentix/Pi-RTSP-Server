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
#include <signal.h>

#include <iostream>

#include "rtsp/server.h"
#include "processing/request_dispatcher.h"

namespace {

volatile bool stop_flag = false;

void SignalHandler(int) {
  stop_flag = true;
}

RequestDispatcher BuildRequestDispatcher() {
  return RequestDispatcher();
}

} // namespace

int main(int argc, char **argv) {
  try {
    signal(SIGINT, SignalHandler);

    RequestDispatcher dispatcher = BuildRequestDispatcher();

    constexpr int kRtspPortNumber = 554;
    rtsp::Server server(kRtspPortNumber);

    std::cout << "Server started" << std::endl;
    while (!stop_flag) {
      rtsp::ClientConnection client = server.ConnectClient();
      rtsp::Response response = dispatcher.dispatch(client.GetRequest());
      client.SendResponce(response);
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
