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

// This file is included by other files and is not in itself syntactically correct.

inline std::vector<std::string> stacktrace(ucontext_t * /*uc*/ = nullptr, int /*start*/ = 2) {
  // std::vector<std::string> stacktrace(ucontext_t *uc, int start) {
  // constexpr size_t max_trace = 128;
  std::string const invalid = "??:0";

  bool euaddr = mkn::kul::env::WHICH("eu-addr2line");

  std::vector<std::string> v;

  //     void* trace[128];

  //     int addrlen = backtrace(trace, sizeof(trace) / sizeof(void*));

  //     if (addrlen == 0) {
  //       v.emplace_back("backtrace failed");
  //       return v;
  //     }

  //     if(uc){
  // #if defined(__arm__)
  //     trace[1] = (void *)uc->uc_mcontext.arm_r0;
  // #elif defined(__APPLE__)
  //     trace[1] = (void *)uc->uc_mcontext->__ss.__rip;
  // #elif defined(__NetBSD__)
  //     trace[1] = (void *)uc->uc_mcontext.__gregs[REG_EIP];
  // #elif defined(__FreeBSD__)
  // #if (__x86_64__)
  //     trace[1] = (void *)uc->uc_mcontext.mc_rip;
  // #else
  //     trace[1] = (void *)uc->uc_mcontext.mc_eip;
  // #endif /* __x86_64__ */
  // #else
  //     trace[1] = (void *)uc->uc_mcontext.gregs[REG_EIP];
  // #endif
  //     }

  //     char** symbollist = backtrace_symbols(trace, addrlen);
  //     std::string str{symbollist[0]};

  //     size_t funcnamesize = 256;
  //     char* funcname = (char*)malloc(funcnamesize);

  //     for (int i = start; i < addrlen; i++)
  //     {
  //       std::string li{symbollist[i]};

  //       char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

  //       for (char *p = symbollist[i]; *p; ++p)
  //       {
  //           if (*p == '(')      begin_name = p;
  //           else if (*p == '+') begin_offset = p;
  //           else if (*p == ')' && begin_offset) { end_offset = p; break; }
  //       }

  //       if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
  //       {
  //           std::string binf, line;
  //           *begin_name++ = '\0';
  //           *begin_offset++ = '\0';
  //           *end_offset = '\0';

  //           int status = 0;
  //           char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
  //           if (status == 0) {
  //             funcname = ret; // use possibly realloc()-ed string
  //             binf = std::string(symbollist[i]) + ": " + std::string(funcname) + "()";
  //           }
  //           else { // demangling failed. Output function name as a C function with no arguments
  //             binf = std::string(symbollist[i]) + ": " + std::string(begin_name) + "()";
  //           }
  //           std::string address;
  //           {
  //             std::stringstream ss;
  //             ss << std::hex << (((int64_t)(trace[i])));
  //             address = ss.str();
  //           }
  //           if (euaddr) {
  //             mkn::kul::Process p("eu-addr2line");
  //             mkn::kul::ProcessCapture pc(p);
  //             p.arg("-e").arg(symbollist[i]).arg(address).start();
  //             std::string out = pc.outs();
  //             KLOG(INF) << p.toString();
  //             KLOG(INF) << out;
  //             if(out.size() > 3) out = out.substr(0, 4);
  //             if(pc.errs().empty() && out != invalid) line = pc.outs();
  //           }

  //           if(line.size()) line.pop_back();
  //           mkn::kul::String::TRIM(line);
  //           if(line.empty()) v.emplace_back(li);
  //           else             v.emplace_back(line);
  //       }
  //       else
  //       {
  //           v.emplace_back("symbolizing failed");
  //       }
  //     }

  //     free(funcname);
  //     free(symbollist);

  if (!euaddr) {
    v.emplace_back("eu-addr2line not found, install elfutils");
    return v;
  }

  constexpr size_t SIZE = 256;
  int i;
  void *buffer[SIZE];

  int nptrs = backtrace(buffer, SIZE);

  for (i = 1; i < nptrs; ++i) {
    char syscom[1024];
    syscom[0] = '\0';
    snprintf(syscom, 1024, "eu-addr2line '%p' --pid=%d > /dev/stderr\n", buffer[i], getpid());
    if (system(syscom) != 0) fprintf(stderr, "eu-addr2line failed\n");
  }
  return v;
}
