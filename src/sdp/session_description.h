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
#include <utility>
#include <vector>
#include <ostream>
#include <optional>

namespace sdp {

/**
 * @brief SDP attribute
 */
using Attribute = std::pair<std::string, std::string>;

std::ostream &operator<<(std::ostream &os, const Attribute &attribute);

/**
 * @brief Struct with Time Description of SDP format
 */
struct TimeDescription {
  std::pair<uint64_t , uint64_t> active_time; //!< Mandatory. Time the session is active
  std::optional<int> repeat; //!< Optional. Zero or more repeat times
};

std::ostream &operator<<(std::ostream &os, const TimeDescription &time_description);


/**
 * @brief Struct with Media description in SDP format
 */
struct MediaDescription {
  std::string name; //!< Media name and transport address

  // Optional fields
  std::string info; //!< Media title or information field
  std::string connection; //!< Connection information — optional if included at session level
  std::vector<std::string> bandwidths; //!< Zero or more bandwidth information lines
  std::string key; //!< Encryption key
  std::vector<Attribute> attributes; //!< Zero or more media attribute lines — overriding the Session attribute lines
};

std::ostream &operator<<(std::ostream &os, const MediaDescription &media_description);


/**
 * @brief Struct with Session description in SDP format
 */
struct SessionDescription {
  // Mandatory fields
  int version; //!< Protocol version number, currently only 0
  std::string originator_and_session_id; //!< Username, id, version number, network address
  std::string session_name; //!< Mandatory with at least one UTF-8-encoded character
  std::vector<TimeDescription> time_descriptions; //!< One or more Time descriptions

  // Optional fields
  std::string info; //!< Session title or short information
  std::string uri; //!< URI of description
  std::vector<std::string> emails; //!< Zero or more email address with optional name of contacts
  std::vector<std::string> phones; //!< Zero or more phone number with optional name of contacts
  std::string connection; //!< Connection information—not required if included in all media
  std::vector<std::string> bandwidths; //!< Zero or more bandwidth information lines
  std::string time_zone; //!< Time zone adjustments
  std::string key; //!< Encryption key
  std::vector<Attribute> attributes; //!< Zero or more session attribute lines
  std::vector<MediaDescription> media_descriptions; //!< Zero or more Media descriptions
};

std::ostream &operator<<(std::ostream &os, const SessionDescription &session_description);

} // namespace sdp
