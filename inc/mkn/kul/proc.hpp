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
#ifndef MKN_KUL_PROC_HPP_
#define MKN_KUL_PROC_HPP_

#ifndef MKN_KUL_PROC_LOOP_NSLEEP_
#define MKN_KUL_PROC_LOOP_NSLEEP_ 1000000
#endif /* MKN_KUL_PROC_LOOP_NSLEEP_ */

#include "mkn/kul/os/any/proc/def.hpp"

#include <cstdio>
// #include <vector>
// #include <sstream>
// #include <iostream>
// #include <functional>

namespace mkn {
namespace kul {
namespace this_proc {

int32_t id();
void kill(uint32_t const& e);

uint64_t virtualMemory();
uint64_t physicalMemory();
uint64_t totalMemory();
uint16_t cpuLoad();
}  // namespace this_proc

}  // namespace kul
}  // namespace mkn

#if defined(_WIN32)
#include "mkn/kul/os/win/proc.hpp"
#else
#include "mkn/kul/os/nixish/proc.hpp"
#endif

#endif  // MKN_KUL_PROC_HPP_