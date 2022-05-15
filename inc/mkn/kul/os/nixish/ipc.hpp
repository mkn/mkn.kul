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
// IWYU pragma: private, include "mkn/kul/ipc.hpp"

#ifndef _MKN_KUL_OS_NIXISH_IPC_HPP_
#define _MKN_KUL_OS_NIXISH_IPC_HPP_

#ifndef _MKN_KUL_IPC_UUID_PREFIX_
#define _MKN_KUL_IPC_UUID_PREFIX_ "/tmp/pipe"
#endif

#include "mkn/kul/log.hpp"
#include "mkn/kul/proc.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 512

namespace mkn {
namespace kul {
namespace ipc {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const *f, uint16_t const &l, std::string const &s)
      : mkn::kul::Exception(f, l, s) {}
};

class IPCCall {
 protected:
  int16_t fd;
  void writePID() const {
    std::string s = std::to_string(this_proc::id());
    while (s.size() < 9) s = "0" + s;
    write(fd, s.c_str(), 9);
  }
  void writeLength(std::string const &m) const {
    std::string s = std::to_string(m.size());
    while (s.size() < 3) s = "0" + s;
    write(fd, s.c_str(), 3);
  }
};
class Server : public IPCCall {
 private:
  int16_t lp;
  const mkn::kul::File uuid;

  void start() KTHROW(Exception) {
    uuid.dir().mk();
    mkfifo(uuid.full().c_str(), 0666);
  }

 protected:
  virtual void handle(std::string const &s) { KLOG(INF) << s; }
  void respond(std::string const &s);

 public:
  virtual ~Server() {}
  void listen() KTHROW(Exception) {
    char buff[BUFSIZE];
    while (lp) {
      memset(buff, 0, BUFSIZE);
      fd = open(uuid.full().c_str(), O_RDONLY);
      if (fd == -1) KEXCEPT(mkn::kul::ipc::Exception, "Cannot open FIFO for read");
      int16_t l;
      read(fd, buff, 3);
      std::istringstream ssl(buff);
      ssl >> l;
      memset(buff, 0, BUFSIZE);
      read(fd, buff, l);
      handle(buff);
      close(fd);
      if (lp != -1) lp--;
    }
  }
  Server(const int16_t &_lp = -1) KTHROW(Exception)
      : lp(_lp),
        uuid(std::to_string(mkn::kul::this_proc::id()),
             Dir(_MKN_KUL_IPC_UUID_PREFIX_ + std::string("/pid/"))) {
    start();
  }
  Server(std::string const &ui, const int16_t &_lp = -1) KTHROW(Exception)
      : lp(_lp), uuid(ui, Dir(_MKN_KUL_IPC_UUID_PREFIX_)) {
    start();
  }
};

class Client : public IPCCall {
 private:
  const mkn::kul::File uuid;

  void start() KTHROW(Exception) {
    fd = open(uuid.full().c_str(), O_WRONLY);
    if (fd == -1) KEXCEPT(mkn::kul::ipc::Exception, "Cannot contact server");
  }
  void stop() const KTHROW(Exception) { close(fd); }

 public:
  virtual ~Client() { stop(); }
  Client(std::string const &ui) KTHROW(Exception) : uuid(ui, Dir(_MKN_KUL_IPC_UUID_PREFIX_)) {
    start();
  }
  Client(const int16_t &pid) KTHROW(Exception)
      : uuid(std::to_string(pid), Dir(_MKN_KUL_IPC_UUID_PREFIX_ + std::string("/pid/"))) {
    start();
  }
  virtual void send(std::string const &m) const KTHROW(Exception) {
    writeLength(m);
    write(fd, m.c_str(), m.size());
  }
};

}  // namespace ipc
}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_OS_NIXISH_IPC_HPP_ */
