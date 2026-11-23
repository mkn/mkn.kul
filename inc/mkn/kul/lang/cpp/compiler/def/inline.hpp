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
#ifndef MKN_KUL_LANG_CPP_COMPILER_DEF_INLINE_HPP_
#define MKN_KUL_LANG_CPP_COMPILER_DEF_INLINE_HPP_

// inlining regardless of the compiler's heuristics: for small hot functions whose inlining
//  otherwise flips with unrelated changes elsewhere in the translation unit
//
//  MKN_KUL_ALWAYS_INLINE         a function:  MKN_KUL_ALWAYS_INLINE auto f() { ... }
//  MKN_KUL_LAMBDA_ALWAYS_INLINE  a lambda, after its parameter list:
//                                  [](auto x) MKN_KUL_LAMBDA_ALWAYS_INLINE { ... }
//  MKN_KUL_INLINE_CALL           one call, whatever the callee: MKN_KUL_INLINE_CALL f(args...);
//                                  (clang 16+'s statement attribute, nothing elsewhere). E.g. a
//                                  GPU kernel calling its functor: out of line, the call takes
//                                  the address of the by value kernel parameter, which is then
//                                  copied whole into each thread's local memory

#if defined(_MSC_VER) && !defined(__clang__)
#define MKN_KUL_ALWAYS_INLINE __forceinline
#define MKN_KUL_LAMBDA_ALWAYS_INLINE
#elif defined(__GNUC__) || defined(__clang__)
#define MKN_KUL_ALWAYS_INLINE __attribute__((always_inline)) inline
#define MKN_KUL_LAMBDA_ALWAYS_INLINE __attribute__((always_inline))
#else
#define MKN_KUL_ALWAYS_INLINE inline
#define MKN_KUL_LAMBDA_ALWAYS_INLINE
#endif

#if defined(__clang__) && __clang_major__ >= 16
#define MKN_KUL_INLINE_CALL [[clang::always_inline]]
#else
#define MKN_KUL_INLINE_CALL
#endif

#endif /* MKN_KUL_LANG_CPP_COMPILER_DEF_INLINE_HPP_ */
