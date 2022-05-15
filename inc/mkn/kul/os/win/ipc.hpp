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
#ifndef _MKN_KUL_OS_WIN_IPC_HPP_
#define _MKN_KUL_OS_WIN_IPC_HPP_

#ifndef _MKN_KUL_IPC_UUID_PREFIX_
#define _MKN_KUL_IPC_UUID_PREFIX_ "\\\\.\\pipe\\"
#endif

#include "mkn/kul/proc.hpp"

#include <conio.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <memory>

#define KUL_IPC_BUFFER 512

namespace mkn {
namespace kul {
namespace ipc {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const *f, uint16_t const &l, std::string const &s)
      : mkn::kul::Exception(f, l, s) {}
};

class Server {
 private:
  int16_t lp;
  const std::string uuid;
  HANDLE hPipe;
  TCHAR *pchRequest = 0;

  void start() KTHROW(Exception) {
    DWORD dwThreadId = 0;
    hPipe = INVALID_HANDLE_VALUE;
    LPTSTR lpszPipename = _strdup(uuid.c_str());
    hPipe = CreateNamedPipe(lpszPipename,                // pipe name
                            PIPE_ACCESS_DUPLEX,          // read/write access
                            PIPE_TYPE_MESSAGE |          // message type pipe
                                PIPE_READMODE_MESSAGE |  // message-read mode
                                PIPE_WAIT,               // blocking mode
                            PIPE_UNLIMITED_INSTANCES,    // max. instances
                            KUL_IPC_BUFFER,              // output buffer size
                            KUL_IPC_BUFFER,              // input buffer size
                            0,                           // client time-out
                            NULL);                       // default security attribute
    if (hPipe == INVALID_HANDLE_VALUE)
      KEXCEPT(mkn::kul::ipc::Exception,
              "CreateNamedPipe failed: " + std::to_string(GetLastError()));
  }

 protected:
  virtual void handle(std::string const &s) { KOUT(INF) << s; }

 public:
  virtual ~Server() {
    if (pchRequest) HeapFree(GetProcessHeap(), 0, pchRequest);
  }
  void listen() KTHROW(Exception) {
    while (lp) {
      HANDLE hHeap = GetProcessHeap();
      pchRequest = (TCHAR *)HeapAlloc(hHeap, 0, KUL_IPC_BUFFER * sizeof(TCHAR));
      bool fConnected =
          ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
      if (!fConnected) continue;
      DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
      if (pchRequest == NULL)
        KEXCEPT(mkn::kul::ipc::Exception,
                "Pipe Server Failure pchRequest: " + std::to_string(GetLastError()));
      bool fSuccess = FALSE;
      while (1) {
        fSuccess = ReadFile(hPipe, pchRequest, KUL_IPC_BUFFER * sizeof(TCHAR), &cbBytesRead, NULL);
        if (!fSuccess || cbBytesRead == 0 && (GetLastError() == ERROR_BROKEN_PIPE)) break;
        handle(pchRequest);
      }
      FlushFileBuffers(hPipe);
      DisconnectNamedPipe(hPipe);
      if (lp != -1) lp--;
    }
    CloseHandle(hPipe);
  }
  Server(const int16_t &lp = -1) KTHROW(Exception)
      : lp(lp),
        uuid(_MKN_KUL_IPC_UUID_PREFIX_ + std::string("pid\\") +
             std::to_string(mkn::kul::this_proc::id())) {
    start();
  }
  Server(std::string const &ui, const int16_t &lp = -1) KTHROW(Exception)
      : uuid(_MKN_KUL_IPC_UUID_PREFIX_ + ui), lp(lp) {
    start();
  }
};

class Client {
 private:
  std::string const uuid;

  HANDLE hPipe;
  void start() KTHROW(Exception) {
    bool fSuccess = FALSE;
    DWORD dwMode;
    LPTSTR lpszPipename = _strdup(uuid.c_str());
    while (1) {
      hPipe = CreateFile(lpszPipename,                  // pipe name
                         GENERIC_READ | GENERIC_WRITE,  // read and write access
                         0,                             // no sharing
                         NULL,                          // default security attributes
                         OPEN_EXISTING,                 // opens existing pipe
                         0,                             // default attributes
                         NULL);                         // no template file

      if (hPipe != INVALID_HANDLE_VALUE) break;

      if (GetLastError() != ERROR_PIPE_BUSY)
        KEXCEPT(mkn::kul::ipc::Exception, "Could not open pipe: " + std::to_string(GetLastError()));

      if (!WaitNamedPipe(lpszPipename, 5000))
        KEXCEPT(mkn::kul::ipc::Exception, "Could not open pipe: 5 second wait timed out.");
    }
    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
    if (!fSuccess)
      KEXCEPT(mkn::kul::ipc::Exception,
              "SetNamedPipeHandleState failed: " + std::to_string(GetLastError()));
  }
  void stop() const KTHROW(Exception) { CloseHandle(hPipe); }

 public:
  virtual ~Client() { stop(); }
  Client(std::string const &ui) KTHROW(Exception) : uuid(_MKN_KUL_IPC_UUID_PREFIX_ + ui) {
    start();
  }
  Client(const int16_t &pid) KTHROW(Exception)
      : uuid(_MKN_KUL_IPC_UUID_PREFIX_ + std::string("pid\\") + std::to_string(pid)) {
    start();
  }
  virtual void send(std::string const &m) const KTHROW(Exception) {
    DWORD cbToWrite, cbWritten;
    LPTSTR lpvMessage = _strdup(m.c_str());
    cbToWrite = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR);
    if (!WriteFile(hPipe, lpvMessage, cbToWrite, &cbWritten, NULL))
      KEXCEPT(mkn::kul::ipc::Exception,
              "WriteFile to pipe failed: " + std::to_string(GetLastError()));
  }
};

}  // namespace ipc
}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_OS_WIN_IPC_HPP_ */
