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

#include "packet.h"

#include <algorithm>

namespace {

/**
 * @brief Get entropy encoded segment (main data) of jpeg image
 *
 * @param jpeg_image Bytes representing jpeg image
 * @return Entropy encoded segment
 */
Bytes GetEntropyEncodedSegment(const Bytes &jpeg_image) {
  auto sos_begin = jpeg_image.begin();
  for (; sos_begin != jpeg_image.end(); ++sos_begin) {
    if ((*sos_begin == 0xFF) &&
        (sos_begin != std::prev(jpeg_image.end())) &&
        (*std::next(sos_begin) == 0xDA)) {
      break;
    }
  }

  if (sos_begin == jpeg_image.end()) {
    return {};
  }

  uint16_t length = (uint16_t (*(sos_begin + 2)) << 8) | uint16_t (*(sos_begin + 3));
  auto encoded_data_begin = sos_begin + length + 2;

  auto encoded_data_end = jpeg_image.rbegin();
  for (; encoded_data_end != std::reverse_iterator(encoded_data_begin); ++encoded_data_end) {
    if ((*encoded_data_end == 0xFF) &&
        (encoded_data_end != jpeg_image.rend()) &&
        (*std::prev(encoded_data_end) == 0xD9)) {
      break;
    }
  }

  return {encoded_data_begin, encoded_data_end.base() + 3};
}

/**
 * @brief Pack given data to one MJPEG over RTP packet
 *
 * @param data Pointer to the whole JPEG data
 * @param start Start pos for current packet
 * @param count Number of bytes to read
 * @param dimensions Pair of width and height
 * @param quality JPEG quality in [0-100] range
 * @return MJPEG over RTP packet with part of JPEG image data
 */
rtp::mjpeg::Packet PackOne(const Byte *const data,
                           const int start,
                           const int count,
                           const unsigned int width,
                           const unsigned int height,
                           const int quality) {
  rtp::mjpeg::Header header;
  header.type_specific = 0;
  header.fragment_offset = start;
  header.type = 1; // Because horiz. and vert. samp. fact. are 2, 1, 1
  header.quality = quality;
  header.width = width / 8;
  header.height = height / 8;

  rtp::mjpeg::Packet packet;
  packet.header = header;
  packet.payload = {data + start, data + start + count}; // ???
  return packet;
}

} // namespace

namespace rtp::mjpeg {

Bytes Packet::Serialize() const {
  Bytes bytes;
  bytes.reserve(8);

  bytes.push_back(header.type_specific);
  Bytes serialized_tmp = Serialize24(header.fragment_offset);
  bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
  bytes.push_back(header.type);
  bytes.push_back(header.quality);
  bytes.push_back(header.width);
  bytes.push_back(header.height);
  if (header.type >= 64 && header.type < 128) {
    serialized_tmp = Serialize32(header.restart_marker_header);
    bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
  }
  if (header.quality >= 128) {
    bytes.push_back(header.quantization_table_header.mbz);
    bytes.push_back(header.quantization_table_header.precision);
    bytes.push_back(header.quantization_table_header.length);
    bytes.insert(bytes.end(),
                 header.quantization_table_header.data.begin(),
                 header.quantization_table_header.data.end());
  }

  bytes.insert(bytes.end(), payload.begin(), payload.end());

  return bytes;
}

std::vector<Packet> PackJpeg(const Bytes &jpeg, const unsigned int width,
                             const unsigned int height, const int quality) {
  const int kMaxBytesPerPacket = 512;
  std::vector<Packet> packets;
  Bytes segment = GetEntropyEncodedSegment(jpeg);
  packets.reserve((segment.size() / kMaxBytesPerPacket) + 1);

  for (std::size_t begin_index = 0;
       begin_index < segment.size();
       begin_index += kMaxBytesPerPacket) {
    const int count = std::min(static_cast<int>(segment.size() - begin_index),
                               kMaxBytesPerPacket);
    packets.push_back(
        PackOne(segment.data(), begin_index, count,
                width, height, quality)
    );
  }

  return packets;
}

rtp::Packet PackToRtpPacket(const Packet &mjpeg_packet, const bool final,
                            const uint16_t sequence_number,
                            const uint32_t timestamp,
                            const uint32_t synchronization_source) {
  rtp::Header header;
  header.version = 2;
  header.padding = 0;
  header.extension = 0;
  header.csrc_count = 0;
  header.marker = (final ? 1 : 0);
  header.payload_type = 26;
  header.sequence_number = sequence_number;
  header.timestamp = timestamp;
  header.synchronization_source = synchronization_source;
  header.contributing_sources = {};
  header.extension_header = {};

  rtp::Packet packet;
  packet.header = header;
  packet.payload = mjpeg_packet.Serialize();
  return packet;
}

} // namespace rtp::mjpeg
