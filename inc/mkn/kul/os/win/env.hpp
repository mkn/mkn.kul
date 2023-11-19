/**
Copyright (c) 2023, Philip Deegan.
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
#ifndef _MKN_KUL_OS_WIN_ENV_HPP_
#define _MKN_KUL_OS_WIN_ENV_HPP_

#include <direct.h>
#include <io.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>
#include <algorithm>
#include <fstream>

namespace mkn {
namespace kul {
namespace env {

#if defined(_MKN_KUL_MAX_PATH_)
constexpr size_t KUL_MAX_PATH = _MKN_KUL_MAX_PATH_;
#elif defined(_MAX_PATH)
constexpr size_t KUL_MAX_PATH = _MAX_PATH;
#else
#error  // could not set KUL_MAX_PATH
#endif  /*_MKN_KUL_MAX_PATH_*/

inline std::string EOL() {
#if (_MSC_VER >= 1800)
  return "\n";
#else
  return "\r\n";
#endif
}

inline bool EXISTS(char const *c) {
  bool set = 0;
  char *r = 0;
  size_t len;
  _dupenv_s(&r, &len, c);
  set = r;
  if (len) free(r);
  return set;
}
inline std::string GET(char const *c, std::string default_ = "") {
  char *r;
  size_t len;
  _dupenv_s(&r, &len, c);
  if (len) {
    std::string s(r);
    free(r);
    return s;
  }
  return default_;
}

inline void SET(char const *var, char const *val) {
  _putenv(std::string(std::string(var) + "=" + std::string(val)).c_str());
}

inline char SEP() { return ';'; }

inline std::string CWD() {
  char c[KUL_MAX_PATH];
  _getcwd(c, KUL_MAX_PATH);
  std::string str(c);
  return str;
}

inline bool CWD(std::string const &c) { return _chdir(c.c_str()) != -1; }

}  // namespace env
}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_OS_WIN_ENV_HPP_ */
