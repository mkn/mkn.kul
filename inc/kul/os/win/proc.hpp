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
#ifndef _KUL_OS_WIN_PROC_HPP_
#define _KUL_OS_WIN_PROC_HPP_

#include <Windows.h>
#include <psapi.h>
#include <stdio.h>
#include <strsafe.h>
#include <tchar.h>
#include <sstream>
#include <string>

#include "kul/defs.hpp"
#include "kul/log.hpp"
#include "kul/os.hpp"
#include "kul/proc.base.hpp"
#include "kul/string.hpp"


// extern char **environ;

namespace kul {

namespace this_proc {
class MemGetter {
 private:
  PROCESS_MEMORY_COUNTERS_EX pmc;
  MemGetter() {
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
  }
  void virtula(uint64_t &v) { v += pmc.PrivateUsage; }
  void physical(uint64_t &v) { v += pmc.WorkingSetSize; }
  friend uint64_t virtualMemory();
  friend uint64_t physicalMemory();
  friend uint64_t totalMemory();
};
inline uint64_t virtualMemory() {
  uint64_t v = 0;
  MemGetter().virtula(v);
  return v / 1000;
}
inline uint64_t physicalMemory() {
  uint64_t v = 0;
  MemGetter().physical(v);
  return v / 1000;
}
inline uint64_t totalMemory() {
  uint64_t v = 0;
  MemGetter pg;
  pg.virtula(v);
  pg.physical(v);
  return v / 1000;
}

inline uint16_t cpuLoad() { return 0; }

inline int32_t id() { return GetCurrentProcessId(); }
inline void kill(const int32_t &e) {
  TerminateProcess(OpenProcess(PROCESS_TERMINATE, 0, kul::this_proc::id()), 128 + e);
}
}  // namespace this_proc

class Process : public kul::AProcess {
 private:
  static ULONG PIPE_ID() {
    static ULONG p = 999;
    p++;
    return p;
  }
  HANDLE g_hChildStd_OUT_Rd = NULL;
  HANDLE g_hChildStd_OUT_Wr = NULL;
  HANDLE g_hChildStd_ERR_Rd = NULL;
  HANDLE g_hChildStd_ERR_Wr = NULL;

  HANDLE revent = CreateEvent(0, 1, 0, 0);

 public:
  Process(const std::string &cmd, const bool &wfe = true) : kul::AProcess(cmd, wfe) {}
  Process(const std::string &cmd, const std::string &path, const bool &wfe = true)
      : kul::AProcess(cmd, path, wfe) {}
  Process(const std::string &cmd, const kul::Dir &d, const bool &wfe = true)
      : kul::AProcess(cmd, d ? d.real() : d.path(), wfe) {}
  ~Process() { tearDown(); }
  bool kill(int16_t k = 6) {
    if (!started()) return 0;
    DWORD dwDesiredAccess = PROCESS_TERMINATE;
    bool bInheritHandle = 0;
    HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, pid());
    if (hProcess == NULL) return 0;
    bool r = TerminateProcess(hProcess, 128 + k);
    CloseHandle(hProcess);
    setFinished();
    return r;
  }

 protected:
#ifndef _KUL_COMPILED_LIB_
  void tearDown() {
#include "kul/os/win/src/proc/tearDown.cpp"
  }
  virtual void expand(std::string &s) const {
#include "kul/os/win/src/proc/expand.cpp"
  }
  void run() KTHROW(kul::Exception) {
#include "kul/os/win/src/proc/run.cpp"
  }
#else
  void tearDown();
  virtual void expand(std::string &s) const;
  void run() KTHROW(kul::Exception);
#endif
};
}  // namespace kul

#endif /* _KUL_OS_WIN_PROC_HPP_ */
