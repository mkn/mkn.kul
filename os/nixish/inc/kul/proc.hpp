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
#ifndef _KUL_PROC_HPP_
#define _KUL_PROC_HPP_

#include <queue>
#include <memory>
#include <string>
#include <sstream>

#include "kul/os.hpp"
#include "kul/log.hpp"
#include "kul/proc.os.hpp"
#include "kul/threads.base.hpp"

#ifndef __KUL_PROC_DUP_RETRY__
#define __KUL_PROC_DUP_RETRY__ 3
#endif//__KUL_PROC_DUP_RETRY__

namespace kul {

namespace this_proc{
inline int32_t id(){
    return getpid();
}
inline void kill(const int32_t& e){
    ::kill(kul::this_proc::id(), e);
}
}

class Process : public kul::AProcess{
    private:
        int inFd[2];
        int outFd[2];
        int errFd[2];
        int popPip[3];
        int cStat; //child status

        inline int16_t recall(const uint16_t& s){
            int ret; 
            while((ret = (s)) < 0x0 && (errno == EINTR)){}
            return ret;
        }

    public:
        Process(const std::string& cmd, const bool& wfe = true)                         : kul::AProcess(cmd, wfe){}
        Process(const std::string& cmd, const std::string& path, const bool& wfe = true): kul::AProcess(cmd, path, wfe){}
        Process(const std::string& cmd, const kul::Dir& d, const bool& wfe = true) : kul::AProcess(cmd, (d ? d.real() : d.path()), wfe){}
        bool kill(int16_t k = 6){
            if(started()){
                bool b = ::kill(pid(), k) == 0;
                if(::kill(pid(), 0) == 0) setFinished();
                return b;
            }
            return 0;
        }

    protected:
#ifndef _KUL_COMPILED_LIB_
        int16_t child(){
#include "kul/src/proc/child.cpp"
        }
        virtual void expand(std::string& s) const {
#include "kul/src/proc/expand.cpp"
        }
        void waitForStatus(){
#include "kul/src/proc/waitForStatus.cpp"
        }
        void waitExit() KTHROW(kul::proc::ExitException){
#include "kul/src/proc/waitExit.cpp"
        }
        void tearDown(){
#include "kul/src/proc/tearDown.cpp"
        }
        void run() KTHROW(kul::proc::Exception){
#include "kul/src/proc/run.cpp"
        }
#else
        int16_t child();
        virtual void expand(std::string& s) const;
        void waitForStatus();
        void waitExit() KTHROW(kul::proc::ExitException);
        void tearDown();
        void run() KTHROW(kul::proc::Exception);
#endif
        virtual void finish()   { }
        virtual void preStart() { }

};


}
#endif /* _KUL_PROC_HPP_ */
