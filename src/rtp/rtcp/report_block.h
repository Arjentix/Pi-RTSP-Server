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

namespace rtp {

namespace rtcp {

/**
 * @brief An RTCP Reception Report Block
 */
struct ReportBlock {
  //! The SSRC identifier of the source to which the information in this
  //! reception report block pertains
  uint32_t source_identifier;
  //! The fraction of RTP data packets from source SSRC n lost since the
  //! previous SR or RR packet was sent
  uint8_t fraction_lost;
  //! The total number of RTP data packets from source SSRC n that have been
  //! lost since the beginning of reception
  uint32_t cumulative_number_of_packets_lost: 24;
  //! The low 16 bits contain the highest sequence number received in an RTP
  //! data packet from source SSRC n, and the most significant 16 bits extend
  //! that sequence number with the cor- responding count of sequence number
  //! cycles, which may be maintained according to the al- gorithm
  uint32_t extended_highest_sequence_number_received;
  //! An estimate of the statistical variance of the RTP data packet
  //! interarrival time, measured in timestamp units and expressed as an
  //! unsigned integer
  uint32_t interarrival_jitter;
  //! The middle 32 bits out of 64 in the NTP timestamp received as part of the
  //! most recent RTCP sender report (SR) packet
  uint32_t last_sr_timestamp;
  //! The delay, expressed in units of 1/65536 seconds, between receiving the
  //! last SR packet from source SSRC n and sending this reception report block
  uint32_t delay_since_last_sr;
};

} // namespace rtcp

} // namespace rtp
