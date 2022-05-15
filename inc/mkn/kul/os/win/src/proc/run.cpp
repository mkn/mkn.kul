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

// void mkn::kul::Process::run() KTHROW(mkn::kul::proc::Exception){

SECURITY_ATTRIBUTES sa;
ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
// Set the bInheritHandle flag so pipe handles are inherited.
sa.nLength = sizeof(SECURITY_ATTRIBUTES);
sa.bInheritHandle = TRUE;
sa.lpSecurityDescriptor = NULL;

std::stringstream ss;
ss << this;

const ULONG &pipeID = PIPE_ID();
std::string pipeOut = "\\\\.\\Pipe\\kul_proc_out." + std::to_string(this_proc::id()) + "." +
                      ss.str() + "." + std::to_string(pipeID);
std::string pipeErr = "\\\\.\\Pipe\\kul_proc_err." + std::to_string(this_proc::id()) + "." +
                      ss.str() + "." + std::to_string(pipeID);
std::string pipeIn = "\\\\.\\Pipe\\kul_proc_in." + std::to_string(this_proc::id()) + "." +
                     ss.str() + "." + std::to_string(pipeID);

LPSTR lPipeOut = _strdup(pipeOut.c_str());
g_hChildStd_OUT_Wr =
    ::CreateNamedPipeA(lPipeOut, PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1,
                       __MKN_KUL_PROCESS_BUFFER__, __MKN_KUL_PROCESS_BUFFER__, 0, &sa);
