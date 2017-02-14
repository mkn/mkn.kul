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
#include <string.h>
#include <algorithm>
#include <iostream>

#include "kul/except.hpp"

namespace kul { 

class StringException : public kul::Exception{
    public:
        StringException(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

class String{
    public:
        static void REPLACE(std::string& s, const std::string& f, const std::string& r){
            size_t p = 0;
            if((p = s.find(f)) != std::string::npos) s.replace(p, f.size(), r);
        }
        static void REPLACE_ALL(std::string& s, const std::string& f, const std::string& r){
            while(s.find(f) != std::string::npos) REPLACE(s, f, r);
        }
        static void TRIM_LEFT(std::string& s, const char& delim=' '){
            while(s.find(delim) == 0)
                s.erase(0, 1);
        }
        static void TRIM_RIGHT(std::string& s, const char& delim=' '){
            while(s.rfind(delim) == s.size() - 1)
                s.pop_back();
        }
        static void TRIM(std::string& s){
            while(s.find(' ') == 0 || s.find('\t') == 0)
                s.erase(0, 1);
            if(s.size() == 0) return;
            while(s.rfind(' ') == s.size() - 1 || s.rfind('\t') == s.size() - 1)
                s.pop_back();
        }
        static void PAD(std::string& s, const uint16_t& p){
            while(s.size() < p) s += " ";
        }
        static std::vector<std::string> SPLIT(const std::string& s, const char& d){
            std::vector<std::string> v;
            SPLIT(s, d, v);
            return v;
        }
        static void SPLIT(const std::string& s, const char& d, std::vector<std::string>& v){
            if(s.find(d) != std::string::npos){
                std::string l;
                std::stringstream stream(s);
                while(std::getline(stream, l, d))
                    if(l.compare("") != 0) v.push_back(l);
            }else v.push_back(s);
        }
        static std::vector<std::string> SPLIT(const std::string& s, const std::string& d){
            std::vector<std::string> v;
            SPLIT(s, d, v);
            return v;
        }
        static void SPLIT(const std::string& s, const std::string& d, std::vector<std::string>& v){
            std::string l = s;
            size_t pos = 0;
            while((pos = l.find(d)) != std::string::npos){
                v.push_back(l.substr(0, pos));
                l = l.substr(pos + 1);
            }
            v.push_back(l);
        }
        static std::vector<std::string> ESC_SPLIT(const std::string& s, const char& d, const char& e = '\\'){
            std::vector<std::string> v;
            ESC_SPLIT(s, d, v, e);
            return v;
        }
        static void ESC_SPLIT(const std::string& s, const char& d, std::vector<std::string>& v, const char& e = '\\'){
            std::string l = s, ds = std::string(1, d), es = std::string(1, e);
            size_t pos = 0, esc = 0;
            while((pos = l.find(d, esc)) != std::string::npos){
                if(pos > 0 && l.find(es + ds) == pos - 1){ 
                    esc++;
                    continue; 
                }
                std::string t = l.substr(0, pos);
                REPLACE_ALL(t, es, "");
                v.push_back(t);
                l = l.substr(pos + 1);
                esc = 0;
            }
            REPLACE_ALL(l, es, "");
            v.push_back(l);
        }
        static bool NO_CASE_CMP(std::string a, std::string b){
            std::transform(a.begin(), a.end(), a.begin(), ::tolower);
            std::transform(b.begin(), b.end(), b.begin(), ::tolower);
            return (a == b);
        }
        static std::vector<std::string> LINES(const std::string& s){
            std::vector<std::string> v;
            LINES(s, v);
            return v;
        }
        static void LINES(const std::string& s, std::vector<std::string>& v){
            if(s.find("\n") != std::string::npos){
                std::string l;
                std::stringstream ss(s);
                while(std::getline(ss, l)) if(!l.empty()) v.push_back(l);
            }else v.push_back(s);
        }

        static bool BOOL(std::string s){
            TRIM(s);
            const std::vector<std::string>& pos {"yes", "y", "true" , "1"}; 
            const std::vector<std::string>& neg {"no" , "n", "false", "0"}; 
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            if (std::find(pos.begin(), pos.end(), s) != pos.end()) return true;
            if (std::find(neg.begin(), neg.end(), s) != neg.end()) return false;
            KEXCEPT(StringException, "input not bool-able, " + s);
        }

        static uint16_t UINT16(const std::string& s) throw(StringException){
            try{
                uint32_t lresult = stoul(s, 0, 10);
                uint16_t result = lresult;
                if (result != lresult) KEXCEPT(StringException, "UINT failed");
                return result;
            }catch(const std::invalid_argument& e){ KEXCEPT(StringException, "UINT failed"); }
            return 0;
        }
        static int16_t INT16(const std::string& s) throw(StringException){
            try{
                return std::stoi(s); 
            }catch(const std::invalid_argument& e){ KEXCEPT(StringException, "stoi failed"); }
        }
        static uint32_t UINT32(const std::string& s) throw(StringException){
            try{
                return std::stoul(s);
            }catch(const std::invalid_argument& e){ KEXCEPT(StringException, "ULONG failed"); }
            return 0;
        }
        static uint64_t UINT64(const std::string& s) throw(StringException){
            try{
                return std::stoull(s);
            }catch(const std::invalid_argument& e){ KEXCEPT(StringException, "ULONGLONG failed"); }
            return 0;
        }
};

}
#endif /* _KUL_STRING_HPP_ */
