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

#include <jpeglib.h>

#include <algorithm>

namespace {

/**
 * @brief Get width and height of the image
 *
 * @param image Pointer to the JPEG image bytes
 * @param size Size of the image in bytes
 * @return Pair of width and height
 */
std::pair<int, int> GetImageDimensions(const rtp::Byte *const image,
                                       const int size) {
  jpeg_decompress_struct cinfo;
  jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  jpeg_mem_src(&cinfo, image, size);

  (void) jpeg_read_header(&cinfo, TRUE);
  jpeg_calc_output_dimensions(&cinfo);

  std::pair res = {cinfo.output_width, cinfo.output_height};

  jpeg_destroy_decompress(&cinfo);

  return res;
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
rtp::mjpeg::Packet PackOne(const rtp::Byte *const data,
                           const int start,
                           const int count,
                           std::pair<int, int> dimensions,
                           const int quality) {
  rtp::mjpeg::Header header;
  header.type_specific = 0;
  header.fragment_offset = start;
  header.type = 1; // Because horiz. and vert. samp. fact. are 2, 1, 1
  header.quality = quality;
  header.width = dimensions.first / 8;
  header.height = dimensions.second / 8;

  rtp::mjpeg::Packet packet;
  packet.header = header;
  packet.payload = {data + start, data + start + count}; // ???
  return packet;
}

} // namespace

namespace rtp::mjpeg {

std::vector<Packet> PackJpeg(const Bytes &jpeg, const int quality) {
  const int kMaxBytesPerPacket = 512;
  std::vector<Packet> packets;
  packets.reserve((jpeg.size() / kMaxBytesPerPacket) + 1);

  for (std::size_t begin_index = 0;
       begin_index < jpeg.size();
       begin_index += kMaxBytesPerPacket) {
    const int count = std::min(static_cast<int>(jpeg.size() - begin_index),
                               kMaxBytesPerPacket);
    packets.push_back(
        PackOne(jpeg.data(),
                begin_index,
                count,
                GetImageDimensions(jpeg.data(), jpeg.size()),
                quality
        )
   );
  }

  return packets;
}

} // namespace rtp::mjpeg
