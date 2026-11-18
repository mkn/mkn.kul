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
#ifndef MKN_KUL_LANG_CPP_COMPILER_DEF_UNROLL_LOOP_HPP_
#define MKN_KUL_LANG_CPP_COMPILER_DEF_UNROLL_LOOP_HPP_

#include "mkn/kul/lang/cpp/compiler/def.hpp"

// MKN_KUL_UNROLL_LOOP(n): unroll the following loop n times
//  nvcc only knows "#pragma unroll", gcc only "#pragma GCC unroll", clang (CUDA and hipcc) takes
//  both, MSVC has no unroll pragma so it's a no-op there
#if defined(__NVCC__)
#define MKN_KUL_UNROLL_LOOP(n) MKN_KUL_PRAGMA(unroll n)
#elif defined(_MSC_VER) && !defined(__clang__)
#define MKN_KUL_UNROLL_LOOP(n)
#else
#define MKN_KUL_UNROLL_LOOP(n) MKN_KUL_PRAGMA(GCC unroll n)
#endif

#endif /* MKN_KUL_LANG_CPP_COMPILER_DEF_UNROLL_LOOP_HPP_ */
