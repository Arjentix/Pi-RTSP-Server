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

#include "server_socket.h"

#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "exception.h"

namespace sock {

ServerSocket::ServerSocket(Type type, int port_number) :
Socket(type) {
  if (fcntl(descriptor_, F_SETFL, fcntl(descriptor_, F_GETFL, 0) | O_NONBLOCK) < 0) {
    throw ServerSocketException("Can't set non blocking option for socket");
  }

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(port_number);

  int opt = 1;
  setsockopt(descriptor_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(descriptor_, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
    throw BindError(std::string("Can't bind socket: ") + strerror(errno));
  }

  if ((type == Type::kTcp) && listen(descriptor_, 1) < 0) {
    throw ListenError(std::string("Listen: ") + strerror(errno));
  }
}

std::optional<Socket> ServerSocket::TryAccept(int sec) const {
  fd_set inputs;
  timeval timeout;
  FD_ZERO(&inputs);
  FD_SET(descriptor_, &inputs);

  timeout.tv_sec = sec;
  timeout.tv_usec = 0;

  int select_res = select(FD_SETSIZE, &inputs, nullptr, nullptr, &timeout);
  if (select_res <= 0) {
    return std::nullopt;
  }

  int client_descriptor = accept(descriptor_, nullptr, nullptr);
  if (client_descriptor < 0) {
    throw AcceptError(std::string("Can't accept client: ") + strerror(errno));
  }

  return Socket(client_descriptor);
}

} // namespace sock
