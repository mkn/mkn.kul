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
#ifndef MKN_KUL_STRIDED_SPAN_HPP_
#define MKN_KUL_STRIDED_SPAN_HPP_

// StridedSpan<T>: n values of T, stride bytes apart, in someone else's memory, without their type:
//
//   one member of an array of structs           stride sizeof(struct)
//   one component of an array of std::array      stride sizeof(std::array)
//   a contiguous column                          stride sizeof(T)
//
// what a live view of it needs (numpy takes strides) and what an HDF5 memory selection needs
//  (a hyperslab with a stride), so either can read or write the owner's data in place. The
//  stride is a whole number of T (stride_values), as numpy and HDF5 count in values

#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace mkn::kul {

template <typename T>
struct StridedSpan {
  using value_type = T;
  using byte_type = std::conditional_t<std::is_const_v<T>, std::byte const, std::byte>;

  T* ptr = nullptr;
  std::size_t n = 0;
  std::size_t stride = sizeof(T);  // bytes

  MKN_KUL_FN_SIG_ALL_RET std::size_t size() const { return n; }
  MKN_KUL_FN_SIG_ALL_RET T* data() const { return ptr; }
  MKN_KUL_FN_SIG_ALL_RET bool contiguous() const { return stride == sizeof(T); }
  MKN_KUL_FN_SIG_ALL_RET std::size_t stride_values() const { return stride / sizeof(T); }

  // the same values, read only (as std::span<T> to std::span<T const>)
  MKN_KUL_FN_SIG_ALL operator StridedSpan<T const>() const
    requires(!std::is_const_v<T>)
  {
    return {ptr, n, stride};
  }

  MKN_KUL_FN_SIG_ALL_RET T& operator[](std::size_t const i) const {
    return *reinterpret_cast<T*>(reinterpret_cast<byte_type*>(ptr) + i * stride);
  }
};

// n values from first, stride bytes apart: a whole number of T
template <typename T>
MKN_KUL_FN_SIG_RET StridedSpan<T> strided_span(T* const first, std::size_t const n,
                                               std::size_t const stride) {
  if (stride < sizeof(T) or stride % sizeof(T) != 0)
    throw std::runtime_error("mkn::kul::strided_span: stride not a whole number of values");
  return {first, n, stride};
}

}  // namespace mkn::kul

#endif /* MKN_KUL_STRIDED_SPAN_HPP_ */
