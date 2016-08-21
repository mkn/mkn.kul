/**
Copyright (c) 2013, Philip Deegan.
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
#ifndef _KUL_PROC_OS_HPP_
#define _KUL_PROC_OS_HPP_

#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdexcept>
#include <sys/wait.h>

#include "kul/proc.base.hpp"

namespace kul { namespace this_proc{

class ProcParser{
    private:

#ifndef _KUL_COMPILED_LIB_
        static int PARSE_LINE(char* line){
#include "kul/src/proc/xparse_line.cpp"
        }
        static void VIRTUAL(uint64_t& mem){
#include "kul/src/proc/xvirtual.cpp"
        }
        static void PHYSICAL(uint64_t& mem){ //Note: this value is in KB!
#include "kul/src/proc/xphysical.cpp"
        }
#else
        static int PARSE_LINE(char* line);
        static void VIRTUAL(uint64_t& mem);
        static void PHYSICAL(uint64_t& mem); //Note: this value is in KB!
#endif

        friend uint64_t virtualMemory();
        friend uint64_t physicalMemory();
        friend uint64_t totalMemory();
};


inline uint64_t virtualMemory(){
    uint64_t v = 0;
    ProcParser::VIRTUAL(v);
    return v;
}
inline uint64_t physicalMemory(){
    uint64_t v = 0;
    ProcParser::PHYSICAL(v);
    return v;
}
inline uint64_t totalMemory(){
    uint64_t v = 0;
    ProcParser::VIRTUAL(v);
    ProcParser::PHYSICAL(v);
    return v;
}
inline uint16_t cpuLoad(){
    return 0;
}

}
}
#endif /* _KUL_PROC_OS_HPP_ */
