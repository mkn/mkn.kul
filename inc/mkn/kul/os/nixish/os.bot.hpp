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
// IWYU pragma: private, include "mkn/kul/os.hpp"

#ifndef _MKN_KUL_OS_NIXISH_OS_BOT_HPP_
#define _MKN_KUL_OS_NIXISH_OS_BOT_HPP_

void mkn::kul::Dir::rm() const {
  if (is()) {
    for (const auto &a : files()) a.rm();
    for (const auto &a : dirs()) a.rm();
    remove(real().c_str());
  }
}

std::string mkn::kul::Dir::LOCL(std::string s) {
  mkn::kul::String::REPLACE_ALL(s, "\\", "/");
  return s;
}

std::string mkn::kul::Dir::ESC(std::string s) {
  ESC_REPLACE(s, " ", "\\ ");
  return s;
}
bool mkn::kul::Dir::is() const {
  if (path().empty()) return false;
  DIR *d = opendir(path().c_str());
  if (d) closedir(d);
  return d;
}
bool mkn::kul::Dir::mk() const {
  if (path().empty()) return false;
  mkn::kul::Dir const &prnt(parent());
  if (_p != prnt.path() && !prnt.is()) parent().mk();
  return mkdir(locl().c_str(), 0777) == 0;
}
bool mkn::kul::Dir::root() const { return is() && real().size() == 1; }

bool mkn::kul::File::is() const {
  if (name().empty()) return false;
  struct stat buffer;
  if (stat(_d.join(_n).c_str(), &buffer) == 0) return S_ISREG(buffer.st_mode);
  return 0;
}
bool mkn::kul::File::rm() const {
  if (is()) {
    remove(real().c_str());
    return true;
  }
  return false;
}
bool mkn::kul::File::mk() const {
  FILE *pFile = fopen(full().c_str(), "w");
  if (pFile != NULL) {
    fclose(pFile);
  }
  return pFile != NULL;
}
uint64_t mkn::kul::File::size() const {
  uint64_t r = 0;
  struct stat att;
  if (stat(mini().c_str(), &att) != -1) r = att.st_size;
  return r;
}

namespace mkn {
namespace kul {
namespace os {

inline int exec(std::string const &cmd, bool q = false) {
  int r = 0;
  if (q)
    r = system(std::string(cmd + " > /dev/null").c_str());
  else
    r = system(cmd.c_str());
  if (r < 0) return r;
  return WEXITSTATUS(r);
}
inline std::string EOL() { return "\n"; }

}  // namespace os

namespace user {

inline mkn::kul::Dir home() { return Dir(env::GET("HOME")); }
inline mkn::kul::Dir home(std::string const &app) {
  return Dir(Dir::JOIN(env::GET("HOME"), "." + app));
}

}  // namespace user

namespace env {
inline bool CWD(mkn::kul::Dir const &d) { return chdir(d.path().c_str()) != -1; }
}  // namespace env
}  // namespace kul
}  // namespace mkn

#ifndef _MKN_KUL_COMPILED_LIB_
#include "mkn/kul/os/nixish/src/os/dir/dirs.ipp"
#include "mkn/kul/os/nixish/src/os/dir/files.ipp"
#include "mkn/kul/os/nixish/src/os/dir/real.ipp"
#endif  //_MKN_KUL_COMPILED_LIB_

#endif /* _MKN_KUL_OS_NIXISH_OS_BOT_HPP_ */
