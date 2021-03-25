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

#include "socket.h"

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <memory>

#include "exception.h"

namespace sock {

Socket::Socket(Type type):
descriptor_(0),
is_moved_(false) {
  int real_type = 0;
  switch (type) {
    case Type::kTcp:
      real_type = SOCK_STREAM;
      break;
    case Type::kUdp:
      real_type = SOCK_DGRAM;
      break;
    default:
      real_type = 0;
  }

  descriptor_ = socket(AF_INET, real_type, 0);
  if (descriptor_ == -1) {
    throw SocketException("Can't create socket");
  }
}

Socket::Socket(int descriptor) :
descriptor_(descriptor),
is_moved_(false) {
}

Socket::Socket(Socket &&other) :
descriptor_(other.descriptor_),
is_moved_(false) {
  other.is_moved_ = true;
}

Socket::~Socket() {
  if (!is_moved_) {
    close(descriptor_);
  }
}

int Socket::GetDescriptor() const {
  return descriptor_;
}

std::string Socket::GetPeerName() const {
  sockaddr_in peer_addr;
  int peer_len = sizeof(peer_addr);
  int res = getpeername(descriptor_, reinterpret_cast<sockaddr *>(&peer_addr),
                        reinterpret_cast<socklen_t *>(&peer_len));
  if (res != 0) {
    return "";
  }

  return inet_ntoa(peer_addr.sin_addr);
}

std::string Socket::Read(int n) {
  auto buf_ptr = std::make_unique<char[]>(n);
  int res = recv(descriptor_, buf_ptr.get(), n, 0);

  if (res < 0) {
    throw ReadError(strerror(errno));
  }
  if (res == 0 && n != 0) {
    throw ReadError("Socket is closed");
  }

  return buf_ptr.get();
}

void Socket::Send(std::string_view str) {
  if (send(descriptor_, str.data(), str.length(), 0) < 0) {
    throw SendError(strerror(errno));
  }
}

Socket &Socket::operator=(Socket &&other) {
  descriptor_ = other.descriptor_;
  other.is_moved_ = true;

  return *this;
}

Socket &operator<<(Socket &socket, std::ostream &(*f)(std::ostream &)) {
  if (f == &(std::endl<char, std::char_traits<char>>)) {
    socket.Send(socket.ss_buffer_.str());
    socket.ss_buffer_.str("");
    socket.ss_buffer_.clear();
  } else {
    f(socket.ss_buffer_);
  }

  return socket;
}

} // namespace sock
