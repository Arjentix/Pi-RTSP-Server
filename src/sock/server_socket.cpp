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
#include <arpa/inet.h>

#include "exception.h"

namespace sock {

ServerSocket::ServerSocket(Type type, int port_number):
Socket(type) {
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(port_number);

  if (bind(descriptor_, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
    throw BindError(strerror(errno));
  }

  if (listen(descriptor_, 1) < 0) {
    throw ListenError(strerror(errno));
  }
}

std::pair<Socket, std::string> ServerSocket::Accept() const {
  sockaddr_in client_addr;
  socklen_t size = sizeof(client_addr);
  int client_descriptor = accept(descriptor_, reinterpret_cast<sockaddr *>(&client_addr), &size);
  if (client_descriptor < 0) {
    throw AcceptError(strerror(errno));
  }

  return {Socket(client_descriptor), inet_ntoa(client_addr.sin_addr)};
}

} // namespace sock
