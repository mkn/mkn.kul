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
#ifndef _KUL_DEFS_HPP_
#define _KUL_DEFS_HPP_

#ifndef uint
	typedef unsigned int uint;
#endif /*  uint */

#ifndef ulong
	typedef unsigned long ulong;
#endif /*  ulong */
#ifndef ulonglong
	typedef unsigned long long ulonglong;
#endif /*  ulonglong */


#define KSTRINGIFY(x) #x
#define KTOSTRING(x) KSTRINGIFY(x)

#ifdef KUL_SHARED
	#if defined _WIN32 || defined __CYGWIN__
	  #ifdef KUL_EXPORT
	    #ifdef __GNUC__
	      #define KUL_PUBLISH __attribute__ ((dllexport))
	    #else
	      #define KUL_PUBLISH __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
	    #endif
	  #else
	    #ifdef __GNUC__
	      #define KUL_PUBLISH __attribute__ ((dllimport))
	    #else
	      #define KUL_PUBLISH __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
	    #endif
	  #endif
	#else
	  #if __GNUC__ >= 4
	    #define KUL_PUBLISH __attribute__ ((visibility ("default")))
	    #define KUL_PRIVATE __attribute__ ((visibility ("hidden")))
	  #endif
	#endif
#endif

#ifndef KUL_PUBLISH
#define KUL_PUBLISH
#endif

#ifndef KUL_PRIVATE
#define KUL_PRIVATE
#endif

#endif /* _KUL_DEFS_HPP_ */
