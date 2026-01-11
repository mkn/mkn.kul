/**
Copyright (c) 2026, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _MKN_KUL_BYTE_HPP_
#define _MKN_KUL_BYTE_HPP_

#include <cstdint>

namespace mkn {
namespace kul {
namespace byte {

inline bool isBigEndian() {
  int const one = 1;
  return (*(char*)&one) == 0;
}

class LittleEndian {
 public:
  static uint16_t UINT16(uint16_t i) {
    uint16_t n = 42;
    if (*(char*)&n != 42) {
      return (i << 8) | (i >> 8);
    }
    return i;
  }
  static int16_t INT16(int16_t i) {
    int16_t n = 42;
    if (*(char*)&n != 42) {
      return (i << 8) | ((i >> 8) & 0xFF);
    }
    return i;
  }
  static uint32_t UINT32(uint32_t i) {
    uint32_t n = 42;
    if (*(char*)&n != 42) {
      i = ((i << 8) & 0xFF00FF00) | ((i >> 8) & 0xFF00FF);
      return (i << 16) | (i >> 16);
    }
    return i;
  }
  static int32_t INT32(int32_t i) {
    int32_t n = 42;
    if (*(char*)&n != 42) {
      i = ((i << 8) & 0xFF00FF00) | ((i >> 8) & 0xFF00FF);
      return (i << 16) | ((i >> 16) & 0xFFFF);
    }
    return i;
  }
  static uint64_t UINT64(uint64_t i) {
    uint64_t n = 42;
    if (*(char*)&n != 42) {
      i = ((i << 8) & 0xFF00FF00FF00FF00ULL) | ((i >> 8) & 0x00FF00FF00FF00FFULL);
      i = ((i << 16) & 0xFFFF0000FFFF0000ULL) | ((i >> 16) & 0x0000FFFF0000FFFFULL);
      return (i << 32) | (i >> 32);
    }
    return i;
  }
  static int64_t INT64(int64_t i) {
    int64_t n = 42;
    if (*(char*)&n != 42) {
      i = ((i << 8) & 0xFF00FF00FF00FF00ULL) | ((i >> 8) & 0x00FF00FF00FF00FFULL);
      i = ((i << 16) & 0xFFFF0000FFFF0000ULL) | ((i >> 16) & 0x0000FFFF0000FFFFULL);
      return (i << 32) | ((i >> 32) & 0xFFFFFFFFULL);
    }
    return i;
  }
};
}  // namespace byte
}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_BYTE_HPP_ */
