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
// IWYU pragma: private, include "mkn/kul/defs.hpp"

#ifndef _MKN_KUL_OS_BSD_DEF_HPP_
#define _MKN_KUL_OS_BSD_DEF_HPP_

#include "mkn/kul/defs.hpp"

#ifndef __MKN_KUL_OS__
#define __MKN_KUL_OS__ bsd
#endif /*  __MKN_KUL_OS__ */

#ifndef __MKN_KUL_THREAD_SPAWN_WAIT__
#define __MKN_KUL_THREAD_SPAWN_WAIT__ 5000000
#endif /*  __MKN_KUL_THREAD_SPAWN_WAIT__ */

#if defined(__APPLE__)

#ifndef _MACH_PORT_T
#define _MACH_PORT_T
#include <sys/_types.h> /* __darwin_mach_port_t */
typedef __darwin_mach_port_t mach_port_t;
#include <pthread.h>
mach_port_t pthread_mach_thread_np(pthread_t);
#endif /* _MACH_PORT_T */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif  //_XOPEN_SOURCE

#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#include "sys/types.h"
#undef _DARWIN_C_SOURCE
#else
#include "sys/types.h"
#endif  // _DARWIN_C_SOURCE

#endif /* __APPLE__ */

#endif /* _MKN_KUL_OS_BSD_DEF_HPP_ */