if (!g_hChildStd_OUT_Wr) error(__LINE__, "CreatePipe failed");
g_hChildStd_OUT_Rd = ::CreateFileA(lPipeOut, GENERIC_READ, 0, &sa, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
if (!g_hChildStd_OUT_Rd) error(__LINE__, "CreatePipe failed");
if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
  error(__LINE__, "SetHandleInformation failed");

LPSTR lPipeErr = _strdup(pipeErr.c_str());
g_hChildStd_ERR_Wr =
    ::CreateNamedPipeA(lPipeErr, PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1,
                       __MKN_KUL_PROCESS_BUFFER__, __MKN_KUL_PROCESS_BUFFER__, 0, &sa);
if (!g_hChildStd_ERR_Wr) error(__LINE__, "CreatePipe failed");
g_hChildStd_ERR_Rd = ::CreateFileA(lPipeErr, GENERIC_READ, 0, &sa, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
if (!g_hChildStd_ERR_Rd) error(__LINE__, "CreatePipe failed");
if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
  error(__LINE__, "SetHandleInformation failed");

bool bSuccess = FALSE;

PROCESS_INFORMATION piProcInfo;
STARTUPINFO siStartInfo;

ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

siStartInfo.cb = sizeof(STARTUPINFO);
siStartInfo.dwFlags = STARTF_USESTDHANDLES;
siStartInfo.hStdError = g_hChildStd_ERR_Wr;
siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
siStartInfo.wShowWindow = SW_HIDE;
siStartInfo.lpDesktop = NULL;
siStartInfo.lpReserved = NULL;
siStartInfo.lpTitle = NULL;
siStartInfo.cbReserved2 = 0;
siStartInfo.lpReserved2 = NULL;

unsigned flags = CREATE_UNICODE_ENVIRONMENT;
HANDLE cons = CreateFile("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL, NULL);
if (cons == INVALID_HANDLE_VALUE) {
  flags |= DETACHED_PROCESS;
} else {
  CloseHandle(cons);
}

preStart();

LPTSTR lpszVariable;
LPCH lpvEnv = GetEnvironmentStrings();
if (lpvEnv == NULL) error(__LINE__, "GetEnvironmentStrings() failed.");
kul::hash::map::S2S env;
for (lpszVariable = (LPTSTR)lpvEnv; *lpszVariable; lpszVariable++) {
  std::stringstream ss;
  while (*lpszVariable) ss << *lpszVariable++;
  std::string var = ss.str();
  if (var.find("=") != std::string::npos)
    env.insert(var.substr(0, var.find("=")), var.substr(var.find("=") + 1));
}
if (FreeEnvironmentStrings(lpvEnv) == 0) error(__LINE__, "FreeEnvironmentStrings() failed");

char const *dir = directory().empty() ? 0 : directory().c_str();
std::string cmd(toString());

expand(cmd);
LPSTR szCmdline = _strdup(cmd.c_str());

if (vars().size()) {
  std::wstring newEnv;

  for (auto &evs : vars()) env[evs.first] = evs.second;

  for (auto &evs : env) {
    std::string name = evs.first;
    std::string var(name + "=" + evs.second);
    newEnv += std::wstring(var.begin(), var.end());
    newEnv.push_back('\0');
  }
  // may not work
  bSuccess = CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, flags, (LPVOID)newEnv.c_str(), dir,
                           &siStartInfo, &piProcInfo);
} else
  bSuccess =
      CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, flags, NULL, dir, &siStartInfo, &piProcInfo);

if (!bSuccess) error(__LINE__, "CreateProcess failed with last error: " + GetLastError());

pid(piProcInfo.dwProcessId);

CloseHandle(g_hChildStd_OUT_Wr);
CloseHandle(g_hChildStd_ERR_Wr);

if (this->waitForExit()) {
  OVERLAPPED il = {0};
  memset(&il, 0, sizeof(il));
  il.hEvent = revent;
  OVERLAPPED ol = {0};
  memset(&ol, 0, sizeof(ol));
  ol.hEvent = revent;
  OVERLAPPED el = {0};
  memset(&el, 0, sizeof(el));
  el.hEvent = revent;

  DWORD dwRead;
  CHAR chBuf[__MKN_KUL_PROCESS_BUFFER__ + 1];
  bSuccess = FALSE;
  bool alive = true;
  do {
#if defined(_MKN_KUL_PROC_LOOP_NSLEEP_) && (_MKN_KUL_PROC_LOOP_NSLEEP_ > 0)
    mkn::kul::this_thread::nSleep(_MKN_KUL_PROC_LOOP_NSLEEP_);
#endif
    alive = WaitForSingleObject(piProcInfo.hProcess, 11) == WAIT_TIMEOUT;
    for (;;) {
      dwRead = 0;
      bSuccess = ::ReadFile(g_hChildStd_OUT_Rd, chBuf, __MKN_KUL_PROCESS_BUFFER__, &dwRead, &ol);
      while (!bSuccess &&
             (GetLastError() == ERROR_IO_PENDING || GetLastError() == ERROR_IO_INCOMPLETE)) {
        WaitForSingleObject(ol.hEvent, 11);
        bSuccess = GetOverlappedResult(g_hChildStd_OUT_Rd, &ol, &dwRead, 0);
      }
      if (!bSuccess || dwRead == 0) break;
      chBuf[dwRead] = '\0';
      out(std::string(chBuf, dwRead));
    }
    for (;;) {
      dwRead = 0;
      bSuccess = ::ReadFile(g_hChildStd_ERR_Rd, chBuf, __MKN_KUL_PROCESS_BUFFER__, &dwRead, &el);
      if (GetLastError() == ERROR_IO_PENDING) {
        WaitForSingleObject(el.hEvent, 11);
        bSuccess = GetOverlappedResult(g_hChildStd_OUT_Rd, &el, &dwRead, 0);
      }
      if (!bSuccess || dwRead == 0) break;
      chBuf[dwRead] = '\0';
      err(std::string(chBuf, dwRead));
    }
  } while (alive);
}
tearDown();
if (this->waitForExit()) {
  DWORD ec = 0;
  if (FALSE == GetExitCodeProcess(piProcInfo.hProcess, &ec))
    KEXCEPT(mkn::kul::proc::Exception, "GetExitCodeProcess failure");
  exitCode(ec);
  finish();
  setFinished();
}
CloseHandle(piProcInfo.hThread);
CloseHandle(piProcInfo.hProcess);

// }