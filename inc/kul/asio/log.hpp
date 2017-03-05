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
#ifndef _KUL_ASIO_LOG_HPP_
#define _KUL_ASIO_LOG_HPP_

#include "kul/log.hpp"
#include "kul/threads.hpp"

namespace kul{ namespace asio { namespace log{

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};
} // END NAMESPACE log

class LogMan;
class Logger : public kul::Logger{
    friend class LogMan;
    private:
        kul::ChroncurrentThreadPool<> ctp;
        std::function<void(const std::string&)> defE, defO;
    public:
    	Logger() : 
    			ctp(1, 1),
    			defE([&](const std::string& s){ kul::Logger::err(s); }),
    			defO([&](const std::string& s){ kul::Logger::out(s); }){
    	}
        void err(const std::string& s) override{
            if(e) ctp.async(std::bind(e, s)); 
            else  ctp.async(std::bind(defE, s)); 
        }
        void out(const std::string& s) override{
            if(o) ctp.async(std::bind(o, s));
            else  ctp.async(std::bind(defO, s));
        }
};

class LogMan : public kul::ALogMan{
    protected:
        LogMan() : ALogMan(new kul::asio::Logger()){}
    public:
        static LogMan& INSTANCE(){
            static LogMan instance;
            return instance;
        };
};

class Message{
    protected:
        std::stringstream ss;
        const kul::log::mode& m;

        Message(const kul::log::mode& m) : m(m){}
    public:
        template<class T> 
        Message& operator<<(const T& s){
            ss << s;
            return *this;
        }
};
class LogMessage : public Message{
    private:
        const char* f;
        const char* fn;
        const uint16_t& l;
    public:     
        ~LogMessage(){
            LogMan::INSTANCE().log(f, fn, l, m, ss.str());
        }
        LogMessage(const char* f, const char* fn, const uint16_t& l, const kul::log::mode& m) : Message(m), f(f), fn(fn), l(l){}
};
class OutMessage : public Message{
    public:
        ~OutMessage(){
            LogMan::INSTANCE().out(m, ss.str());
        }
        OutMessage(const kul::log::mode& m = kul::log::mode::NON) : Message(m){}
};
class ErrMessage : public Message{
    public:
        ~ErrMessage(){
            LogMan::INSTANCE().err(m, ss.str());
        }
        ErrMessage() : Message(kul::log::mode::ERR){}
};

#define KASIO_LOG_INF    kul::asio::LogMessage(__FILE__, __func__, __LINE__, kul::log::mode::INF)
#define KASIO_LOG_ERR    kul::asio::LogMessage(__FILE__, __func__, __LINE__, kul::log::mode::ERR)
#define KASIO_LOG_DBG    kul::asio::LogMessage(__FILE__, __func__, __LINE__, kul::log::mode::DBG)
#define KASIO_LOG_TRC    kul::asio::LogMessage(__FILE__, __func__, __LINE__, kul::log::mode::TRC)
#define KASIO_LOG(sev) KLOG_ ## sev

#define KASIO_OUT_NON    kul::asio::OutMessage()
#define KASIO_OUT_INF    kul::asio::OutMessage(kul::log::mode::INF)
#define KASIO_OUT_ERR    kul::asio::OutMessage(kul::log::mode::ERR)
#define KASIO_OUT_DBG    kul::asio::OutMessage(kul::log::mode::DBG)
#define KASIO_OUT_TRC    kul::asio::OutMessage(kul::log::mode::TRC)
#define KASIO_OUT(sev) KASIO_OUT_ ## sev

#define KASIO_ERR        kul::ErrMessage()

} // END NAMESPACE asio
} // END NAMESPACE kul
#endif /* _KUL_LOG_HPP_ */
