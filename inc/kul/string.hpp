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
#ifndef _KUL_STRING_HPP_
#define _KUL_STRING_HPP_

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

namespace kul { 

class String{
	public:
		static void replace(std::string& s, const std::string& f, const std::string& r){
			size_t p = 0;
			if((p = s.find(f)) != std::string::npos)
				s.replace(p, f.size(), r);
		}
		static void replaceAll(std::string& s, const std::string& f, const std::string& r){
			while(s.find(f) != std::string::npos) replace(s, f, r);
		}
		static void leftTrim(std::string& s, const char& delim=' '){
			while(s.find(delim) == 0)
				s.erase(0, 1);
		}
		static void rightTrim(std::string& s, const char& delim=' '){
			while(s.rfind(delim) == s.size() - 1)
				s.pop_back();
		}
		static void trim(std::string& s){
			while(s.find(' ') == 0 || s.find('	') == 0)
				s.erase(0, 1);
			while(s.rfind(' ') == s.size() - 1 || s.rfind('\t') == s.size() - 1)
				s.pop_back();
		}
		static void pad(std::string& s, const uint16_t& p){
			while(s.size() < p) s += " ";
		}
		static std::vector<std::string> split(const std::string& s, const char& d){
			std::vector<std::string> v;
			split(s, d, v);
			return v;
		}
		static void split(const std::string& s, const char& d, std::vector<std::string>& v){
			if(s.find(d) != std::string::npos){
				std::string l;
				std::stringstream stream(s);
				while(std::getline(stream, l, d))
					if(l.compare("") != 0) v.push_back(l);
			}else v.push_back(s);
		}
		static std::vector<std::string> split(const std::string& s, const std::string& d){
			std::vector<std::string> v;
			split(s, d, v);
			return v;
		}
		static void split(const std::string& s, const std::string& d, std::vector<std::string>& v){
			std::string l = s;
			size_t pos = 0;
			while((pos = s.find(d, pos + 1)) < l.size()){
				v.push_back(l.substr(0, pos));
				l = l.substr(pos + 1);
			}
			v.push_back(l);
		}
		static bool cicmp(const std::string& a, const std::string& b){
			    std::string aCpy(a);
			    std::string bCpy(b);
			    std::transform(aCpy.begin(), aCpy.end(), aCpy.begin(), ::tolower);
			    std::transform(bCpy.begin(), bCpy.end(), bCpy.begin(), ::tolower);
			    return (aCpy == bCpy);
		}
		static std::vector<std::string> lines(const std::string& s){
			std::vector<std::string> v;
			if(s.find("\n") != std::string::npos){
				std::string l;
				std::stringstream ss(s);
				while(std::getline(ss, l)) if(!l.empty()) v.push_back(l);
			}else v.push_back(s);
			return v;
		}
};

}
#endif /* _KUL_STRING_HPP_ */
