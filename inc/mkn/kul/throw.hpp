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
#ifndef MKN_KUL_THROW_HPP_
#define MKN_KUL_THROW_HPP_

#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <cassert>  // IWYU pragma: keep, device code asserts
#include <utility>
#include <stdexcept>

namespace mkn::kul {

// throws Exception{args...}, in device code, which can't throw, it asserts instead
template <typename Exception, typename... Args>
MKN_KUL_FN_SIG_ALL inline void throw_exception([[maybe_unused]] Args&&... args) {
#if defined(__CUDA_ARCH__) || defined(__HIP_DEVICE_COMPILE__)
  assert(false);
#else
  throw Exception{std::forward<Args>(args)...};
#endif
}

template <typename T>
MKN_KUL_FN_SIG_ALL inline void throw_runtime_error(T const& err) {
  throw_exception<std::runtime_error>(err);
}

}  // namespace mkn::kul

#endif /* MKN_KUL_THROW_HPP_ */
