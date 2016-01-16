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
#ifndef _KUL_PROC_BASE_HPP_
#define _KUL_PROC_BASE_HPP_

#include <vector>
#include <sstream>
#include <iostream>
#include <functional>

#include "kul/cli.hpp"
#include "kul/map.hpp"
#include "kul/except.hpp"

namespace kul { 

namespace this_proc{
    int32_t id();
    void kill(const uint32_t& e);
}

namespace proc{

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

class ExitException : public kul::proc::Exception{
    private:
        const short ec;
    public:
        ExitException(const char*f, const uint16_t& l, const short ec, const std::string& s) : Exception(f, l, s), ec(ec){}
        const short& code() const { return ec; }
};


class Call{
    private:
        std::string cwd;
        const std::string d;
        const std::string& s;
        kul::hash::map::S2S oldEvs;
        void setCWD(){
            if(d.size()){
                cwd = kul::env::CWD();
                if(!kul::env::CWD(d)) KEXCEPTION("FAILED TO SET DIRECTORY: "+ d);
            }
        }
    public:
        ~Call(){
            if(d.size()) kul::env::CWD(cwd);
            for(const std::pair<std::string, std::string>& oldEv : oldEvs)
                kul::env::SET(oldEv.first.c_str(), oldEv.second.c_str());
        }
        Call(const std::string& s, const std::string& d= "") : d(d), s(s){
            setCWD();
        }
        Call(const std::string& s, const kul::hash::map::S2S& evs, const std::string& d= "") : d(d), s(s){
            setCWD();
            for(const auto& ev : evs){
                const std::string& v = kul::env::GET(ev.first.c_str());
                if(v.size()) oldEvs.insert(ev.first, v);
                kul::env::SET(ev.first.c_str(), ev.second.c_str());
            }
        }
        int run(){
            return s.size() ? kul::os::exec(s) : 1;
        }
};
}

class AProcess{
    private:
        bool f = 0, s = 0;
        const bool wfe = 1;
        int32_t pec = 0, pi = 0;
        const std::string d;
        std::function<void(std::string)> e;
        std::function<void(std::string)> o;
        std::vector<std::string> argv;
        kul::hash::map::S2S evs;
        friend std::ostream& operator<<(std::ostream&, const AProcess&);
    protected:
        AProcess(const std::string& cmd, const bool& wfe) : wfe(wfe){ argv.push_back(cmd); }
        AProcess(const std::string& cmd, const std::string& d, const bool& wfe) : wfe(wfe), d(d){ argv.push_back(cmd); }
        virtual ~AProcess(){}

        const std::string&  directory()const { return d; }
        void setFinished()  { f = 1; }
        virtual bool kill(int16_t k = 6) = 0;
        virtual void preStart() {}
        virtual void finish()   {}
        virtual void tearDown() {}
        virtual void run() throw (kul::Exception) = 0;
        bool waitForExit()  const { return wfe; }
        void pid(const int32_t& pi )  { this->pi = pi; }

        const std::vector<std::string>&     args()  const { return argv; };
        const kul::hash::map::S2S&          vars()  const { return evs; }
        virtual void out(const std::string& s){
            if(this->o) this->o(s);
            else        printf("%s", s.c_str());
        }
        virtual void err(const std::string& s){
            if(this->e) this->e(s);
            else        fprintf(stderr, "%s", s.c_str());
        }
        void error(const int16_t& line, const std::string& s) throw (kul::Exception){
            tearDown();
            throw Exception("kul/proc.hpp", line, s);
        }
        void exitCode(const int32_t& e){ pec = e; }
    public:
        template <class T> AProcess& arg(const T& a) { 
            std::stringstream ss;
            ss << a;
            if(ss.str().size()) argv.push_back(ss.str());
            return *this; 
        }
        AProcess& arg(const std::string& a) { if(a.size()) argv.push_back(a); return *this; }
        AProcess& var(const std::string& n, const std::string& v) { evs.insert(n, v); return *this;}
        virtual void start() throw(kul::Exception){
            if(this->s) KEXCEPT(kul::proc::Exception, "Process is already started");
            this->s = true;
            if(this->o || this->e) this->run();
            else pec = proc::Call(toString(), evs, d).run();
            if(pec != 0)
                kul::LogMan::INSTANCE().err()
                    ? throw proc::ExitException(__FILE__, __LINE__, pec, "Process exit code: " + std::to_string(pec) + kul::os::EOL() + toString())
                    : throw proc::ExitException(__FILE__, __LINE__, pec, "Process exit code: " + std::to_string(pec));
        }
        const int32_t& pid() const { return pi; }
        bool started()       const { return pi > 0; }
        bool finished()      const { return f; }
        virtual const std::string toString() const{
            std::string s;
            for(const std::string& a : args()) s += a + " ";
            s.pop_back();
            return s;
        }
        void setOut(std::function<void(std::string)> o) { this->o = o; }
        void setErr(std::function<void(std::string)> e) { this->e = e; }
        const int32_t& exitCode(){ return pec; }
};

inline std::ostream& operator<<(std::ostream &s, const AProcess &p){
    return s << p.toString();
}

class ProcessCapture{
    private:
        std::stringstream so;
        std::stringstream se;
    protected:
        ProcessCapture(){}
        ProcessCapture(const ProcessCapture& pc) : so(pc.so.str()), se(pc.se.str()){}
        virtual void out(const std::string& s){
            so << s;
        }
        virtual void err(const std::string& s){
            se << s;
        }
    public:
        ProcessCapture(AProcess& p){
            setProcess(p);
        }
        virtual ~ProcessCapture(){}
        const std::string outs() const { return so.str(); }
        const std::string errs() const { return se.str(); }
        void setProcess(AProcess& p){
            p.setOut(std::bind(&ProcessCapture::out, std::ref(*this), std::placeholders::_1));
            p.setErr(std::bind(&ProcessCapture::err, std::ref(*this), std::placeholders::_1));
        }
};

}
#endif /* _KUL_PROC_BASE_HPP_ */
