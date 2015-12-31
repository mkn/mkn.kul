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
#ifndef _KUL_LOG_HPP_
#define _KUL_LOG_HPP_

#include <memory>
#include <string>
#include <string.h>

#include "kul/os.hpp"
#include "kul/def.hpp"
#include "kul/time.hpp"
#include "kul/except.hpp"
#include "kul/string.hpp"
#include "kul/threads.os.hpp"

#ifndef __KUL_LOG_TIME_FRMT__
#define __KUL_LOG_TIME_FRMT__ "%Y-%m-%d-%H:%M:%S:%i"
#endif

#ifndef __KUL_LOG_FRMT__
#define __KUL_LOG_FRMT__ "[%M] : %T - %D : %F : %L - %S"
#endif

namespace kul{ namespace log{

enum mode { NON = 0, INF, ERR, DBG};

class Exception : public kul::Exception{
	public:
		Exception(const char*f, const int l, const std::string& s) : kul::Exception(f, l, s){}
};
} // END NAMESPACE log

class Logger{
	public:
		void out(const std::string& s, const log::mode& m) const{
			if(m != log::ERR) 
				printf("%s", s.c_str());
			else
				fprintf(stderr, "%s", s.c_str());
		}
		void log(const char* f, const int& l, const std::string& s, const log::mode& m) const{
			std::string mode(modeTxt(m));
			std::string tr(kul::this_thread::id());
			std::string str(__KUL_LOG_FRMT__);
			kul::String::replace(str, "%M", mode);
			kul::String::replace(str, "%T", tr);
			kul::String::replace(str, "%D", kul::DateTime::NOW(__KUL_LOG_TIME_FRMT__));
			kul::String::replace(str, "%F", f);
			kul::String::replace(str, "%L", std::to_string(l));
			kul::String::replace(str, "%S", s);
			str += kul::os::EOL();
			out(str, m);
		}
		const std::string modeTxt(const log::mode& m) const{
			std::string s("NON");
			if(m == 1)		s = "INF";
			else if(m == 2)	s = "ERR";
			else if(m == 3) s = "DBG";
			return s;
		}
};

class LogMan{
	private:
		log::mode m;
		const Logger logger;
		LogMan() : m(kul::log::mode::NON), logger(){
			const char* klog = kul::env::GET("KLOG");
			if(klog){
				bool e = 0;
				std::string s(klog);
				if(s.empty()) e = 1;
				else{
					kul::String::trim(s);
					if(s.compare("0") == 0 || s.compare("NON") == 0)      m = log::mode::NON;
					else if(s.compare("1") == 0 || s.compare("INF") == 0) m = log::mode::INF;
					else if(s.compare("2") == 0 || s.compare("ERR") == 0) m = log::mode::ERR;
					else if(s.compare("3") == 0 || s.compare("DBG") == 0) m = log::mode::DBG;
					else e = 1;
				} 
				if(e){
					m = log::mode::ERR;
					out(m, "ERROR DISCERNING LOG LEVEL, ERROR LEVEL IN USE");
				}
			}
		}
	public:
		static LogMan& INSTANCE(){
			static LogMan instance;
			return instance;
		};
		bool inf(){ return m >= log::INF;}
		bool err(){ return m >= log::ERR;}
		bool dbg(){ return m >= log::DBG;}
		void log(const char* f, const int& l, const log::mode& m, const std::string& s){
			if(this->m >= m) logger.log(f, l, s, m);
		}
		void out(const log::mode& m, const std::string& s){
			if(this->m >= m) logger.out(s + kul::os::EOL(), m);
		}
		void err(const log::mode& m, const std::string& s){
			logger.out(s + kul::os::EOL(), m);
		}
};

class Message{
	protected:
		std::stringstream ss;
		const log::mode& m;

		Message(const log::mode& m) : m(m){}
	public:
		template<class T> Message& operator<<(const T& s){
			ss << s;
			return *this;
		}
};
class LogMessage : public Message{
	private:
		const char* f;
		const int& l;
	public:		
		~LogMessage(){
			LogMan::INSTANCE().log(f, l, m, ss.str());
		}
		LogMessage(const char* f, const int& l, const log::mode& m) : Message(m), f(f), l(l){}
};
class OutMessage : public Message{
	public:
		~OutMessage(){
			LogMan::INSTANCE().out(m, ss.str());
		}
		OutMessage(const log::mode& m = kul::log::mode::NON) : Message(m){}
};
class ErrMessage : public Message{
	public:
		~ErrMessage(){
			LogMan::INSTANCE().err(m, ss.str());
		}
		ErrMessage() : Message(log::mode::ERR){}
};

#define KLOG_INF 	kul::LogMessage(__FILE__, __LINE__, kul::log::mode::INF)
#define KLOG_ERR 	kul::LogMessage(__FILE__, __LINE__, kul::log::mode::ERR)
#define KLOG_DBG 	kul::LogMessage(__FILE__, __LINE__, kul::log::mode::DBG)
#define KLOG(sev) KLOG_ ## sev

#define KOUT_NON 	kul::OutMessage()
#define KOUT_INF 	kul::OutMessage(kul::log::mode::INF)
#define KOUT_ERR 	kul::OutMessage(kul::log::mode::ERR)
#define KOUT_DBG 	kul::OutMessage(kul::log::mode::DBG)
#define KOUT(sev) KOUT_ ## sev

#define KERR 		kul::ErrMessage()

} // END NAMESPACE kul
#endif /* _KUL_LOG_HPP_ */
