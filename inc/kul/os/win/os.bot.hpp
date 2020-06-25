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
#ifndef _KUL_OS_WIN_OS_BOT_HPP_
#define _KUL_OS_WIN_OS_BOT_HPP_

void kul::Dir::rm() const {
  if (is()) {
    for (const auto &a : files()) a.rm();
    for (const auto &a : dirs()) a.rm();
    _rmdir(path().c_str());
  }
}

std::string kul::Dir::ESC(std::string s) {
  ESC_REPLACE(s, "\\", "\\\\");
  return s;
}

std::string kul::Dir::LOCL(std::string s) {
  kul::String::REPLACE_ALL(s, "/", "\\");
  return s;
}

bool kul::Dir::is() const {
  if (path().empty()) return false;
  DWORD ftyp = GetFileAttributesA(path().c_str());
  return (ftyp != INVALID_FILE_ATTRIBUTES && ftyp & FILE_ATTRIBUTE_DIRECTORY);
}
bool kul::Dir::mk() const {
  if (path().empty()) return false;
  kul::Dir const &prnt(parent());
  if (_p != prnt.path() && !prnt.is()) parent().mk();
  return CreateDirectory(locl().c_str(), NULL);
}
bool kul::Dir::root() const { return is() && real().size() == 3; }

bool kul::File::is() const { return !name().empty() && (bool)std::ifstream(_d.join(_n).c_str()); }
bool kul::File::rm() const {
  if (is()) {
    _unlink(_d.join(_n).c_str());
    return true;
  }
  return false;
}

bool kul::File::mk() const {
  FILE *pFile;
  fopen_s(&pFile, full().c_str(), "w");
  if (pFile != NULL) {
    fclose(pFile);
  }
  return pFile != NULL;
}

uint64_t kul::File::size() const {
  uint64_t r = 0;
  WIN32_FIND_DATA ffd;
  HANDLE h = FindFirstFile(TEXT(mini().c_str()), &ffd);
  if (h) {
    ULARGE_INTEGER ul;
    ul.HighPart = ffd.nFileSizeHigh;
    ul.LowPart = ffd.nFileSizeLow;
    r = ul.QuadPart;
    FindClose(h);
  }
  return r;
}

namespace kul {
namespace os {

inline uint16_t exec(std::string const &cmd, bool q = false) {
  if (q) {
    return system(std::string(cmd + " > nul").c_str());
  }
  return system(cmd.c_str());
}
inline std::string EOL() {
#if (_MSC_VER >= 1800)
  return "\n";
#else
  return "\r\n";
#endif  // _MSC_VER
}

}  // namespace os

namespace user {

inline kul::Dir home() {
  const std::string h(env::GET("HOME"));
  if (h.size()) return kul::Dir(h);
  return kul::Dir(std::string(env::GET("HOMEDRIVE")) + std::string(env::GET("HOMEPATH")));
}
inline kul::Dir home(std::string const &app) { return kul::Dir(home().join(app)); }

}  // namespace user

namespace env {
inline bool CWD(kul::Dir const &d) { return _chdir(d.path().c_str()) != -1; }
}  // namespace env
}  // namespace kul

#ifndef _KUL_COMPILED_LIB_
#include "kul/os/win/src/os/dir/dirs.ipp"
#include "kul/os/win/src/os/dir/files.ipp"
#include "kul/os/win/src/os/dir/real.ipp"
#endif  //_KUL_COMPILED_LIB_

#endif /* _KUL_OS_WIN_OS_BOT_HPP_ */
