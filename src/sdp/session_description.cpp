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

#include "session_description.h"

namespace {

//! SDP line ending characters
const char kCrLf[] = "\r\n";

/**
 * @brief Write SDP key and value to the stream
 *
 * @param os Stream to write into
 * @param key SDP key character
 * @param value SDP value
 */
void Write(std::ostream &os, char key, const std::string &value) {
  os << key << "=" << value << kCrLf;
}

/**
 * @brief Check if value is not empty and write it to the stream
 *
 * @param os Stream to write into
 * @param key SDP key character
 * @param value SDP value
 */
void CheckAndWrite(std::ostream &os, char key, const std::string &value) {
  if (value != "") {
    Write(os, key, value);
  }
}

/**
 * @brief For every value in vector check if value is not empty and write it to the stream
 *
 * @param os Stream to write into
 * @param key SDP key character
 * @param values Vector of SDP values
 */
void CheckAndWrite(std::ostream &os, char key, const std::vector<std::string> &values) {
  for (const std::string &value : values) {
    CheckAndWrite(os, key, value);
  }
}

} // namespace

namespace sdp {

std::ostream &operator<<(std::ostream &os, const TimeDescription &time_description) {
  os << "t=" << time_description.active_time.first << " "
     << time_description.active_time.second << kCrLf;

  if (time_description.repeat) {
    os << "r=" << time_description.repeat.value() << kCrLf;
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const MediaDescription &media_description) {
  Write(os, 'm', media_description.name);

  CheckAndWrite(os, 'i', media_description.info);
  CheckAndWrite(os, 'c', media_description.connection);
  CheckAndWrite(os, 'b', media_description.bandwidths);
  CheckAndWrite(os, 'k', media_description.key);
  CheckAndWrite(os, 'a', media_description.attributes);

  return os;
}

std::ostream &operator<<(std::ostream &os, const SessionDescription &session_description) {
  Write(os, 'v', std::to_string(session_description.version));
  Write(os, 'o', session_description.originator_and_session_id);
  Write(os, 's', session_description.session_name);

  CheckAndWrite(os, 'i', session_description.info);
  CheckAndWrite(os, 'u', session_description.uri);
  CheckAndWrite(os, 'e', session_description.emails);
  CheckAndWrite(os, 'p', session_description.phones);
  CheckAndWrite(os, 'c', session_description.connection);
  CheckAndWrite(os, 'b', session_description.bandwidths);

  for (const TimeDescription &time_description : session_description.time_descriptions) {
    os << time_description;
  }

  CheckAndWrite(os, 'z', session_description.time_zone);
  CheckAndWrite(os, 'k', session_description.key);
  CheckAndWrite(os, 'a', session_description.attributes);

  for (const MediaDescription &media_description : session_description.media_descriptions) {
    os << media_description;
  }

  return os;
}

} // namespace sdp
