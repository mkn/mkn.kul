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
#ifndef _KUL_OS_HPP_
#define _KUL_OS_HPP_

#include <vector>

#include "kul/cpu.hpp"
#include "kul/env.hpp"
#include "kul/except.hpp"
#include "kul/string.hpp"

#if KUL_IS_WIN
#include "kul/os/win/os.top.hpp"
#else
#include "kul/os/nixish/os.top.hpp"
#endif

namespace kul {

class Dir;
class File;

namespace fs {
class Exception : public kul::Exception {
 public:
  Exception(const char *f, const uint16_t &l, const std::string &s) : kul::Exception(f, l, s) {}
};

class TimeStamps {
 private:
  const uint64_t _a, _c, _m;
  TimeStamps(const uint64_t &a, const uint64_t &c, const uint64_t &m) : _a(a), _c(c), _m(m) {}

 public:
  const uint64_t &accessed() const { return _a; }
  const uint64_t &created() const { return _c; }
  const uint64_t &modified() const { return _m; }
  friend class kul::Dir;
  friend class kul::File;
};

class Item {
 public:
  virtual ~Item() {}
  virtual fs::TimeStamps timeStamps() const = 0;
  virtual std::string esc() const = 0;
  virtual std::string real() const = 0;
  virtual std::string mini() const = 0;
};
}  // namespace fs

class Dir : public fs::Item {
 private:
  std::string _p;
  static fs::TimeStamps TIMESTAMPS(const std::string &s) {
    uint64_t a = 0, c = 0, m = 0;
    fs::KulTimeStampsResolver::GET(s.c_str(), a, c, m);
    return fs::TimeStamps(a, c, m);
  }
  static inline std::string LOCL(std::string s);
  static void ESC_REPLACE(std::string &s, const std::string &f, const std::string &r) {
    size_t p = s.find(f);
    while (p != std::string::npos) {
      s.replace(s.find(f, p), f.size(), r);
      p = s.find(f, p + r.size());
    }
  }
  static inline std::string ESC(std::string s);
  static std::string PRNT(const std::string &s) {
    const std::string &p = s.substr(0, s.rfind(SEP()) + 1);
    return kul::Dir(p).root() ? p : s.substr(0, s.rfind(SEP()));
  }
  static std::string MINI(const std::string &a) {
    return a.find(env::CWD()) == std::string::npos
               ? a
               : a.compare(env::CWD()) == 0 ? a.substr(std::string(env::CWD()).size())
                                            : a.substr(std::string(env::CWD()).size() + 1);
  }

 public:
  Dir() : _p() {}
  Dir(const char *p, bool m = false) KTHROW(fs::Exception) : Dir(std::string(p), m) {}
  Dir(const std::string &p, bool m = false) KTHROW(fs::Exception) : _p(Dir::LOCL(p)) {
#ifndef _WIN32
    if (p.size() && p[0] == '~') this->_p = (env::GET("HOME") + p.substr(1));
#endif
    if (m && !is() && !mk()) KEXCEPT(fs::Exception, "Invalid directory path provided");
  }
  Dir(const Dir &d) : _p(d._p) {}
  Dir(const std::string &s, const Dir &d) : _p(d.join(s)) {}

  bool cp(const Dir &d) const;
  bool mv(const Dir &d) const { return std::rename(this->path().c_str(), d.path().c_str()); }

  inline void rm() const;
  inline bool is() const;
  inline bool mk() const;
  inline bool root() const;

  const std::string join(const std::string &s) const {
    return _p.size() == 0 ? s : root() ? path() + s : JOIN(path(), s);
  }
  const std::string name() const {
    return root() ? path() : path().substr(path().rfind(SEP()) + 1);
  }
  const std::string &path() const { return _p; }

  std::string esc() const { return ESC(path()); }
  std::string escr() const { return ESC(is() ? real() : path()); }
  std::string escm() const { return ESC(mini()); }
  std::string locl() const { return LOCL(path()); }
  std::string real() const { return REAL(path()); }
  std::string mini() const { return MINI(real()); }

  fs::TimeStamps timeStamps() const { return TIMESTAMPS(_p); }

  Dir parent() const { return Dir(PRNT(path())); }

  inline std::vector<Dir> dirs(bool incHidden = false) const KTHROW(fs::Exception);
  inline std::vector<File> files(bool recursive = false) const KTHROW(fs::Exception);

  static std::string JOIN(const std::string &a, const std::string &b) { return a + SEP() + b; }

  static inline std::string REAL(const std::string &s) KTHROW(fs::Exception);

#ifdef _WIN32
  static std::string SEP() { return std::string("\\"); }
#else
  static const std::string SEP() { return std::string("/"); }
#endif
  friend class File;

  Dir &operator=(const Dir &d) {
    this->_p = d._p;
    return *this;
  }
  bool operator==(const Dir &d) const {
    if (is() && d.is()) return real().compare(d.real()) == 0;
    return path().compare(d.path()) == 0;
  }
  explicit operator bool() const { return is(); }

