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
#ifndef _MKN_KUL_ASIO_LOG_HPP_
#define _MKN_KUL_ASIO_LOG_HPP_

#include "mkn/kul/log.hpp"
#include "mkn/kul/threads.hpp"

namespace mkn {
namespace kul {
namespace asio {
namespace log {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const *f, uint16_t const &l, std::string const &s)
      : mkn::kul::Exception(f, l, s) {}
};
}  // namespace log

class LogMan;
class Logger : public mkn::kul::Logger {
  friend class LogMan;

 private:
  mkn::kul::ChroncurrentThreadPool<> ctp;
  std::function<void(std::string const &)> defE, defO;

 public:
  Logger()
      : ctp(1, 1),
        defE([&](std::string const &s) { mkn::kul::Logger::err(s); }),
        defO([&](std::string const &s) { mkn::kul::Logger::out(s); }) {}
  void err(std::string const &s) override {
    if (e)
      ctp.async(std::bind(e, s));
    else
      ctp.async(std::bind(defE, s));
  }
  void out(std::string const &s) override {
    if (o)
      ctp.async(std::bind(o, s));
    else
      ctp.async(std::bind(defO, s));
  }
};

class LogMan : public mkn::kul::ALogMan {
 protected:
  LogMan() : ALogMan(new mkn::kul::asio::Logger()) {}

 public:
  static LogMan &INSTANCE() {
    static LogMan instance;
    return instance;
  };
};

class Message {
 protected:
  std::stringstream ss;
  const mkn::kul::log::mode &m;

  Message(const mkn::kul::log::mode &_m) : m(_m) {}

 public:
  template <class T>
  Message &operator<<(const T &s) {
    ss << s;
    return *this;
  }
};
class LogMessage : public Message {
 public:
  ~LogMessage() { LogMan::INSTANCE().log(f, fn, l, m, ss.str()); }
  LogMessage(char const *_f, char const *_fn, uint16_t const &_l, const mkn::kul::log::mode &_m)
      : Message(_m), f(_f), fn(_fn), l(_l) {}

 private:
  char const *f, *fn;
  uint16_t const &l;
};
struct OutMessage : public Message {
  OutMessage(const mkn::kul::log::mode &_m = mkn::kul::log::mode::NON) : Message(_m) {}
  ~OutMessage() { LogMan::INSTANCE().out(m, ss.str()); }
};
struct ErrMessage : public Message {
  ErrMessage() : Message(mkn::kul::log::mode::ERR) {}
  ~ErrMessage() { LogMan::INSTANCE().err(ss.str()); }
};

#define KASIO_LOG_INF \
  mkn::kul::asio::LogMessage(__FILE__, __func__, __LINE__, mkn::kul::log::mode::INF)
#define KASIO_LOG_ERR \
  mkn::kul::asio::LogMessage(__FILE__, __func__, __LINE__, mkn::kul::log::mode::ERR)
#define KASIO_LOG_DBG \
  mkn::kul::asio::LogMessage(__FILE__, __func__, __LINE__, mkn::kul::log::mode::DBG)
#define KASIO_LOG_TRC \
  mkn::kul::asio::LogMessage(__FILE__, __func__, __LINE__, mkn::kul::log::mode::TRC)
#define KASIO_LOG(sev) KLOG_##sev

#define KASIO_OUT_NON mkn::kul::asio::OutMessage()
#define KASIO_OUT_INF mkn::kul::asio::OutMessage(mkn::kul::log::mode::INF)
#define KASIO_OUT_ERR mkn::kul::asio::OutMessage(mkn::kul::log::mode::ERR)
#define KASIO_OUT_DBG mkn::kul::asio::OutMessage(mkn::kul::log::mode::DBG)
#define KASIO_OUT_TRC mkn::kul::asio::OutMessage(mkn::kul::log::mode::TRC)
#define KASIO_OUT(sev) KASIO_OUT_##sev

#define KASIO_ERR mkn::kul::ErrMessage()

}  // namespace asio
}  // namespace kul
}  // namespace mkn
#endif /* _MKN_KUL_LOG_HPP_ */
