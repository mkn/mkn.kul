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
#ifndef _KUL_SCM_MAN_HPP_
#define _KUL_SCM_MAN_HPP_
 
#include "kul/scm.hpp"

namespace kul{ namespace scm{

// "make_unique is not yet ubiquitous"

class Manager{
    private:
        Manager(){
            git.reset(new Git());
            svn.reset(new Svn());
            SCMs.insert(std::pair<std::string, SCM*>("git", git.get()));
            SCMs.insert(std::pair<std::string, SCM*>("svn", svn.get()));
        }
        hash::map::S2T<SCM*> SCMs;
        std::unique_ptr<SCM> git;
        std::unique_ptr<SCM> svn;
    public:
        static Manager& INSTANCE(){ 
            static Manager instance; 
            return instance;
        }
        const SCM& get(const std::string& s) throw(NotFoundException){
            if(SCMs.count(s) > 0) return *(*SCMs.find(s)).second;
            KEXCEPT(NotFoundException, "Source Control Management for " + s + " is not implemented");
        }
};

}// END NAMESPACE scm
}// END NAMESPACE kul
#endif /* _KUL_SCM_MAN_HPP_ */
