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

#include <cstdint>

#include <array>
#include <ostream>

#include "byte.h"

namespace rtp {

/**
 * @brief An RTP header
 */
struct Header {
  //! Max number of contributing sources
  static const std::size_t kContributingSourcesMaxCount = 15;

  unsigned int version: 2; //!< RTP version
  unsigned int padding: 1; //!< Padding bit
  unsigned int extension: 1; //!< Extension bit
  //!< Number of CSRC identifiers. Equals to contributing_sources.size()
  unsigned int csrc_count: 4;
  unsigned int marker: 1; //!< The interpretation of this is defined by a profile
  unsigned int payload_type: 7; //!< Identifies the format of the RTP payload
  uint16_t sequence_number; //!< Increments by one for each RTP data packet sent
  //! The timestamp reflects the sampling instant of the first octet in the RTP data packet
  uint32_t timestamp;
  uint32_t synchronization_source; //!< Identifies the synchronization source
  //! Identifiers of the extra sources
  std::array<uint32_t, kContributingSourcesMaxCount> contributing_sources;
  struct {
    uint16_t id; //!< The id of extension header. Defined by a profile
    uint16_t length; //!< Length of the extension header. Equals to content.size()
    Bytes content; //!< The actual header represented in bytes
  } extension_header; //!< Extension header. Used then extension bit is set
};

/**
 * @brief An RTP packet
 */
struct Packet {
  Header header;
  Bytes payload;
};

std::ostream &operator<<(std::ostream &os, const Packet &packet);

} // namespace rtp
