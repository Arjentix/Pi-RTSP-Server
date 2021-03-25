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
#include <sstream>
#include <ostream>

namespace sock {

/**
 * @brief Possible socket types
 */
enum class Type {
  kTcp,
  kUdp
};

/**
 * @brief Linux socket wrapper
 */
class Socket {
 public:
  /**
   * @brief Construct a new Socket object
   *
   * @param type Type of the Socket
   */
  explicit Socket(Type type);

  /**
   * @brief Construct a new Socket object using provided descriptor
   *
   * @param descriptor Valid socket descriptor
   */
  explicit Socket(int descriptor);

  Socket(Socket &&other);

  virtual ~Socket();

  /**
   * @brief Get socket descriptor
   * @return descriptor
   */
  int GetDescriptor() const;

  /**
   * @brief Get ip address of the peer
   * @return String with ip address in success
   * @return Empty string in other way
   */
  std::string GetPeerName() const;

  /**
   * @brief Read n chars
   *
   * @param n number of chars to read. Default is 256
   * @return String with n chars
   */
  std::string Read(int n = 256);

  /**
   * @brief Send string
   *
   * @param str string to be sent
   */
  void Send(std::string_view str);

  Socket &operator=(Socket &&other);

 protected:
  int descriptor_; //!< Socket descriptor

 private:
  template <typename T>
  friend Socket &operator<<(Socket &socket, const T &obj);

  friend Socket &operator<<(Socket &socket, std::ostream &(*f)(std::ostream &));

  bool is_moved_; //!< True, if Socket was moved
  std::ostringstream ss_buffer_; //!< Buffer for operator<<
};

/**
 * @brief Output operator for socket
 * @details Store everything in buffer until std::endl is passed
 *
 * @tparam Any type that can be printed into a std::ostringstream
 * @param socket Socket that will store info
 * @param obj Object to be printed
 * @return Provided socket
 */
template <typename T>
Socket &operator<<(Socket &socket, const T &obj) {
  socket.ss_buffer_ << obj;
  return socket;
}

/**
 * @brief Output operator for std::endl and etc.
 * @details If std::endl is passed, than internal buffer will be sent to the socket
 *
 * @param socket Socket, which buffer will be processed
 * @param f Pointer to function that process ostream object
 * @return Provided socket
 */
Socket &operator<<(Socket &socket, std::ostream &(*f)(std::ostream &));

} // namespace sock
