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
#ifndef _KUL_OS_WIN_THREADS_OS_HPP_
#define _KUL_OS_WIN_THREADS_OS_HPP_



#include <TlHelp32.h>
#include <Windows.h>

namespace kul {
namespace this_thread {
inline const std::string id() {
  std::ostringstream os;
  os << std::hex << std::hash<std::thread::id>()(std::this_thread::get_id());
  return os.str();
}

#ifndef _KUL_COMPILED_LIB_
inline bool main() {
#include "kul/os/win/src/thread/main.cpp"

}
#else
bool main();
#endif

inline void kill() {
  HANDLE h = GetCurrentThread();
  TerminateThread(h, 0);
  CloseHandle(h);
}
}  // END NAMESPACE this_thread

class Mutex {
 private:
  CRITICAL_SECTION critSec;

 public:
  Mutex() { InitializeCriticalSection(&critSec); }
  ~Mutex() { DeleteCriticalSection(&critSec); }
  void lock() { EnterCriticalSection(&critSec); }
  void unlock() { LeaveCriticalSection(&critSec); }
};

namespace threading {
DWORD WINAPI threadFunction(LPVOID th);
}

class Thread : public threading::AThread {
 private:
  std::function<void()> func;
  HANDLE h;
  friend DWORD WINAPI threading::threadFunction(LPVOID);
  void act() {
    try {
      func();
    } catch (const std::exception &e) {
      ep = std::current_exception();
    }
    f = 1;
  }

 public:
  Thread(const std::function<void()> &func) : func(func) {}
  template <class T>
  Thread(const T &t) : func(std::bind((void (T::*)()) & T::operator(), t)) {}
  template <class T>
  Thread(const std::reference_wrapper<T> &r)
      : func(std::bind((void (T::*)()) & T::operator(), r)) {}
  template <class T>
  Thread(const std::reference_wrapper<const T> &r)
      : func(std::bind((void (T::*)() const) & T::operator(), r)) {}
  virtual ~Thread() {}
  void join() {
    if (!s) run();
    WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
    s = 0;
  }
  bool detach() { return CloseHandle(h); }
  void interrupt() KTHROW(kul::threading::InterruptionException) {
    TerminateThread(h, 1);
    f = 1;
  }
  void run() KTHROW(kul::threading::Exception) {
    if (s) KEXCEPTION("Thread running");
    f = 0;
    s = 1;
    h = CreateThread(0, 5120000, threading::threadFunction, this, 0, 0);
  }
};

namespace threading {
inline DWORD WINAPI threadFunction(LPVOID th) {
  reinterpret_cast<Thread *>(th)->act();
  return 0;
}
}  // namespace threading

}  // END NAMESPACE kul
#endif /* _KUL_OS_WIN_THREADS_OS_HPP_ */
