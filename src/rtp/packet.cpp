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

namespace rtp {

Bytes Packet::Serialize() const {
  Bytes bytes;

  bytes.push_back((header.version << 6) | (header.padding << 5) |
                  (header.extension << 4 ) | header.csrc_count);
  bytes.push_back((header.marker << 7) | header.payload_type);
  Bytes serialized_tmp = Serialize16(header.sequence_number);
  bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
  serialized_tmp = Serialize32(header.timestamp);
  bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
  serialized_tmp = Serialize32(header.synchronization_source);
  bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
  if (header.csrc_count > 0) {
    for (auto it = header.contributing_sources.begin();
         it != header.contributing_sources.end();
         ++it) {
      serialized_tmp = Serialize32(*it);
      bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
    }
  }
  if (header.extension == 1) {
    serialized_tmp = Serialize16(header.extension_header.id);
    bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
    serialized_tmp = Serialize16(header.extension_header.length);
    bytes.insert(bytes.end(), serialized_tmp.begin(), serialized_tmp.end());
    bytes.insert(bytes.end(), header.extension_header.content.begin(),
                 header.extension_header.content.begin());
  }

  bytes.insert(bytes.end(), payload.begin(), payload.end());

  return bytes;
}

} // namespace rtp
