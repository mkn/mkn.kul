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
#ifndef _KUL_TIME_HPP_
#define _KUL_TIME_HPP_

#include <ctime>
#include <chrono>
#include <sstream>

#include "kul/string.hpp"

namespace kul {
namespace time{
class Exception : public kul::Exception{
	public:
		Exception(const char*f, const int l, const std::string& s) : kul::Exception(f, l, s){}
};
}

class Now{
	public:
		static uint64_t MILLIS(){ return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
		static uint64_t MICROS(){ return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
		static uint64_t NANOS()	{ return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
};

class DateTime{
	private:
		static const std::string MILLIS(){
			std::string s(std::to_string(Now::MILLIS()));
			return s.substr(s.length() - 3);
		}
	public:
		static const std::string AS(const std::time_t t, std::string f = "%Y-%m-%d-%H:%M:%S"){
			kul::String::replace(f, "%i", MILLIS());
			char buffer [80];
			struct tm ti;
#ifdef _WIN32
			localtime_s(&ti, &t);
#else
			localtime_r(&ti, &t);
#endif
			std::strftime(buffer, 80, f.c_str(), &ti);
			return std::string(buffer);
		}
		static const std::string AS(const std::string& epoch, const std::string& f = "%Y-%m-%d-%H:%M:%S"){
			ulong e = 0;
			std::stringstream ss(epoch);
			ss >> e;
			if(!e) KEXCEPT(time::Exception, "Invalid time used :" + epoch);
			return AS(e, f);
		}
		static const std::string NOW(const std::string& f = "%Y-%m-%d-%H:%M:%S"){
			return AS(std::time(NULL), f);
		}
};
}
#endif /* _KUL_TIME_HPP_ */
