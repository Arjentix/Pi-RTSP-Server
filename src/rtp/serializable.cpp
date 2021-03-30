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

#include "serializable.h"

namespace {

const uint32_t kFirstOctetMask = 0xFF;
const uint32_t kSecondOctetMask = 0xFF00;
const uint32_t kThirdOctetMask = 0xFF0000;
const uint32_t kFourthOctetMask = 0xFF000000;

} // namespace

namespace rtp {

Bytes Serialize16(uint16_t value) {
  Bytes bytes;
  bytes.reserve(2);

  bytes.push_back((value & kSecondOctetMask) >> 8);
  bytes.push_back(value & kFirstOctetMask);

  return bytes;
}

Bytes Serialize24(const unsigned int value) {
  Bytes bytes;
  bytes.reserve(3);

  bytes.push_back((value & kThirdOctetMask) >> 16);
  bytes.push_back((value & kSecondOctetMask) >> 8);
  bytes.push_back(value & kFirstOctetMask);

  return bytes;
}

Bytes Serialize32(const uint32_t value) {
  Bytes bytes;
  bytes.reserve(4);

  bytes.push_back((value & kFourthOctetMask) >> 24);
  bytes.push_back((value & kThirdOctetMask) >> 16);
  bytes.push_back((value & kSecondOctetMask) >> 8);
  bytes.push_back(value & kFirstOctetMask);

  return bytes;
}

} // namespace rtp