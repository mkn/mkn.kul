/**
Copyright (c) 2024, Philip Deegan.
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
#ifndef _MKN_KUL_PROC_HPP_
#define _MKN_KUL_PROC_HPP_

#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

#include "mkn/kul/cli.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/map.hpp"

namespace mkn {
namespace kul {

namespace this_proc {
int32_t id();
void kill(uint32_t const& e);

uint64_t virtualMemory();
uint64_t physicalMemory();
uint64_t totalMemory();
uint16_t cpuLoad();
}  // namespace this_proc

namespace proc {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class ExitException : public mkn::kul::proc::Exception {
 private:
  short const ec;

 public:
  ExitException(char const* f, uint16_t const& l, short const _ec, std::string const& s)
      : Exception(f, l, s), ec(_ec) {}
  short const& code() const { return ec; }
};

class Call {
 private:
  std::string cwd;
  std::string const d;
  std::string const& s;
  mkn::kul::hash::map::S2S oldEvs;
  void setCWD() {
    if (d.size()) {
      cwd = mkn::kul::env::CWD();
      if (!mkn::kul::env::CWD(d)) KEXCEPTION("FAILED TO SET DIRECTORY: " + d);
    }
  }

 public:
  ~Call() {
    if (d.size()) mkn::kul::env::CWD(cwd);
    for (auto const& oldEv : oldEvs) mkn::kul::env::SET(oldEv.first.c_str(), oldEv.second.c_str());
  }
  Call(std::string const& _s, std::string const& _d = "") : d(_d), s(_s) { setCWD(); }
  Call(std::string const& _s, mkn::kul::hash::map::S2S const& evs, std::string const& _d = "")
      : d(_d), s(_s) {
    setCWD();
    for (auto const& ev : evs) {
      std::string const& v = mkn::kul::env::GET(ev.first.c_str());
      if (v.size()) oldEvs.insert(ev.first, v);
      mkn::kul::env::SET(ev.first.c_str(), ev.second.c_str());
    }
  }
  int run() { return s.size() ? mkn::kul::os::exec(s) : 1; }
};
}  // namespace proc

class AProcess {
 private:
  bool f = 0, s = 0;
  bool const wfe = 1;
  int32_t pec = -1, pi = 0;
  std::string const d;
  std::function<void(std::string const&)> e, o;
  std::vector<std::string> argv;
  mkn::kul::hash::map::S2S evs;
  friend std::ostream& operator<<(std::ostream&, AProcess const&);

 protected:
  AProcess(std::string const& cmd, bool const& _wfe) : wfe(_wfe) { argv.push_back(cmd); }
  AProcess(std::string const& cmd, std::string const& _d, bool const& _wfe) : wfe(_wfe), d(_d) {
    argv.push_back(cmd);
  }
  virtual ~AProcess() {}

  std::string const& directory() const { return d; }
  void setFinished() { f = 1; }
  virtual bool kill(int16_t k = 6) = 0;
  virtual void preStart() {}
  virtual void finish() {}
  virtual void tearDown() {}
  virtual void run() KTHROW(mkn::kul::Exception) = 0;
  bool waitForExit() const { return wfe; }
  void pid(int32_t const& _pi) { this->pi = _pi; }

  std::vector<std::string> const& args() const { return argv; };
  mkn::kul::hash::map::S2S const& vars() const { return evs; }
  virtual void out(std::string const& _s) {
    if (this->o)
      this->o(_s);
    else
      printf("%s", _s.c_str());
  }
  virtual void err(std::string const& _s) {
    if (this->e)
      this->e(_s);
    else
      fprintf(stderr, "%s", _s.c_str());
  }
  void error(int16_t const& line, std::string const& _s) KTHROW(mkn::kul::Exception) {
    tearDown();
    throw Exception("mkn/kul/proc.hpp", line, _s);
  }
  void exitCode(int32_t const& _e) { pec = _e; }

 public:
  template <class T>
  AProcess& arg(T const& a) {
    std::stringstream ss;
    ss << a;
    if (ss.str().size()) argv.push_back(ss.str());
    return *this;
  }
  AProcess& arg(std::string const& a) {
    argv.push_back(a);
    return *this;
  }
  AProcess& args(std::string const& a) {
    if (a.size())
      for (auto const& c : mkn::kul::cli::asArgs(a)) argv.push_back(c);
    return *this;
  }
  AProcess& var(std::string const& n, std::string const& v) {
    evs[n] = v;
    return *this;
  }
  AProcess& set(std::vector<mkn::kul::cli::EnvVar> const& in) {
    for (auto const& ev : in) evs[ev.name()] = ev.toString();
    return *this;
  }

  virtual inline void start() KTHROW(mkn::kul::Exception);
  virtual inline std::string toString() const;

  int32_t const& pid() const { return pi; }
  bool started() const { return pi > 0; }
  bool finished() const { return f; }
  int32_t const& exitCode() { return pec; }
  AProcess& operator<<(std::string const& _s) {
    arg(_s);
    return *this;
  }
  void setOut(std::function<void(std::string const&)> _o) { this->o = _o; }
  void setErr(std::function<void(std::string const&)> _e) { this->e = _e; }
};

inline std::ostream& operator<<(std::ostream& s, AProcess const& p) { return s << p.toString(); }

class ProcessCapture {
 public:
  ProcessCapture() {}
  ProcessCapture(AProcess& p) { setProcess(p); }
  ProcessCapture(ProcessCapture const& pc) : so(pc.so.str()), se(pc.se.str()) {}

  virtual ~ProcessCapture() {}

  std::string const outs() const { return so.str(); }
  std::string const errs() const { return se.str(); }

  void setProcess(AProcess& p) {
    p.setOut(std::bind(&ProcessCapture::out, std::ref(*this), std::placeholders::_1));
    p.setErr(std::bind(&ProcessCapture::err, std::ref(*this), std::placeholders::_1));
  }

 protected:
  virtual void out(std::string const& s) { so << s; }
  virtual void err(std::string const& s) { se << s; }

 private:
  std::stringstream so, se;
};
}  // namespace kul
}  // namespace mkn

#ifndef _MKN_KUL_COMPILED_LIB_
#include "mkn/kul/src/proc.base/start.ipp"
#include "mkn/kul/src/proc.base/toString.ipp"
#endif

#if defined(_WIN32)
#include "mkn/kul/os/win/proc.hpp"
#else
#include "mkn/kul/os/nixish/proc.hpp"
#endif

#endif  // _MKN_KUL_PROC_HPP_