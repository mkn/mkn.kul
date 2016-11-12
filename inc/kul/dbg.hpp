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
#ifndef _KUL_DBG_HPP_
#define _KUL_DBG_HPP_

#include <kul/log.hpp>

namespace kul{ namespace dbg{

#ifndef NDEBUG

#ifndef KUL_DBG_FUNC_ENTER
#define KUL_DBG_FUNC_ENTER kul::dbg::FunctionScope(__FILE__, __func__);
#endif//KUL_DBG_FUNC_ENTER

#ifndef KUL_DBG_FUNC_ON_ENTER
#define KUL_DBG_FUNC_ON_ENTER 
#endif//KUL_DBG_FUNC_ON_ENTER

#ifndef KUL_DBG_FUNC_ON_EXIT
#define KUL_DBG_FUNC_ON_EXIT KOUT(DBG) \
	    << kul::LogMan::INSTANCE().str(m_fi, m_fu, 0, kul::log::mode::DBG, "", "[%M]: %T - %D : %F fn(%N)") \
		<< " - Time in function: " << (kul::Now::MICROS() - m_start);
#endif//KUL_DBG_FUNC_ON_EXIT

#else//

#define KUL_DBG_FUNC_ENTER 
#define KUL_DBG_FUNC_ON_ENTER
#define KUL_DBG_FUNC_ON_EXIT 


#endif//NDEBUG

class FunctionScope {
	private:
		uint16_t m_line = 0;
		uint64_t m_start = 0;
		const char* m_fi;
		const char* m_fu;
	public:
		FunctionScope(const char* fi, const char* fu) : m_fi(fi), m_fu(fu), m_start(kul::Now::MICROS()){
			KUL_DBG_FUNC_ON_ENTER
		}
		~FunctionScope(){
			KUL_DBG_FUNC_ON_EXIT
		}
};
}}


#endif /* _KUL_DBG_HPP_ */


