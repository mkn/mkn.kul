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
#ifndef MKN_KUL_LANG_CPP_COMPILER_DEF_WARNING_HPP_
#define MKN_KUL_LANG_CPP_COMPILER_DEF_WARNING_HPP_

#include "mkn/kul/lang/cpp/compiler/def.hpp"

// MKN_KUL_DISABLE_WARNING(gcc_option, clang_option, msvc_code) ... MKN_KUL_ENABLE_WARNING(same)
//  turns one warning off between the two, each compiler taking its own argument, options without
//  the -W. clang-format reads them as subtractions (unused - variable), so keep it off around them:
//   // clang-format off
//   MKN_KUL_DISABLE_WARNING(unused-variable, unused-variable, 4189)
//   // clang-format on
//   int unused = 0;
//   // clang-format off
//   MKN_KUL_ENABLE_WARNING(unused-variable, unused-variable, 4189)
//   // clang-format on
//  from https://stackoverflow.com/a/36175016/795574

#define MKN_KUL_WARNING_STR_(s) #s
#define MKN_KUL_WARNING_OPTION_(x, y) MKN_KUL_WARNING_STR_(x##y)
// x is macro expanded here, before MKN_KUL_PRAGMA stringizes it
#define MKN_KUL_DIAGNOSTIC_(compiler, x) MKN_KUL_PRAGMA(compiler diagnostic x)

#if defined(__clang__)
#define MKN_KUL_DISABLE_WARNING(gcc_unused, clang_option, msvc_unused) \
  MKN_KUL_DIAGNOSTIC_(clang, push)                                     \
  MKN_KUL_DIAGNOSTIC_(clang, ignored MKN_KUL_WARNING_OPTION_(-W, clang_option))
#define MKN_KUL_ENABLE_WARNING(gcc_unused, clang_option, msvc_unused) \
  MKN_KUL_DIAGNOSTIC_(clang, pop)
#elif defined(_MSC_VER)
#define MKN_KUL_DISABLE_WARNING(gcc_unused, clang_unused, msvc_code) \
  MKN_KUL_PRAGMA(warning(push)) MKN_KUL_PRAGMA(warning(disable : msvc_code))
#define MKN_KUL_ENABLE_WARNING(gcc_unused, clang_unused, msvc_code) MKN_KUL_PRAGMA(warning(pop))
#elif defined(__GNUC__)
#define MKN_KUL_DISABLE_WARNING(gcc_option, clang_unused, msvc_unused) \
  MKN_KUL_DIAGNOSTIC_(GCC, push)                                       \
  MKN_KUL_DIAGNOSTIC_(GCC, ignored MKN_KUL_WARNING_OPTION_(-W, gcc_option))
#define MKN_KUL_ENABLE_WARNING(gcc_option, clang_unused, msvc_unused) MKN_KUL_DIAGNOSTIC_(GCC, pop)
#else
#define MKN_KUL_DISABLE_WARNING(gcc_unused, clang_unused, msvc_unused)
#define MKN_KUL_ENABLE_WARNING(gcc_unused, clang_unused, msvc_unused)
#endif

#endif /* MKN_KUL_LANG_CPP_COMPILER_DEF_WARNING_HPP_ */
