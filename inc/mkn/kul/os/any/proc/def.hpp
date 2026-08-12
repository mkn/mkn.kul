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
// IWYU pragma: private, include "mkn/kul/proc.hpp"

#ifndef MKN_KUL_OS_ANY_PROC_DEF_HPP_
#define MKN_KUL_OS_ANY_PROC_DEF_HPP_

#include "mkn/kul/except.hpp"

#include <cstdint>

#if defined(_WIN32)
#ifndef MKN_KUL_PROCESS_BUFFER
#define MKN_KUL_PROCESS_BUFFER 1024
#endif /*  MKN_KUL_PROCESS_BUFFER */
#else
#ifndef MKN_KUL_PROCESS_BUFFER
#define MKN_KUL_PROCESS_BUFFER 64000
#endif /*  MKN_KUL_PROCESS_BUFFER */
#endif

namespace mkn::kul::proc {

constexpr int32_t BUFFER_SIZE = MKN_KUL_PROCESS_BUFFER;

class Exception : public kul::Exception {
 public:
  Exception(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class ExitException : public Exception {
 private:
  short const ec;

 public:
  ExitException(char const* f, uint16_t const& l, short const _ec, std::string const& s)
      : Exception(f, l, s), ec(_ec) {}
  short const& code() const { return ec; }
};

}  // namespace mkn::kul::proc

#endif /* MKN_KUL_OS_ANY_PROC_DEF_HPP_ */
