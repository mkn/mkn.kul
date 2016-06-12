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
#ifndef _KUL_IO_HPP_
#define _KUL_IO_HPP_

#include <memory>
#include <time.h>
#include <fstream>
#include <stdexcept>

#include "kul/os.hpp"
#include "kul/except.hpp"
#include "kul/string.hpp"

namespace kul{  namespace io {

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

class AReader{
    private:
        std::string s1;
        // std::unique_ptr<std::string> str;
    protected:
        const char* readLine(std::ifstream& f){
            s1.clear();
            if(f.good()){
                std::getline(f, s1);
                return s1.c_str();
            }
            return 0;
        }
        const char* read(std::ifstream& f, const uint16_t& l){
            if(f.good()){
                std::vector<char> v;
                v.resize(l);
                f.read(&v[0], l);
                v.resize((uint16_t)f.gcount());
                // str = std::make_unique<std::string>(std::string(v.begin(), v.end()));
                return &v[0];
            }
            return 0;
        }
    public:
        virtual ~AReader(){}
        virtual const char* readLine() = 0;
        virtual const char* read(const uint16_t& l) = 0;
        virtual void seek(const uint16_t& l) = 0;
        virtual void seek(std::ifstream& f, const uint16_t& l){
            f.seekg(l);
        }
};
class Reader : public AReader{
    private:
        std::ifstream f;
    public:
        Reader(const char* c) : f(c, std::ios::in) { 
            if(!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
        }
        Reader(const File& c) : Reader(c.full().c_str()){}
        ~Reader() { f.close();}
        const char* readLine(){
            return AReader::readLine(f);
        }
        const char* read(const uint16_t& l){
            return AReader::read(f, l);
        }
        void seek(const uint16_t& l){
            AReader::seek(f, l);
        }
};
class BinaryReader : public AReader{
    private:
        std::ifstream f;
    public:
        BinaryReader(const char* c) : f(c, std::ios::in |std::ios::binary){
            if(!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
        }
        BinaryReader(const File& c) : BinaryReader(c.full().c_str()){}
        ~BinaryReader() { f.close();}
        const char* readLine(){
            return AReader::readLine(f);
        }
        const char* read(const uint16_t& s){
            return AReader::read(f, s);
        }
        void seek(const uint16_t& s){
            AReader::seek(f, s);
        }
};


class AWriter{
    protected:
        void write(std::ofstream& f, const char*c, bool nl){ 
            if(nl)  f << c << kul::os::EOL();
            else    f << c;
        }
    public:
        virtual AWriter& write(const char*c, bool nl = false) = 0;
};

class Writer: public AWriter{
    private:
        std::ofstream f;
    public:
        Writer(const char*c, bool a = 0){ 
            if(a) f.open(c, std::ios::out | std::ios::app);
            else  f.open(c, std::ios::out);
            if(!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
        }
        Writer(const File& c, bool a = 0) : Writer(c.full().c_str(), a){}
        ~Writer() { f.close();}
        AWriter& write(const char*c, bool nl = false){
            AWriter::write(f, c, nl);
            return *this;
        }
        template<class T> Writer& operator<<(const T& s){
            f << s;
            return *this;
        }
        AWriter& operator<< (std::ostream& (*os)(std::ostream&)) {
            f << std::flush;
            return *this;
        }
        AWriter& flush(){
            f << std::flush;
            return *this;
        }
};
class BinaryWriter : public AWriter{
    private:
        std::ofstream f;
    public:
        BinaryWriter(const char* c) : f(c, std::ios::out |std::ios::binary){ 
            if(!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
            f.unsetf(std::ios_base::skipws);
        }
        BinaryWriter(const File& c) : BinaryWriter(c.full().c_str()){}
        ~BinaryWriter() { f.close();}
        AWriter& write(const char*c, bool nl = false){
            AWriter::write(f, c, nl);
            return *this;
        }
        template<class T> BinaryWriter& operator<<(const T& s){
            f << s;
            return *this;
        }
        BinaryWriter& operator<< (std::ostream& (*os)(std::ostream&)) {
            f << std::flush;
            return *this;
        }
        BinaryWriter& flush(){
            f << std::flush;
            return *this;
        }
};

}}
#endif /* _KUL_IO_HPP_ */

