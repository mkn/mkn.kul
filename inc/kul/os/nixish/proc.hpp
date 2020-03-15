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
#ifndef _KUL_OS_NIXISH_PROC_HPP_
#define _KUL_OS_NIXISH_PROC_HPP_

#include <queue>
#include <memory>
#include <string>
#include <sstream>

#include "kul/os.hpp"
#include "kul/log.hpp"

#if   KUL_IS_NIX
#include "kul/os/nix/proc.os.hpp"
#elif KUL_IS_BSD
#include "kul/os/bsd/proc.os.hpp"
#else
#error unresolved
#endif


#ifndef __KUL_PROC_DUP_RETRY__
#define __KUL_PROC_DUP_RETRY__ 3
#endif  //__KUL_PROC_DUP_RETRY__

namespace kul {

namespace this_proc {
inline int32_t id() { return getpid(); }
inline void kill(const int32_t &e) { ::kill(kul::this_proc::id(), e); }
}  // namespace this_proc

class Process : public kul::AProcess {
 private:
  int inFd[2], outFd[2], errFd[2], popPip[3];
  int cStat;  // child status

  inline int16_t recall(const uint16_t &s) {
    int ret;
    while ((ret = (s)) < 0x0 && (errno == EINTR)) {
    }
    return ret;
  }

 public:
  Process(const std::string &cmd, const bool &wfe = true) : kul::AProcess(cmd, wfe) {}
  Process(const std::string &cmd, const std::string &path, const bool &wfe = true)
      : kul::AProcess(cmd, path, wfe) {}
  Process(const std::string &cmd, const kul::Dir &d, const bool &wfe = true)
      : kul::AProcess(cmd, (d ? d.real() : d.path()), wfe) {}
  bool kill(int16_t k = 6) {
    if (started()) {
      bool b = ::kill(pid(), k) == 0;
      if (::kill(pid(), 0) == 0) setFinished();
      return b;
    }
    return 0;
  }

 protected:
  int16_t inline child();
  virtual void inline expand(std::string &s) const;
  void inline waitForStatus();
  void inline waitExit() KTHROW(kul::proc::ExitException);
  void inline tearDown();
  void inline run() KTHROW(kul::proc::Exception);

  virtual void finish() {}
  virtual void preStart() {}
};
}  // namespace kul


#ifndef _KUL_COMPILED_LIB_
#include "kul/os/nixish/src/proc/child.ipp"
#include "kul/os/nixish/src/proc/expand.ipp"
#include "kul/os/nixish/src/proc/waitForStatus.ipp"
#include "kul/os/nixish/src/proc/waitExit.ipp"
#include "kul/os/nixish/src/proc/tearDown.ipp"
#include "kul/os/nixish/src/proc/run.ipp"
#endif

#endif /* _KUL_OS_NIXISH_PROC_HPP_ */
