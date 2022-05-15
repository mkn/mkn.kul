/**
Copyright (c) 2017, Philip Deegan.
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
// IWYU pragma: private, include "mkn/kul/proc.hpp"

void mkn::kul::Process::run() KTHROW(mkn::kul::proc::Exception) {
  {
    int16_t ret = 0;
    if ((ret = pipe(inFd)) < 0) error(__LINE__, "Failed to pipe in");
    if ((ret = pipe(outFd)) < 0) error(__LINE__, "Failed to pipe out");
    if ((ret = pipe(errFd)) < 0) error(__LINE__, "Failed to pipe err");
  }

  this->preStart();
  pid(fork());
  if (pid() > 0) {
    int16_t ret = 0;
    if (this->waitForExit()) {  // parent
      popPip[0] = inFd[1];
      popPip[1] = outFd[0];
      popPip[2] = errFd[0];

#ifdef __MKN_KUL_PROC_BLOCK_ERR__
      if ((ret = fcntl(popPip[1], F_SETFL, O_NONBLOCK)) < 0)
        error(__LINE__, "Failed nonblocking for popPip[1]");
      if ((ret = fcntl(popPip[2], F_SETFL, O_NONBLOCK)) < 0)
        error(__LINE__, "Failed nonblocking for popPip[2]");
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

      char cOut[30024] = {'\0'};
      char cErr[30024] = {'\0'};
      do {
#if defined(_MKN_KUL_PROC_LOOP_NSLEEP_) && (_MKN_KUL_PROC_LOOP_NSLEEP_ > 0)
        mkn::kul::this_thread::nSleep(_MKN_KUL_PROC_LOOP_NSLEEP_);
#endif
        alive = ::kill(pid(), 0) == 0;
        if (FD_ISSET(popPip[1], &childOutFds)) {
          bool b = 0;
          do {
            memset(cOut, 0, sizeof(cOut));
            ret = recall(read(popPip[1], cOut, sizeof(cOut)));
            cOut[ret > 0 ? ret : 0] = 0;
            if (ret < 0) {
              if (b && ((errno != EAGAIN) || (errno != EWOULDBLOCK)))
                error(__LINE__, "read on childout failed");
              if (((errno != EAGAIN) || (errno != EWOULDBLOCK))) b = 1;
            } else if (ret)
              out(cOut);
            else
              waitForStatus();
          } while (ret > 0);
        }
        if (FD_ISSET(popPip[2], &childOutFds)) {
          bool b = 0;
          do {
            memset(cErr, 0, sizeof(cErr));
            ret = recall(read(popPip[2], cErr, sizeof(cErr)));
            cErr[ret > 0 ? ret : 0] = 0;
            if (ret < 0) {
              if (b && ((errno != EAGAIN) || (errno != EWOULDBLOCK)))
                error(__LINE__, "read on childout failed");
              if (((errno != EAGAIN) || (errno != EWOULDBLOCK))) b = 1;
            } else if (ret)
              err(cErr);
            else
              waitForStatus();
          } while (ret > 0);
        }
        recall(waitpid(pid(), &cStat, WNOHANG));
      } while (alive);

      waitExit();
    }
  } else if (pid() == 0) {  // child
    close(inFd[1]);
    close(outFd[0]);
    close(errFd[0]);

    int16_t ret = 0;  // check rets
    int8_t retry = __MKN_KUL_PROC_DUP_RETRY__;
    if (retry < 1) retry = 1;

    close(0);
    for (uint8_t i = 0; i < retry; i++)
      if ((ret = dup(inFd[0])) >= 0) break;
    if (ret < 0) error(__LINE__, "dup in call failed");

    close(1);
    for (uint8_t i = 0; i < retry; i++)
      if ((ret = dup(outFd[1])) >= 0) break;
    if (ret < 0) error(__LINE__, "dup out call failed");

    close(2);
    for (uint8_t i = 0; i < retry; i++)
      if ((ret = dup(errFd[1])) >= 0) break;
    if (ret < 0) error(__LINE__, "dup err call failed");

    /* SETUP EnvVars */  // SET ENV, it's a forked process so it doesn't matter -
                         // it'll die soon, like you.
    for (auto const &ev : vars()) env::SET(ev.first.c_str(), ev.second.c_str());

    if (!this->directory().empty()) mkn::kul::env::CWD(this->directory());
    exit(this->child());
  } else
    error(__LINE__, "Unhandled process id for child: " + std::to_string(pid()));
}
