/**
Copyright (c) 2017, Philip Deegan.
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
// IWYU pragma: private, include "mkn/kul/env.hpp"

#ifndef _MKN_KUL_OS_NIXISH_ENV_HPP_
#define _MKN_KUL_OS_NIXISH_ENV_HPP_

#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <thread>

#include <limits.h>

namespace mkn {
namespace kul {
namespace env {

inline std::string EOL() { return "\r\n"; }

#if defined(_MKN_KUL_MAX_PATH_)
constexpr size_t KUL_MAX_PATH = _MKN_KUL_MAX_PATH_;
#elif defined(PATH_MAX)
constexpr size_t KUL_MAX_PATH = PATH_MAX;
#else
#error  // could not set KUL_MAX_PATH
#endif  /*_MKN_KUL_MAX_PATH_*/

inline bool EXISTS(char const* c) { return getenv(c); }
inline std::string GET(char const* c, std::string default_ = "") {
  char const* r = getenv(c);
  return std::string(r ? r : default_);
}
inline void SET(char const* var, char const* val) { setenv(var, val, 1); }
inline char SEP() { return ':'; }

inline std::string CWD() {
  char c[KUL_MAX_PATH];
  auto r = getcwd(c, KUL_MAX_PATH);
  (void)r;
  std::string str(c);
  return str;
}

inline bool CWD(std::string const& c) { return chdir(c.c_str()) != -1; }

}  // namespace env
}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_OS_NIXISH_ENV_HPP_ */
