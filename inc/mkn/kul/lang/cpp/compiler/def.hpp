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
#ifndef MKN_KUL_LANG_CPP_COMPILER_DEF_HPP_
#define MKN_KUL_LANG_CPP_COMPILER_DEF_HPP_

// MKN_KUL_PRAGMA(x): "#pragma x" from inside a macro
#if defined(_MSC_VER) && !defined(__clang__)
#define MKN_KUL_PRAGMA(x) __pragma(x)
#else
#define MKN_KUL_PRAGMA(x) _Pragma(#x)
#endif

// MKN_KUL_FN_SIG: leads the signature of mkn.kul functions that can run on a device, empty unless
//  defined before including mkn.kul, e.g. "__host__ __device__" from mkn.gpu
#if !defined(MKN_KUL_FN_SIG)
#define MKN_KUL_FN_SIG
#endif

// MKN_KUL_FN_SIG_RET: MKN_KUL_FN_SIG for functions whose return value is the point of calling them
#if !defined(MKN_KUL_FN_SIG_RET)
#define MKN_KUL_FN_SIG_RET [[nodiscard]] MKN_KUL_FN_SIG
#endif

#endif /* MKN_KUL_LANG_CPP_COMPILER_DEF_HPP_ */
