/**
Copyright (c) 2024, Philip Deegan.
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

// bool mkn::kul::this_thread::main(){

const std::shared_ptr<void> hThreadSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0),
                                            CloseHandle);
if (hThreadSnapshot.get() == INVALID_HANDLE_VALUE)
  KEXCEPT(mkn::kul::threading::Exception, "GetMainThreadId failed");
THREADENTRY32 tEntry;
tEntry.dwSize = sizeof(THREADENTRY32);
DWORD result = 0;
DWORD currentPID = GetCurrentProcessId();
for (BOOL success = Thread32First(hThreadSnapshot.get(), &tEntry);
     !result && success && GetLastError() != ERROR_NO_MORE_FILES;
     success = Thread32Next(hThreadSnapshot.get(), &tEntry))
  if (tEntry.th32OwnerProcessID == currentPID) result = tEntry.th32ThreadID;

std::stringstream ss;
ss << std::this_thread::get_id();
return std::to_string(result) == ss.str();

// }
