/**
Copyright (c) 2026, Philip Deegan.
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
// IWYU pragma: friend ".*mkn/kul/os/(nix|nixish|bsd)/.*"

#ifndef MKN_KUL_OS_NIXISH_PROC_HPP_
#define MKN_KUL_OS_NIXISH_PROC_HPP_

#include "mkn/kul/os.hpp"
#include "mkn/kul/cli.hpp"
#include "mkn/kul/os/any/proc/any_proc.hpp"  // IWYU pragma: export
#include "mkn/kul/defs.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/os/any/proc/def.hpp"
#include "mkn/kul/os/nixish/env.hpp"

#if MKN_KUL_IS_NIX
#include "mkn/kul/os/nix/proc.os.hpp"  // IWYU pragma: export
#elif MKN_KUL_IS_BSD
#include "mkn/kul/os/bsd/proc.os.hpp"  // IWYU pragma: export
#else
#error unresolved
#endif

#ifndef MKN_KUL_PROC_DUP_RETRY
#define MKN_KUL_PROC_DUP_RETRY 3
#endif  // MKN_KUL_PROC_DUP_RETRY

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <initializer_list>
#include <unordered_map>
#include <utility>

namespace mkn {
namespace kul {

namespace this_proc {
inline int32_t id() { return getpid(); }
inline void kill(int32_t const& e) { ::kill(mkn::kul::this_proc::id(), e); }
}  // namespace this_proc

class Process : public mkn::kul::AProcess {
 public:
  Process(std::string const& cmd, bool const& _wfe = true) : mkn::kul::AProcess(cmd, _wfe) {}
  Process(std::string const& cmd, std::string const& path, bool const& _wfe = true)
      : mkn::kul::AProcess(cmd, path, _wfe) {}
  Process(std::string const& cmd, mkn::kul::Dir const& _d, bool const& _wfe = true)
      : mkn::kul::AProcess(cmd, (_d ? _d.real() : _d.path()), _wfe) {}
  bool kill(int16_t k = 6) {
    if (started()) {
      bool b = ::kill(pid(), k) == 0;
      if (::kill(pid(), 0) == 0) setFinished();
      return b;
    }
    return 0;
  }

 protected:
  int inline child();
  virtual void inline expand(std::string&) const;
  void inline waitForStatus();
  void inline waitExit() KTHROW(mkn::kul::proc::ExitException);
  void inline tearDown();
  void inline run() KTHROW(mkn::kul::proc::Exception);

  virtual void finish() {}
  virtual void preStart() {}

 private:
  // popPip aliases the parent's ends of the pipes below and is not closed itself
  int inFd[2]{-1, -1}, outFd[2]{-1, -1}, errFd[2]{-1, -1}, popPip[3]{-1, -1, -1};
  int cStat;  // child status

  // close once - a repeated close() can hit an fd number another thread has since been given
  static void closeFd(int& fd) {
    if (fd < 0) return;
    close(fd);
    fd = -1;
  }

  // a forked child must never return or throw into the parent's code (e.g. a thread pool loop),
  // nor run atexit handlers/static destructors it inherited
  [[noreturn]] static void childFail(char const* msg) {
    ssize_t const w = write(STDERR_FILENO, msg, strlen(msg));
    (void)w;
    _exit(127);
  }

  // close-on-exec so concurrent children don't inherit each other's pipes, the child's
  // dup()'d stdin/out/err don't carry the flag and survive exec
  static int pipeCloExec(int fds[2]) {
#if defined(__APPLE__)
    int const ret = pipe(fds);
    if (ret == 0)
      for (int i = 0; i < 2; ++i) fcntl(fds[i], F_SETFD, fcntl(fds[i], F_GETFD) | FD_CLOEXEC);
    return ret;
#else
    return pipe2(fds, O_CLOEXEC);
#endif
  }

  inline int recall(int const& _s) {
    int ret;
    while ((ret = (_s)) < 0x0 && (errno == EINTR)) {
    }
    return ret;
  }
};

int Process::child() {
  std::string _s(toString());
  expand(_s);
  std::vector<std::string> cli{mkn::kul::cli::asArgs(_s)};
  std::vector<char*> argV;
  for (auto& a : cli) argV.push_back(&a[0]);
  argV.push_back(NULL);
  return execvp(cli[0].c_str(), &argV[0]);
}

void Process::expand(std::string& _s) const {
  std::string r = _s;
  auto lb = _s.find("$(");
  auto clb = _s.find("\\$(");
  while ((lb - clb + 1) == 0) {
    lb = r.find("$(", clb + 3);
    clb = r.find("\\$(", clb + 3);
  }
  if (lb == std::string::npos) return;
  auto rb = _s.find(")");
  auto crb = _s.find("\\)");
  while ((rb - crb + 1) == 0) {
    rb = r.find(")", crb + 2);
    crb = r.find("\\)", crb + 2);
  }
  if (rb == std::string::npos) return;

  std::string k(r.substr(lb + 2, rb - 2 - lb));
  std::vector<std::string> cli{mkn::kul::cli::asArgs(k)};
  std::stringstream ss;
  if (cli.size() > 1) {
    mkn::kul::Process p(cli[0]);
    mkn::kul::ProcessCapture pc(p);
    for (size_t i = 1; i < cli.size(); i++) p.arg(cli[i]);
    p.start();
    std::string out(pc.outs());
    if (*out.rbegin() == '\n') out.pop_back();
    ss << r.substr(0, lb) << out << r.substr(rb + 1);
  } else
    ss << r.substr(0, lb) << mkn::kul::env::GET(cli[0].c_str()) << r.substr(rb + 1);

  std::string t(ss.str());
  expand(t);
  _s = t;
}

void Process::run() KTHROW(mkn::kul::proc::Exception) {
  if (pipeCloExec(inFd) < 0) error(__LINE__, "Failed to pipe in");
  if (pipeCloExec(outFd) < 0) error(__LINE__, "Failed to pipe out");
  if (pipeCloExec(errFd) < 0) error(__LINE__, "Failed to pipe err");

  this->preStart();
  pid(fork());
  if (pid() > 0) {
    int32_t ret = 0;
    if (this->waitForExit()) {  // parent
      popPip[0] = inFd[1];
      popPip[1] = outFd[0];
      popPip[2] = errFd[0];

#ifdef MKN_KUL_PROC_BLOCK_ERR
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
      closeFd(inFd[1]);
      popPip[0] = -1;
      bool alive = true;

      char cOut[proc::BUFFER_SIZE] = {'\0'};
      char cErr[proc::BUFFER_SIZE] = {'\0'};
      do {
#if MKN_KUL_PROC_LOOP_NSLEEP
        mkn::kul::this_thread::nSleep(MKN_KUL_PROC_LOOP_NSLEEP);
#endif
        alive = ::kill(pid(), 0) == 0;
        if (FD_ISSET(popPip[1], &childOutFds)) {
          bool b = 0;
          do {
#if MKN_KUL_PROC_LOOP_NSLEEP
            mkn::kul::this_thread::nSleep(MKN_KUL_PROC_LOOP_NSLEEP);
#endif

            memset(cOut, 0, sizeof(cOut));
            ret = recall(read(popPip[1], cOut, sizeof(cOut) - 1));
            if (ret >= proc::BUFFER_SIZE)
              error(__LINE__, "ret >= proc::BUFFER_SIZE: " + std::to_string(ret));
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
#if MKN_KUL_PROC_LOOP_NSLEEP
            mkn::kul::this_thread::nSleep(MKN_KUL_PROC_LOOP_NSLEEP);
#endif

            memset(cErr, 0, sizeof(cErr));
            ret = recall(read(popPip[2], cErr, sizeof(cErr) - 1));
            if (ret >= proc::BUFFER_SIZE)
              error(__LINE__, "ret >= proc::BUFFER_SIZE: " + std::to_string(ret));
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

    int32_t ret = 0;  // check rets
    int16_t retry = MKN_KUL_PROC_DUP_RETRY;
    if (retry < 1) retry = 1;

    close(0);
    for (uint16_t i = 0; i < retry; ++i)
      if ((ret = dup(inFd[0])) >= 0) break;
    if (ret < 0) childFail("dup in call failed");

    close(1);
    for (uint16_t i = 0; i < retry; ++i)
      if ((ret = dup(outFd[1])) >= 0) break;
    if (ret < 0) childFail("dup out call failed");

    close(2);
    for (uint16_t i = 0; i < retry; ++i)
      if ((ret = dup(errFd[1])) >= 0) break;
    if (ret < 0) childFail("dup err call failed");

    /* SETUP EnvVars */  // SET ENV, it's a forked process so it doesn't matter -
                         // it'll die soon, like you.
    for (auto const& ev : vars()) env::SET(ev.first.c_str(), ev.second.c_str());

    try {
      if (!this->directory().empty()) mkn::kul::env::CWD(this->directory());
      this->child();
    } catch (...) {
    }
    childFail("exec failed");
  } else
    error(__LINE__, "Unhandled process id for child: " + std::to_string(pid()));
}

void Process::tearDown() {
  for (int* fd : {&errFd[1], &errFd[0], &outFd[1], &outFd[0], &inFd[1], &inFd[0]}) closeFd(*fd);
  popPip[0] = popPip[1] = popPip[2] = -1;
}

void Process::waitExit() KTHROW(mkn::kul::proc::ExitException) {
  tearDown();
  exitCode(WEXITSTATUS(cStat));
  finish();
  setFinished();
}

void Process::waitForStatus() {
  int32_t ret = 0;
  ret = recall(waitpid(pid(), &cStat, 0));
  (void)ret;
  assert(ret);
}

}  // namespace kul
}  // namespace mkn

#endif /* MKN_KUL_OS_NIXISH_PROC_HPP_ */
