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
#ifndef _KUL_CPU_HPP_
#define _KUL_CPU_HPP_

#include <pwd.h>
#include <thread>
#include <fstream>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __APPLE__
    #include <sys/param.h>
    #include <sys/sysctl.h>
#endif

namespace kul{
namespace cpu{
inline unsigned int cores() {
	unsigned int numCPU = sysconf( _SC_NPROCESSORS_ONLN );
	int mib[4];
	size_t len = sizeof(numCPU);

	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;   // alternatively, try HW_NCPU;

	sysctl(mib, 2, &numCPU, &len, NULL, 0);

	if(numCPU < 1){
		mib[1] = HW_NCPU;
		sysctl( mib, 2, &numCPU, &len, NULL, 0 );
		if(numCPU < 1) numCPU = 1;
	}
	return numCPU;
}
inline unsigned int threads() {
	return std::thread::hardware_concurrency();
}
} // END NAMESPACE cpu
} // END NAMESPACE kul

#endif /* _KUL_CPU_HPP_ */


