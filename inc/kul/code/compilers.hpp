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
#ifndef _KUL_CODE_COMPILERS_HPP_
#define _KUL_CODE_COMPILERS_HPP_

#include "kul/code/cpp.hpp"
#include "kul/code/csharp.hpp"

namespace kul{ namespace code{ 

class CompilerNotFoundException : public kul::Exception{
	public:
		CompilerNotFoundException(const char*f, const int l, std::string s) : kul::Exception(f, l, s){}
};

class Compilers{
	private:
		Compilers(){
			gcc		= std::make_unique<cpp::GCCompiler>();
			clang	= std::make_unique<cpp::ClangCompiler>();
			intel	= std::make_unique<cpp::IntelCompiler>();
			winc	= std::make_unique<cpp::WINCompiler>();

			wincs 	= std::make_unique<csharp::WINCompiler>();

			cs.insert(std::pair<std::string, Compiler*>("gcc"		, gcc.get()));
			cs.insert(std::pair<std::string, Compiler*>("g++"		, gcc.get()));
			cs.insert(std::pair<std::string, Compiler*>("nvcc"		, gcc.get()));
			cs.insert(std::pair<std::string, Compiler*>("clang"		, clang.get()));
			cs.insert(std::pair<std::string, Compiler*>("clang++"	, clang.get()));
			cs.insert(std::pair<std::string, Compiler*>("icc"		, intel.get()));
			cs.insert(std::pair<std::string, Compiler*>("icpc"		, intel.get()));
			cs.insert(std::pair<std::string, Compiler*>("cl"		, winc.get()));
			cs.insert(std::pair<std::string, Compiler*>("csc"		, wincs.get()));
		}
		std::unique_ptr<Compiler> gcc;
		std::unique_ptr<Compiler> clang;
		std::unique_ptr<Compiler> intel;
		std::unique_ptr<Compiler> winc;
		std::unique_ptr<Compiler> wincs;
		hash::map::S2T<Compiler*> cs;
	public:
		static Compilers& INSTANCE(){ 
			static Compilers instance;
			return instance;
		}
		const std::string key(std::string comp){
			kul::String::REPLACE_ALL(comp, ".exe", "");
			if(cs.count(comp) > 0)return comp;
			if(comp.find(" ") != std::string::npos)
				for(const std::string& s :kul::String::SPLIT(comp, ' ')){
					if(cs.count(s) > 0) return s;
					if(std::string(kul::Dir(s).locl()).find(kul::Dir::SEP()) != std::string::npos)
						if(cs.count(s.substr(s.rfind(kul::Dir::SEP()) + 1)) > 0)
							return s.substr(s.rfind(kul::Dir::SEP()) + 1);
				}
			if(std::string(kul::Dir(comp).locl()).find(kul::Dir::SEP()) != std::string::npos)
				if(cs.count(comp.substr(comp.rfind(kul::Dir::SEP()) + 1)) > 0)
					return comp.substr(comp.rfind(kul::Dir::SEP()) + 1);
			KEXCEPT(CompilerNotFoundException, "Compiler for " + comp + " is not implemented");
		}
		const Compiler* get(std::string comp) throw(CompilerNotFoundException){
			return (*cs.find(key(comp))).second;
		}
};


}}
#endif /* _KUL_CODE_COMPILERS_HPP_ */