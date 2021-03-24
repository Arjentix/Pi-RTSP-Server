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
#include <vector>

#include "report_block.h"

namespace rtp {

namespace rtcp {

/**
 * @brief Sender Report RTCP Packet
 */
struct SenderReport {
  unsigned int version: 2; //!< RTP (RTCP) version
  unsigned int padding: 1; //!< Padding bit
  //! The number of reception report blocks
  unsigned int reception_report_count: 5;
  //! Contains the constant 200 to identify this as an RTCP SR packet
  uint8_t packet_type = 200;
  uint16_t length; //!< The length of this RTCP packet in 32-bit words minus one
  uint32_t synchronization_source; //!< Identifies the synchronization source
  //! Summarizes the data transmissions from this sender
  struct {
    uint64_t ntp_timestamp; //!< Indicates the wallclock time
    uint32_t rtp_timestamp; //!< Corresponds to the same time as ntp, but in RTP format
    //! The total number of RTP data packets transmitted by the sender
    uint32_t packet_count;
    uint32_t octet_count; //! The total number of payload octets transmitted
  } sender_info;
  std::vector<ReportBlock> report_blocks; //! Zero ore more reception report blocks
};

} // namespace rtcp

} // namespace rtp
