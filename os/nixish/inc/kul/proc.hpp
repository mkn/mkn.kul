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
#ifndef _KUL_PROC_HPP_
#define _KUL_PROC_HPP_

#include <queue>
#include <memory>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdexcept>
#include <sys/wait.h>

#include "kul/os.hpp"
#include "kul/log.hpp"
#include "kul/proc.base.hpp"

namespace kul {

namespace this_proc{
inline int id(){
	return getpid();
}
inline void kill(const int& e){
	::kill(kul::this_proc::id(), e);
}
}

class Process : public kul::AProcess{
	private:
		int inFd[2];
		int outFd[2];
		int errFd[2];
		int popPip[3];
		int cStat; //child status
		int cec; //child exit code

		inline int recall(const int& s){
			int ret; 
			while((ret = (s)) < 0x0 && (errno == EINTR)){}
			return ret;
		}
	protected:
		int	child(){
			std::string s(args()[0]);
			char** as = new char*[args().size() + 1]; // doesnt' need deleting - process exists after method
			int i = 0;
			for(const std::string& c : args()){ as[i] = const_cast<char*>(c.c_str()); i++; }
			as[i] = NULL;
			return execvp(s.c_str(), as);
		}
		void finish()	{ }
		void preStart()	{ }
	public:
		Process(const std::string& cmd, const bool& wfe = true)							: kul::AProcess(cmd, wfe){}
		Process(const std::string& cmd, const std::string& path, const bool& wfe = true): kul::AProcess(cmd, path, wfe){}
		bool kill(int k = 6){
			if(started()){
				bool b = ::kill(pid(), k) == 0;
				if(::kill(pid(), 0) == 0) setFinished();
				return b;
			}
			return 0;
		}
	protected:
		void waitForStatus(){
			int ret = 0;
			ret = recall(waitpid(pid(), &cStat, 0));
			assert(ret);
		}
		void waitExit() throw (kul::proc::ExitException){
			tearDown();
			cec = WEXITSTATUS(cStat);
			if(cec != 0)
				kul::LogMan::INSTANCE().err()
					? throw proc::ExitException(__FILE__, __LINE__, cec, "Process exit code: " + std::to_string(cec) + kul::os::EOL() + toString())
					: throw proc::ExitException(__FILE__, __LINE__, cec, "Process exit code: " + std::to_string(cec));
			finish();
			setFinished();
		}
		void tearDown(){
			recall(close(popPip[0]));
			recall(close(popPip[1]));
			recall(close(popPip[2]));
			recall(close(errFd[1]));
			recall(close(errFd[0]));
			recall(close(outFd[1]));
			recall(close(outFd[0]));
			recall(close(inFd[1]));
			recall(close(inFd[0]));
		}
		void run() throw (kul::proc::Exception){
			int ret = 0;

			if((ret = pipe(inFd)) < 0) 	error(__LINE__, "Failed to pipe in");
			if((ret = pipe(outFd)) < 0)	error(__LINE__, "Failed to pipe out");
			if((ret = pipe(errFd)) < 0)	error(__LINE__, "Failed to pipe err");

			this->preStart();
			pid(fork());
			if(pid() > 0){
				if(this->waitForExit()){ // parent
					popPip[0] = inFd[1];
					popPip[1] = outFd[0];
					popPip[2] = errFd[0];

		#ifdef __KUL_PROC_BLOCK_ERR__
					if((ret = fcntl(popPip[1], F_SETFL, O_NONBLOCK)) < 0) error(__LINE__, "Failed nonblocking for popPip[1]");
					if((ret = fcntl(popPip[2], F_SETFL, O_NONBLOCK)) < 0) error(__LINE__, "Failed nonblocking for popPip[2]");
		#else
					fcntl(popPip[1], F_SETFL, O_NONBLOCK);
					fcntl(popPip[2], F_SETFL, O_NONBLOCK);
		#endif
					fd_set childOutFds;
					FD_ZERO(&childOutFds);
					FD_SET(popPip[1], &childOutFds);
					FD_SET(popPip[2], &childOutFds);
					close(inFd[1]);
					bool alive = true;

					do {
						alive = ::kill(pid(), 0) == 0;
						if(FD_ISSET(popPip[1], &childOutFds)) {
							bool b = 0;
							do {
								char cOut[1024] = {'\0'};
								int ret = recall(read(popPip[1], cOut, sizeof(cOut)));
								cOut[ret > 0 ? ret : 0] = 0;
								if (ret < 0){
									if(b && ((errno != EAGAIN) || (errno != EWOULDBLOCK)))
										error(__LINE__, "read on childout failed");
									if(((errno != EAGAIN) || (errno != EWOULDBLOCK))) b = 1;
								}
								else if (ret) out(cOut);
								else waitForStatus();
							} while(ret > 0);
						}
						if(FD_ISSET(popPip[2], &childOutFds)) {
							bool b = 0;
							do {
								char cErr[1024] = {'\0'};
								int ret = recall(read(popPip[2], cErr, sizeof(cErr)));
								cErr[ret > 0 ? ret : 0] = 0;
								if (ret < 0){
									if(b && ((errno != EAGAIN) || (errno != EWOULDBLOCK)))
										error(__LINE__, "read on childout failed");
									if(((errno != EAGAIN) || (errno != EWOULDBLOCK))) b = 1;
								}
								else if (ret) err(cErr);
								else waitForStatus();
							} while(ret > 0);
						}
						recall(waitpid(pid(), &cStat, WNOHANG));
					}while(alive);

					waitExit();
				}
			}else if(pid() == 0){ // child
				close(inFd[1]);
				close(outFd[0]);
				close(errFd[0]);

				int ret = 0; //check rets
				close(0);
				if((ret = dup(inFd[0])) < 0) 	error(__LINE__, "dup in call failed");
				close(1);
				if((ret = dup(outFd[1])) < 0) 	error(__LINE__, "dup out call failed");
				close(2);
				if((ret = dup(errFd[1])) < 0) 	error(__LINE__, "dup err call failed");

				/* SETUP EnvVars */ // SET ENV, it's a forked process so it doesn't matter - it'll die soon, like you.
				for(const std::pair<const std::string, const std::string>& ev : vars())
					env::SET(ev.first.c_str(), ev.second.c_str());

				if(!this->directory().empty()) kul::env::CWD(this->directory());
				exit(this->child());
			}else error(__LINE__, "Unhandled process id for child: " + std::to_string(pid()));
		}
};


}
#endif /* _KUL_PROC_HPP_ */
