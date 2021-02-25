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
  Socket(Type type);

  Socket(Socket&& other);

  virtual ~Socket();


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
   * @param str string to be sended
   */
  void Send(std::string_view str);


  Socket &operator=(Socket &&other);

 private:
  int descriptor_; //!< Socket descriptor
  bool is_moved_; //!< True, if Socket was moved
};

} // namespace sock
