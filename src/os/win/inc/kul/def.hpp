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
#ifndef _KUL_DEF_HPP_
#define _KUL_DEF_HPP_

#include "kul/defs.hpp"

#ifndef __KUL_OS__
#define __KUL_OS__ win
#endif /*  __KUL_OS__ */

#ifndef __KUL_THREAD_SPAWN_WAIT__
#define __KUL_THREAD_SPAWN_WAIT__ 0
#endif /*  __KUL_THREAD_SPAWN_WAIT__ */

#ifndef __KUL_ENV_BUFFER__
#define __KUL_ENV_BUFFER__ 1024
#endif /*  __KUL_ENV_BUFFER__ */

#ifndef __KUL_PROCESS_BUFFER__
#define __KUL_PROCESS_BUFFER__ 1024
#endif /*  __KUL_PROCESS_BUFFER__ */

#ifndef __KUL_PROCESS_ENV_BUFFER__
#define __KUL_PROCESS_ENV_BUFFER__ 4096
#endif /*  __KUL_PROCESS_ENV_BUFFER__ */

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

#endif /* _KUL_DEF_HPP_ */