  std::string relative(const kul::Dir &r) const {
    const auto &l(*this);
    std::string left = l.real(), right = r.real();
    auto l_p = l, r_p = r;
    while (!l_p.root()) l_p = l_p.parent().real();
    while (!r_p.root()) r_p = r_p.parent().real();
    if (l_p.path() != r_p.path()) return right;
    l_p = l;
    r_p = r;
    size_t upsies = 0, pos = 0;
    while (true) {
      left = l_p.real();
      if ((pos = right.find(left)) != std::string::npos) break;
      l_p = kul::Dir(l_p.real()).parent();
      upsies++;
    }
    std::string rel = "";
    for (size_t i = 0; i < upsies; i++) rel += ".." + Dir::SEP();
    right = right.substr(left.size());
    if (right[0] == Dir::SEP().c_str()[0]) right = right.substr(1);
    rel += right;
    return rel;
  }
};

inline std::ostream &operator<<(std::ostream &s, const Dir &d) { return s << d.path(); }

class File : public fs::Item {
 private:
  std::string _n;
  Dir _d;

 public:
  File() : _n(), _d() {}
  File(const std::string &n, bool m = false) : _n(Dir::LOCL(n)), _d(env::CWD()) {
    if (is()) {
      try {
        this->_n = Dir::REAL(this->_n);
        this->_d = Dir(Dir::PRNT(this->_n), m);
        this->_n = this->_n.substr(_d.path().size() + 1);
      } catch (const kul::fs::Exception &e) {
      }
    } else {
      if (n.find(Dir::SEP()) != std::string::npos) {
        this->_d = Dir(n.substr(0, n.rfind(Dir::SEP())));
        this->_n = this->_n.substr(n.rfind(Dir::SEP()) + 1);
      } else {
        try {
          this->_d = Dir(Dir::PRNT(Dir::REAL(this->_n)), m);
        } catch (const kul::fs::Exception &e) {
        }
      }
    }
  }
  File(const char *n, bool m = false) : File(std::string(n), m) {}
  File(const std::string &n, const Dir &d) : _n(n), _d(d) {}
  File(const std::string &n, const char *c) : _n(n), _d(c) {}
  File(const std::string &n, const std::string &d1) : _n(n), _d(d1) {}
  File(const File &f) : _n(f._n), _d(f._d) {}

  bool cp(const Dir &d) const {
    if (!d.is() && !d.mk()) KEXCEPT(fs::Exception, "Directory: \"" + _d.path() + "\" is not valid");
    return cp(kul::File(name(), d._p));
  }
  bool cp(const File &f) const {
    std::ifstream src(_d.join(_n), std::ios::binary);
    std::ofstream dst(f.dir().join(f.name()), std::ios::binary);
    return (bool)(dst << src.rdbuf());
  }

  inline bool is() const;
  inline bool mk() const;
  inline bool rm() const;

  bool mv(const File &f) const { return std::rename(this->full().c_str(), f.full().c_str()); }
  bool mv(const Dir &d) const {
    return std::rename(this->full().c_str(), d.join(this->name()).c_str());
  }

  const std::string &name() const { return _n; }

  std::string esc() const { return Dir::ESC(full()); }
  std::string escr() const { return Dir::ESC(is() ? real() : full()); }
  std::string escm() const { return Dir::ESC(mini()); }
  std::string full() const { return Dir::JOIN(_d.path(), _n); }
  std::string real() const { return Dir::JOIN(_d.real(), _n); }
  std::string mini() const { return Dir::MINI(real()); }

  inline uint64_t size() const;

  const Dir &dir() const { return _d; }
  fs::TimeStamps timeStamps() const { return Dir::TIMESTAMPS(mini()); }

  File &operator=(const File &f) = default;
  bool operator==(const File &f) const {
    if (is() && f.is()) return real().compare(f.real()) == 0;
    return full().compare(f.full()) == 0;
  }
  explicit operator bool() const { return is(); }

  std::string relative(const kul::Dir &r) const { return this->dir().relative(r); }

  std::string relative(const kul::File &r) const {
    std::string rel = this->dir().relative(r.dir());
    rel += Dir::SEP() + r.name();
    return rel;
  }
};

inline bool kul::Dir::cp(const Dir &d) const {
  if (!d.is() && !d.mk()) KEXCEPT(fs::Exception, "Directory: \"" + d.path() + "\" is not valid");
  Dir c(d.join(name()));
  c.mk();
  for (const auto &f : files()) f.cp(c);
  for (const auto &dd : dirs()) dd.cp(c);
  return 1;
}

inline std::ostream &operator<<(std::ostream &s, const File &d) { return s << d.full(); }

}  // namespace kul

namespace kul {
namespace os {

class PushDir {
 private:
  std::string cwd;
  kul::Dir m_dir;

 public:
  PushDir(const std::string &d) : m_dir(d) {
    if (!m_dir) KEXCEPTION("PushDir directory does not exist: ") << d;
    cwd = kul::env::CWD();
    kul::env::CWD(m_dir.real());
  }
  PushDir(const kul::Dir &d) : m_dir(d) {
    if (!m_dir) KEXCEPTION("PushDir directory does not exist: ") << d;
    cwd = kul::env::CWD();
    kul::env::CWD(m_dir.real());
  }
  ~PushDir() { kul::env::CWD(cwd); }
};

}  // namespace os

namespace env {

inline std::string WHERE(const char *c) {
  for (const auto &s : kul::String::SPLIT(env::GET("PATH"), kul::env::SEP())) {
    const kul::Dir d(s);
    if (d)
      for (const auto &f : d.files())
        if (f.name().compare(c) == 0) return f.real();
  }
  return "";
}

inline bool WHICH(const char *c) { return WHERE(c).size(); }
}  // namespace env
}  // namespace kul

#if KUL_IS_WIN
#include "kul/os/win/os.bot.hpp"
#else
#include "kul/os/nixish/os.bot.hpp"
#endif

#endif /* _KUL_OS_HPP_ */
