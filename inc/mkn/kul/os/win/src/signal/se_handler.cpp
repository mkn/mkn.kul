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

// void kul_real_se_handler(EXCEPTION_POINTERS* pExceptionInfo){

std::string const &tid(mkn::kul::this_thread::id());
uint16_t sig = pExceptionInfo->ExceptionRecord->ExceptionCode;
if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
  kul_sig_function_handler(sig = 11);

if (!mkn::kul::SignalStatic::INSTANCE().q) {
  HANDLE process = GetCurrentProcess();
  SymInitialize(process, NULL, TRUE);

  CONTEXT context_record = *pExceptionInfo->ContextRecord;

  STACKFRAME64 stack_frame;
  memset(&stack_frame, 0, sizeof(stack_frame));

#if defined(_ARM_)
  int mach = IMAGE_FILE_MACHINE_ARM;
  stack_frame.AddrPC.Offset = context_record.Pc;
  stack_frame.AddrFrame.Offset = context_record.Sp;
  stack_frame.AddrStack.Offset = context_record.R11;
#elif defined(_ARM64)
  int mach = 0;  // IMAGE_FILE_MACHINE_ARM64;
#elif defined(_WIN64)
  int mach = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = context_record.Rip;
  stack_frame.AddrFrame.Offset = context_record.Rbp;
  stack_frame.AddrStack.Offset = context_record.Rsp;
#else
  int mach = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = context_record.Eip;
  stack_frame.AddrFrame.Offset = context_record.Ebp;
  stack_frame.AddrStack.Offset = context_record.Esp;
#endif
  if (mach == 0) {
    KERR << "CANNOT WALK STACK <> UNSUPPORTED CPU";
    exit(sig);
  };

  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;

  SYMBOL_INFO *symbol;
  symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  std::cout << "[bt] Stacktrace:" << std::endl;
  while (StackWalk64(mach, GetCurrentProcess(), GetCurrentThread(), &stack_frame, &context_record,
                     NULL, &SymFunctionTableAccess64, &SymGetModuleBase64, NULL)) {
    DWORD64 displacement = 0;
    if (SymFromAddr(process, (DWORD64)stack_frame.AddrPC.Offset, &displacement, symbol)) {
      DWORD dwDisplacement;
      IMAGEHLP_LINE64 line;
      IMAGEHLP_MODULE64 moduleInfo;
      ZeroMemory(&moduleInfo, sizeof(IMAGEHLP_MODULE64));
      moduleInfo.SizeOfStruct = sizeof(moduleInfo);

      std::cout << "[bt] ";
      if (::SymGetModuleInfo64(process, symbol->ModBase, &moduleInfo))
        std::cout << moduleInfo.ModuleName << " ";

      std::cout << symbol->Name << " + [0x" << std::hex << displacement << "]";

      if (SymGetLineFromAddr64(process, (DWORD64)stack_frame.AddrPC.Offset, &dwDisplacement, &line))
        std::cout << " - " << line.FileName << ": " << std::to_string(line.LineNumber);
      else
        std::cout << " - ??:";
      std::cout << std::endl;
    }
  }
}
exit(sig);

// }