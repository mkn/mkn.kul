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
#ifndef _KUL_LOG_HPP_
#define _KUL_LOG_HPP_

#include <string.h>
#include <memory>
#include <string>

#include "kul/os.hpp"
#include "kul/os/threads.hpp"
#include "kul/time.hpp"

#ifndef __KUL_LOG_TIME_FRMT__
#define __KUL_LOG_TIME_FRMT__ "%Y-%m-%d-%H:%M:%S:%i"
#endif

#ifndef __KUL_LOG_FRMT__
#define __KUL_LOG_FRMT__ "[%M]: %T - %D : %F fn(%N)#%L - %S"
#endif

namespace kul {
namespace log {

enum mode { OFF = -1, NON = 0, INF, ERR, DBG, OTH, TRC };

class Exception : public kul::Exception {
 public:
  Exception(char const *f, uint16_t const &l, std::string const &s) : kul::Exception(f, l, s) {}
};
}  // namespace log

class ALogMan;
class Logger {
  friend class ALogMan;

 protected:
  std::function<void(std::string const &)> e, o;
  const std::string modeTxt(const log::mode &m) const {
    std::string s("NON");
    if (m == 1)
      s = "INF";
    else if (m == 2)
      s = "ERR";
    else if (m == 3)
      s = "DBG";
    else if (m == 4)
      s = "OTH";
    else if (m == 5)
      s = "TRC";
    return s;
  }

 public:
  virtual ~Logger() {}
  void str(char const *f, char const *fn, uint16_t const &l, std::string const &s,
           const log::mode &m, std::string &str) {
    kul::String::REPLACE(str, "%M", modeTxt(m));
    kul::String::REPLACE(str, "%T", kul::this_thread::id());
    kul::String::REPLACE(str, "%D", kul::DateTime::NOW(__KUL_LOG_TIME_FRMT__));
    kul::String::REPLACE(str, "%F", f);
    kul::String::REPLACE(str, "%N", fn);
    kul::String::REPLACE(str, "%L", std::to_string(l));
    kul::String::REPLACE(str, "%S", s);
  }
  virtual void err(std::string const &s) {
    if (e)
      e(s);
    else
      fprintf(stderr, "%s", s.c_str());
  }
  virtual void out(std::string const &s) {
    if (o)
      o(s);
    else
      printf("%s", s.c_str());
  }
  void log(char const *f, char const *fn, uint16_t const &l, std::string const &s,
           const log::mode &m) {
    std::string st(__KUL_LOG_FRMT__);
    str(f, fn, l, s, m, st);
    out(st + kul::os::EOL());
  }
  void setOut(std::function<void(std::string const &)> _o) { this->o = _o; }
  void setErr(std::function<void(std::string const &)> _e) { this->e = _e; }
};

class ALogMan {
 protected:
  log::mode m;
  mutable std::unique_ptr<Logger> logger;
  ALogMan(Logger *_logger) : m(kul::log::mode::NON), logger(_logger) {
    std::string s(kul::env::GET("KLOG"));
    if (s.size()) {
      kul::String::TRIM(s);
      if (s == "-1" || s == "OFF")
        m = log::mode::OFF;
      else if (s == "0" || s == "NON")
        m = log::mode::NON;
      else if (s == "1" || s == "INF")
        m = log::mode::INF;
      else if (s == "2" || s == "ERR")
        m = log::mode::ERR;
      else if (s == "3" || s == "DBG")
        m = log::mode::DBG;
      else if (s == "4" || s == "OTH")
        m = log::mode::OTH;
      else if (s == "5" || s == "TRC")
        m = log::mode::TRC;
      else {
        m = log::mode::ERR;
        out(m, "ERROR DISCERNING LOG LEVEL, ERROR LEVEL IN USE");
      }
    }
  }

 public:
  virtual ~ALogMan() {}
  void setMode(const log::mode &m1) { m = m1; }
  bool inf() { return m >= log::INF; }
  bool err() { return m >= log::ERR; }
  bool dbg() { return m >= log::DBG; }
  void log(char const *f, char const *fn, uint16_t const &l, const log::mode &_m,
           std::string const &s) {
    if (this->m >= _m) logger->log(f, fn, l, s, _m);
  }
  void out(const log::mode &_m, std::string const &s) {
    if (this->m >= _m) logger->out(s + kul::os::EOL());
  }
  void err(std::string const &s) { logger->err(s + kul::os::EOL()); }
  std::string str(char const *f, char const *fn, uint16_t const &l, const log::mode &_m,
                  std::string const &s = "", const std::string fmt = __KUL_LOG_FRMT__) {
    std::string st(fmt);
    logger->str(f, fn, l, s, _m, st);
    return st;
  }
  void setOut(std::function<void(std::string const &)> o) { logger->setOut(o); }
  void setErr(std::function<void(std::string const &)> e) { logger->setErr(e); }
};

class LogMan : public ALogMan {
 protected:
  LogMan() : ALogMan(new Logger()) {}

