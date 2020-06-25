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
#ifndef _KUL_OS_WIN_OS_TOP_HPP_
#define _KUL_OS_WIN_OS_TOP_HPP_

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

#include "kul/defs.hpp"

namespace kul {

class Dir;
namespace fs {

class KulTimeStampsResolver {
 private:
  static uint64_t FileTimeToPOSIX(FILETIME &ft) {
    LARGE_INTEGER date, adjust;
    date.HighPart = ft.dwHighDateTime;
    date.LowPart = ft.dwLowDateTime;
    adjust.QuadPart = 11644473600000 * 10000;
    date.QuadPart -= adjust.QuadPart;
    return (uint64_t)date.QuadPart / 10000000;
  }
  static void GET(char const *const p, uint64_t &a, uint64_t &c, uint64_t &m) {
    WIN32_FIND_DATA ffd;
    HANDLE h = FindFirstFile(TEXT(p), &ffd);
    if (h) {
      a = FileTimeToPOSIX(ffd.ftLastAccessTime);
      c = FileTimeToPOSIX(ffd.ftCreationTime);
      m = FileTimeToPOSIX(ffd.ftLastWriteTime);
      FindClose(h);
    }
  }
  friend class kul::Dir;
};
}  // namespace fs

}  // namespace kul

#endif /* _KUL_OS_WIN_OS_TOP_HPP_ */
