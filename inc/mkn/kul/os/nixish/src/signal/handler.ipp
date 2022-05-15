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
// IWYU pragma: private, include "mkn/kul/signal.hpp"

void kul_sig_handler(int s, siginfo_t *info, void *v) {
  if (info->si_pid == 0 || info->si_pid == mkn::kul::this_proc::id()) {
    if (s == SIGABRT)
      for (auto &f : mkn::kul::SignalStatic::INSTANCE().ab) f(s);
    if (s == SIGINT)
      for (auto &f : mkn::kul::SignalStatic::INSTANCE().in) f(s);
    if (s == SIGSEGV)
      for (auto &f : mkn::kul::SignalStatic::INSTANCE().se) f(s);
    if (s == SIGSEGV && !mkn::kul::SignalStatic::INSTANCE().q) {
      auto tid = mkn::kul::this_thread::id();
      ucontext_t *uc = (ucontext_t *)v;
      printf("[bt] Stacktrace:\n");
      for (auto const &st : mkn::kul::this_thread::stacktrace(uc)) KOUT(NON) << tid << " : " << st;
    }
    exit(s);
  }
}
