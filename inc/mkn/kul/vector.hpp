/**
Copyright (c) 2022, Philip Deegan.
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
#ifndef _MKN_KUL_VECTOR_HPP_
#define _MKN_KUL_VECTOR_HPP_

#include "mkn/kul/alloc.hpp"

#include <vector>

namespace mkn::kul {

template <typename T, typename A = void>  // A ignored but there for std::vector interop
using NonConstructingVector = std::vector<T, NonConstructingAllocator<T>>;

template <typename T>
std::vector<T, Allocator<T>>& as_super(std::vector<T, NonConstructingAllocator<T>>& v) {
  return *reinterpret_cast<std::vector<T, Allocator<T>>*>(&v);
}
template <typename T>
std::vector<T, Allocator<T>> const& as_super(std::vector<T, NonConstructingAllocator<T>> const& v) {
  return *reinterpret_cast<std::vector<T, Allocator<T>> const*>(&v);
}

}  // namespace mkn::kul

template <typename T, typename A0, typename A1 = void>
bool operator==(std::vector<T, A0> const& v0, mkn::kul::NonConstructingVector<T, A1> const& v1) {
  if (v0.size() != v1.size()) return false;
  for (std::size_t i = 0; i < v0.size(); i++)
    if (v0[i] != v1[i]) return false;
  return true;
}

template <typename T, typename A0, typename A1 = void>
bool operator==(mkn::kul::NonConstructingVector<T, A1> const& v0, std::vector<T, A0> const& v1) {
  return v1 == v0;
}

template <typename T, typename A0, typename A1 = void>
bool operator!=(std::vector<T, A0> const& v0, mkn::kul::NonConstructingVector<T, A1> const& v1) {
  return !(v0 == v1);
}

template <typename T, typename A0, typename A1 = void>
bool operator!=(mkn::kul::NonConstructingVector<T, A1> const& v0, std::vector<T, A0> const& v1) {
  return !(v0 == v1);
}

#endif /*_MKN_KUL_VECTOR_HPP_*/
