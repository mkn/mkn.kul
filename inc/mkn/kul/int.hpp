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
#ifndef MKN_KUL_INT_HPP_
#define MKN_KUL_INT_HPP_

#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <type_traits>

namespace mkn::kul {

// equality of a signed and an unsigned int without the conversion, a negative is never equal
template <typename SignedInt, typename UnsignedInt>
MKN_KUL_FN_SIG_RET constexpr bool diff_sign_int_equals(SignedInt const& i0, UnsignedInt const& i1) {
  static_assert(std::is_unsigned_v<UnsignedInt>);
  static_assert(std::is_signed_v<SignedInt>);
  static_assert(sizeof(UnsignedInt) >= sizeof(SignedInt), "Bad int comparison!");
  if (i0 < 0) return false;
  return static_cast<UnsignedInt>(i0) == i1;
}

// equality of ints of any signedness, see diff_sign_int_equals
template <typename Int0, typename Int1>
MKN_KUL_FN_SIG_RET constexpr bool int_equals(Int0 const& i0, Int1 const& i1) {
  if constexpr (std::is_unsigned_v<Int0> == std::is_unsigned_v<Int1>)
    return i0 == i1;
  else if constexpr (std::is_unsigned_v<Int0>)
    return diff_sign_int_equals(i1, i0);
  else
    return diff_sign_int_equals(i0, i1);
}

}  // namespace mkn::kul

#endif /* MKN_KUL_INT_HPP_ */
