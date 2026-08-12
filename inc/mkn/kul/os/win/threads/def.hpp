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
// IWYU pragma: private, include "mkn/kul/threads/def.hpp"

#ifndef MKN_KUL_OS_WIN_THREADS_DEF_HPP_
#define MKN_KUL_OS_WIN_THREADS_DEF_HPP_

#include <Windows.h>

#include <functional>
#include <sstream>
#include <string>
#include <thread>

namespace mkn::kul::this_thread {
inline std::string const id() {
  std::ostringstream os;
  os << std::hex << std::hash<std::thread::id>()(std::this_thread::get_id());
  return os.str();
}

inline bool main() {
#include "mkn/kul/os/win/src/thread/main.cpp"
}

inline void kill() {
  HANDLE h = GetCurrentThread();
  TerminateThread(h, 0);
  CloseHandle(h);
}
}  // namespace mkn::kul::this_thread

#endif /* MKN_KUL_OS_WIN_THREADS_DEF_HPP_ */
