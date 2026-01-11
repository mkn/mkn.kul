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
#ifndef _MKN_KUL_DEFS_HPP_
#define _MKN_KUL_DEFS_HPP_

#define KSTRINGIFY(x) #x
#define KTOSTRING(x) KSTRINGIFY(x)

#ifdef MKN_KUL_SHARED
#if defined _WIN32 || defined __CYGWIN__
#ifdef MKN_KUL_EXPORT
#ifdef __GNUC__
#define MKN_KUL_PUBLISH __attribute__((dllexport))
#else
#define MKN_KUL_PUBLISH __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define MKN_KUL_PUBLISH __attribute__((dllimport))
#else
#define MKN_KUL_PUBLISH __declspec(dllimport)
#endif
#endif
#else
#if __GNUC__ >= 4
#define MKN_KUL_PUBLISH __attribute__((visibility("default")))
#define MKN_KUL_PRIVATE __attribute__((visibility("hidden")))
#endif
#endif
#endif  // MKN_KUL_SHARED

#ifndef MKN_KUL_PUBLISH
#define MKN_KUL_PUBLISH
#endif

#ifndef MKN_KUL_PRIVATE
#define MKN_KUL_PRIVATE
#endif

#if defined(__APPLE__) || defined(__NetBSD__) || defined(__FreeBSD__)
#define MKN_KUL_IS_BSD 1
#endif

#if defined(_WIN32)
#define MKN_KUL_IS_WIN 1
#endif

#ifndef MKN_KUL_IS_WIN
#define MKN_KUL_IS_WIN 0
#endif

#ifndef MKN_KUL_IS_BSD
#define MKN_KUL_IS_BSD 0
#endif

#if !MKN_KUL_IS_WIN && !MKN_KUL_IS_BSD
#define MKN_KUL_IS_NIX 1
#endif

#ifndef MKN_KUL_IS_NIX
#define MKN_KUL_IS_NIX 0
#endif

#if !defined(NDEBUG) || defined(KUL_FORCE_DEBUG_DO)
#define MKN_KUL_DEBUG_DO(...) __VA_ARGS__
#define MKN_KUL_DEBUG_DO_ELSE(...)
#else
#define MKN_KUL_DEBUG_DO(...)
#define MKN_KUL_DEBUG_DO_ELSE(...) __VA_ARGS__
#endif

#include "mkn/kul/os/def.hpp"

#endif /* _MKN_KUL_DEFS_HPP_ */
