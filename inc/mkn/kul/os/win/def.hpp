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
// IWYU pragma: private, include "mkn/kul/defs.hpp"

#ifndef MKN_KUL_OS_WIN_DEF_HPP_
#define MKN_KUL_OS_WIN_DEF_HPP_

#ifndef MKN_KUL_OS
#define MKN_KUL_OS win
#endif /*  MKN_KUL_OS */

#ifndef MKN_KUL_THREAD_SPAWN_WAIT
#define MKN_KUL_THREAD_SPAWN_WAIT 0
#endif /*  MKN_KUL_THREAD_SPAWN_WAIT */

#ifndef MKN_KUL_ENV_BUFFER
#define MKN_KUL_ENV_BUFFER 1024
#endif /*  MKN_KUL_ENV_BUFFER */

#ifndef MKN_KUL_PROCESS_ENV_BUFFER
#define MKN_KUL_PROCESS_ENV_BUFFER 4096 * 2
#endif /*  MKN_KUL_PROCESS_ENV_BUFFER */

#pragma warning(disable : 4005)
#pragma warning(disable : 4101)
#pragma warning(disable : 4290)
#pragma warning(disable : 4503)
#pragma warning(disable : 4800)

#ifndef INTMAX_MAX
#define INTMAX_MAX _I64_MAX
#endif /*  INTMAX_MAX */
#ifndef UINTMAX_MAX
#define UINTMAX_MAX _UI64_MAX
#endif /*  UINTMAX_MAX */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#include <string>

namespace mkn::kul::os {
inline std::string EOL() {
#if (_MSC_VER >= 1800)
  return "\n";
#else
  return "\r\n";
#endif  // _MSC_VER
}
}  // namespace mkn::kul::os

#endif /* MKN_KUL_OS_WIN_DEF_HPP_ */
