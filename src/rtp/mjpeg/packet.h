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

#include "rtp/serializable.h"
#include "rtp/packet.h"

namespace rtp::mjpeg {

/**
 * @brief An MJPEG over RTP header
 */
struct Header {
  uint8_t type_specific; //!< Interpretation depends on the value of the type field
  //! The offset in bytes of the current packet in the JPEG frame data
  unsigned int fragment_offset: 24;
  uint8_t type; //!< Specifies how to recover image
  uint8_t quality; //!< Image quality
  uint8_t width; //!< Image width divided by 8 pixels
  uint8_t height; //!< Image height divided by 8 pixels
  //! Used when there are RST markers in jpeg. Occurs only when 63 < type < 128
  uint32_t restart_marker_header;
  //! Occurs only when 127 < quality < 256
  struct {
    uint8_t mbz; //!< MBZ
    uint8_t precision; //!< Precision
    uint16_t length; //!< The length of data in bytes. Equals to data.size()
    Bytes data; //!< Quantization table data
  } quantization_table_header;
};

/**
 * @brief An MJPEG over RTP packet
 */
struct Packet : Serializable {
  Header header;
  Bytes payload;

  Bytes Serialize() const override;
};

/**
 * @brief Pack and split JPEG image into MJPEG over RTP packets
 *
 * @param jpeg Bytes of the JPEG image
 * @return Vector of MJPEG packets
 */
std::vector<Packet> PackJpeg(const Bytes &jpeg, int quality);

/**
 * @brief Pack MJPEG over RTP packet to the RTP packet
 *
 * @param mjpeg_packet MJPEG over RTP packet
 * @param final Flag of last JPEG part in mjpeg_packet
 * @param sequence_number Number of packet, increments by one for each packet,
 * init value should be random
 * @param timestamp The timestamp of whole frame
 * @param synchronization_source Random id of the current RTP source
 * @return RTP packet
 */
rtp::Packet PackToRtpPacket(const mjpeg::Packet &mjpeg_packet, bool final,
                            uint16_t sequence_number, uint32_t timestamp,
                            uint32_t synchronization_source);

} // namespace rtp::mjpeg
