/**
Copyright (c) 2023, Philip Deegan.
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
#ifndef _MKN_KUL_OS_WIN_SIGNAL_HPP_
#define _MKN_KUL_OS_WIN_SIGNAL_HPP_

#include "mkn/kul/log.hpp"

#include <DbgHelp.h>
#include <Windows.h>
#pragma comment(lib, "Dbghelp.lib")

void kul_real_se_handler(EXCEPTION_POINTERS* pExceptionInfo);

LONG WINAPI kul_top_level_exception_handler(PEXCEPTION_POINTERS pExceptionInfo) {
  kul_real_se_handler(pExceptionInfo);
  return (LONG)0L;
}

void kul_se_translator_function(unsigned int sig, EXCEPTION_POINTERS* pExceptionInfo) {
  kul_real_se_handler(pExceptionInfo);
}

void kul_sig_function_handler(uint16_t const& s);

BOOL WINAPI kul_sigint_function(DWORD d) {
  switch (d) {
    case CTRL_C_EVENT:
      kul_sig_function_handler(2);
      break;
    // case CTRL_BREAK_EVENT:
    //     printf("break\n");
    //     break;
    default:
      break;
  }
  return TRUE;
}

namespace mkn {
namespace kul {
namespace this_thread {
std::vector<std::string> stacktrace() {
  // #include "mkn/kul/os/win/src/signal/stacktrace.ipp"
  return std::vector<std::string>{};
}

void print_stacktrace() {
  for (auto const& s : stacktrace()) std::cout << s << std::endl;
}
}  // namespace this_thread

class Signal;

class SignalStatic {
 private:
  bool q = 0;
  std::vector<std::function<void(int)>> ab, in, se;
  friend class Signal;
  friend void ::kul_real_se_handler(EXCEPTION_POINTERS* pExceptionInfo);
  friend void ::kul_sig_function_handler(uint16_t const& s);
  static SignalStatic& INSTANCE() {
    static SignalStatic ss;
    return ss;
  }
  void intr(std::function<void(int)> const& f) {
    if (in.size() == 0)
      if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)kul_sigint_function, TRUE))
        KEXCEPTION("Unable to install signal handler!");
    in.push_back(f);
  }
};

class Signal {
 private:
  bool q = 0;
  std::vector<std::function<void(int)>> ab, in, se;
  friend class Signal;

 public:
  Signal() {
    _set_se_translator(kul_se_translator_function);
    SetUnhandledExceptionFilter(kul_top_level_exception_handler);
  }
  Signal& abrt(std::function<void(int16_t)> const& f) {
    mkn::kul::SignalStatic::INSTANCE().ab.push_back(f);
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

void kul_sig_function_handler(uint16_t const& s) {
  if (s == 2)
    for (auto& f : mkn::kul::SignalStatic::INSTANCE().in) f(s);
  if (s == 11)
    for (auto& f : mkn::kul::SignalStatic::INSTANCE().se) f(s);
}

#ifndef _MKN_KUL_COMPILED_LIB_
void kul_real_se_handler(EXCEPTION_POINTERS* pExceptionInfo) {
#include "mkn/kul/os/win/src/signal/se_handler.cpp"
}
#else
void kul_real_se_handler(EXCEPTION_POINTERS* pExceptionInfo);
#endif

#endif /* _MKN_KUL_OS_WIN_SIGNAL_HPP_ */
