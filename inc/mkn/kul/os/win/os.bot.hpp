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
// IWYU pragma: private, include "mkn/kul/os.hpp"
// IWYU pragma: friend ".*mkn/kul/os/win/.*"

#ifndef MKN_KUL_OS_WIN_OS_BOT_HPP_
#define MKN_KUL_OS_WIN_OS_BOT_HPP_

std::string mkn::kul::Dir::ESC(std::string s) {
  ESC_REPLACE(s, "\\", "\\\\");
  return s;
}

std::string mkn::kul::Dir::LOCL(std::string s) {
  mkn::kul::String::REPLACE_ALL(s, "/", "\\");
  return s;
}

namespace mkn {
namespace kul {
namespace os {

inline uint16_t exec(std::string const& cmd, bool q = false) {
  if (q) {
    return system(std::string(cmd + " > nul").c_str());
  }
  return system(cmd.c_str());
}
}  // namespace os

namespace user {

inline mkn::kul::Dir home() {
  std::string const h(env::GET("HOME"));
  if (h.size()) return mkn::kul::Dir(h);
  return mkn::kul::Dir(std::string(env::GET("HOMEDRIVE")) + std::string(env::GET("HOMEPATH")));
}
inline mkn::kul::Dir home(std::string const& app) { return mkn::kul::Dir(home().join(app)); }

}  // namespace user

namespace env {
inline bool CWD(mkn::kul::Dir const& d) { return _chdir(d.path().c_str()) != -1; }
}  // namespace env
}  // namespace kul
}  // namespace mkn

#endif /* MKN_KUL_OS_WIN_OS_BOT_HPP_ */