 public:
  static LogMan &INSTANCE() {
    static LogMan instance;
    return instance;
  };
};

class Message {
 protected:
  std::stringstream ss;
  const log::mode &m;

  Message(const log::mode &_m) : m(_m) {}

 public:
  template <class T>
  Message &operator<<(const T &s) {
    ss << s;
    return *this;
  }
};
class LogMessage : public Message {
 public:
  LogMessage(char const *_f, char const *_fn, uint16_t const &_l, const log::mode &_m)
      : Message(_m), f(_f), fn(_fn), l(_l) {}
  ~LogMessage() { LogMan::INSTANCE().log(f, fn, l, m, ss.str()); }

 private:
  char const *f, *fn;
  uint16_t const &l;
};
class DBgMessage : public Message {
 public:
  ~DBgMessage() { KUL_DEBUG_DO(LogMan::INSTANCE().log(f, fn, l, m, ss.str())); }

#if !defined(NDEBUG)
  DBgMessage(char const *_f, char const *_fn, uint16_t const &_l, const log::mode &_m)
      : Message(_m), f(_f), fn(_fn), l(_l) {}
#else
  DBgMessage() : Message(kul::log::mode::NON) {}
#endif

  template <class T>
  DBgMessage &operator<<([[maybe_unused]] const T &s) {
    KUL_DEBUG_DO(ss << s;)
    return *this;
  }

 private:
  KUL_DEBUG_DO(char const *f, *fn;)
  KUL_DEBUG_DO(uint16_t const &l;)
};
class OutMessage : public Message {
 public:
  ~OutMessage() { LogMan::INSTANCE().out(m, ss.str()); }
  OutMessage(const log::mode &_m = kul::log::mode::NON) : Message(_m) {}
};
class ErrMessage : public Message {
 public:
  ~ErrMessage() { LogMan::INSTANCE().err(ss.str()); }
  ErrMessage() : Message(log::mode::ERR) {}
};
class DBoMessage : public Message {
 public:
  ~DBoMessage() { KUL_DEBUG_DO(LogMan::INSTANCE().out(m, ss.str())); }
  DBoMessage(const log::mode &_m = kul::log::mode::NON) : Message(_m) {}
  template <class T>
  DBoMessage &operator<<([[maybe_unused]] const T &s) {
    KUL_DEBUG_DO(ss << s;)
    return *this;
  }
};

#define KLOG_NON kul::LogMessage(__FILE__, __func__, __LINE__, kul::log::mode::NON)
#define KLOG_INF kul::LogMessage(__FILE__, __func__, __LINE__, kul::log::mode::INF)
#define KLOG_ERR kul::LogMessage(__FILE__, __func__, __LINE__, kul::log::mode::ERR)
#define KLOG(sev) KLOG_##sev

#if !defined(NDEBUG)
#define KLOG_DBG kul::DBgMessage(__FILE__, __func__, __LINE__, kul::log::mode::DBG)
#define KLOG_OTH kul::DBgMessage(__FILE__, __func__, __LINE__, kul::log::mode::OTH)
#define KLOG_TRC kul::DBgMessage(__FILE__, __func__, __LINE__, kul::log::mode::TRC)
#else
#define KLOG_DBG kul::DBgMessage()
#define KLOG_OTH kul::DBgMessage()
#define KLOG_TRC kul::DBgMessage()
#endif

#define KOUT_NON kul::OutMessage()
#define KOUT_INF kul::OutMessage(kul::log::mode::INF)
#define KOUT_ERR kul::OutMessage(kul::log::mode::ERR)
#define KOUT_DBG kul::DBoMessage(kul::log::mode::DBG)
#define KOUT_OTH kul::DBoMessage(kul::log::mode::OTH)
#define KOUT_TRC kul::DBoMessage(kul::log::mode::TRC)
#define KOUT(sev) KOUT_##sev

#define KERR kul::ErrMessage()

}  // namespace kul
#endif /* _KUL_LOG_HPP_ */
