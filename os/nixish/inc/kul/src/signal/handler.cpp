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

// This file is included by other files and is not in itself syntactically
// correct.

// void kul_sig_handler(int s, siginfo_t* info, void* v) {

if (info->si_pid == 0 || info->si_pid == kul::this_proc::id()) {
  // if(s == SIGABRT) for(auto& f : kul::SignalStatic::INSTANCE().ab ) f(s);
  if (s == SIGINT)
    for (auto &f : kul::SignalStatic::INSTANCE().in)
      f(s);
  if (s == SIGSEGV)
    for (auto &f : kul::SignalStatic::INSTANCE().se)
      f(s);
#ifdef HAVE_EXECINFO_H
  if (s == SIGSEGV && !kul::SignalStatic::INSTANCE().q) {
    ucontext_t *uc = (ucontext_t *)v;
    void *trace[16];
    char **messages = (char **)NULL;
    int16_t i, trace_size = 0;
    trace_size = backtrace(trace, 16);
#if defined(__arm__)
    trace[1] = (void *)uc->uc_mcontext.arm_r0;
#elif defined(__APPLE__)
    trace[1] = (void *)uc->uc_mcontext->__ss.__rip;
#elif defined(__NetBSD__)
    trace[1] = (void *)uc->uc_mcontext.__gregs[REG_EIP];
#elif defined(__FreeBSD__)
#if (__x86_64__)
    trace[1] = (void *)uc->uc_mcontext.mc_rip;
#else
    trace[1] = (void *)uc->uc_mcontext.mc_eip;
#endif /* __x86_64__ */
#else
    trace[1] = (void *)uc->uc_mcontext.gregs[REG_EIP];
#endif
    messages = backtrace_symbols(trace, trace_size);
    printf("[bt] Stacktrace:\n");
    for (i = 2; i < trace_size; ++i) {
      printf("[bt] %s : ", messages[i]);
      size_t p = 0;
      while (messages[i][p] != '(' && messages[i][p] != ' ' &&
             messages[i][p] != 0)
        ++p;
      std::string str(messages[i]);
      str = str.substr(0, p);
      if (kul::SignalStatic::INSTANCE().addr) {
        kul::Process p("addr2line");
        kul::ProcessCapture pc(p);
        p.arg(trace[i]).arg("-e").arg(str).start();
        printf("%s", pc.outs().c_str());
      }
    }
  }
#endif /* HAVE_EXECINFO_H */
  exit(s);
}

// }