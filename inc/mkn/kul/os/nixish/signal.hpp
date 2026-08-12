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
// IWYU pragma: private, include "mkn/kul/signal.hpp"

#ifndef MKN_KUL_OS_NIXISH_SIGNAL_HPP_
#define MKN_KUL_OS_NIXISH_SIGNAL_HPP_

#include "mkn/kul/defs.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/proc.hpp"

#include <signal.h>

#include <execinfo.h>

#include <cxxabi.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */
#include <ucontext.h>

#ifndef REG_EIP
#ifdef __x86_64__
#define REG_EIP REG_RIP
#endif /* __x86_64__ */
#if defined(__NetBSD__)
#if defined(REG_EIP)
#undef REG_EIP
#endif /* REG_EIP */
#if (__x86_64__)
#define REG_EIP _REG_RIP
#else
#define REG_EIP _REG_EIP
#endif /* __x86_64__ */
#endif /* __NetBSD__ */
#endif /* REG_EIP */

inline void kul_sig_handler(int s, siginfo_t* info, void* v);

namespace mkn {
namespace kul {
namespace this_thread {

inline std::vector<std::string> stacktrace(ucontext_t* /*uc*/ = nullptr, int /*start*/ = 2) {
  bool euaddr = mkn::kul::env::WHICH("eu-addr2line");

  std::vector<std::string> v;

  if (!euaddr) {
    v.emplace_back("eu-addr2line not found, install elfutils");
    return v;
  }

  constexpr size_t SIZE = 256;
  int i;
  void* buffer[SIZE];

  int nptrs = backtrace(buffer, SIZE);

  for (i = 1; i < nptrs; ++i) {
    char syscom[1024];
    syscom[0] = '\0';
    snprintf(syscom, 1024, "eu-addr2line '%p' --pid=%d > /dev/stderr\n", buffer[i], getpid());
    if (system(syscom) != 0) fprintf(stderr, "eu-addr2line failed\n");
  }
  return v;
}

inline void print_stacktrace() {
  for (auto const& s : stacktrace()) std::cout << s << std::endl;
}
}  // namespace this_thread

class Signal;
class SignalStatic {
 private:
  bool addr = 0, euaddr = 0, q = 0;
  struct sigaction sigHandler;
  std::vector<std::function<void(int)>> ab, in, se;
  SignalStatic() {
    addr = mkn::kul::env::WHICH("addr2line");
    euaddr = mkn::kul::env::WHICH("eu-addr2line");
    sigemptyset(&sigHandler.sa_mask);
    sigHandler.sa_flags = SA_SIGINFO;
    sigHandler.sa_sigaction = kul_sig_handler;
    sigaction(SIGSEGV, &sigHandler, NULL);
  }
  static SignalStatic& INSTANCE() {
    static SignalStatic ss;
    return ss;
  }
  void abrt(std::function<void(int)> const& f) {
    if (ab.size() == 0) sigaction(SIGABRT, &sigHandler, NULL);
    ab.push_back(f);
  }
  void intr(std::function<void(int)> const& f) {
    if (in.size() == 0) sigaction(SIGINT, &sigHandler, NULL);
    in.push_back(f);
  }

 public:
  void quiet() { q = 1; }
  friend class Signal;
  friend void ::kul_sig_handler(int s, siginfo_t* i, void* v);
};

class Signal {
 public:
  Signal() { mkn::kul::SignalStatic::INSTANCE(); }
  Signal& abrt(std::function<void(int16_t)> const& f) {
    mkn::kul::SignalStatic::INSTANCE().abrt(f);
    return *this;
  }
  Signal& intr(std::function<void(int16_t)> const& f) {
    mkn::kul::SignalStatic::INSTANCE().intr(f);
    return *this;
  }
  Signal& segv(std::function<void(int16_t)> const& f) {
    mkn::kul::SignalStatic::INSTANCE().se.push_back(f);
    return *this;
  }

  void quiet() { mkn::kul::SignalStatic::INSTANCE().q = 1; }
};
}  // namespace kul
}  // namespace mkn

inline void kul_sig_handler(int s, siginfo_t* info, void* v) {
  if (info->si_pid == 0 || info->si_pid == mkn::kul::this_proc::id()) {
    if (s == SIGABRT)
      for (auto& f : mkn::kul::SignalStatic::INSTANCE().ab) f(s);
    if (s == SIGINT)
      for (auto& f : mkn::kul::SignalStatic::INSTANCE().in) f(s);
    if (s == SIGSEGV)
      for (auto& f : mkn::kul::SignalStatic::INSTANCE().se) f(s);
    if (s == SIGSEGV && !mkn::kul::SignalStatic::INSTANCE().q) {
      auto tid = mkn::kul::this_thread::id();
      ucontext_t* uc = (ucontext_t*)v;
      printf("[bt] Stacktrace:\n");
      for (auto const& st : mkn::kul::this_thread::stacktrace(uc)) KOUT(NON) << tid << " : " << st;
    }
    exit(s);
  }
}

#endif /* MKN_KUL_OS_NIXISH_SIGNAL_HPP_ */
