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
#ifndef MKN_KUL_OS_HPP_
#define MKN_KUL_OS_HPP_

#include "mkn/kul/defs.hpp"
#include "mkn/kul/env.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/string.hpp"
#include "mkn/kul/lang/cpp/compiler/def.hpp"

#if MKN_KUL_IS_WIN
#include "mkn/kul/os/win/os.top.hpp"  // IWYU pragma: export
#else
#include "mkn/kul/os/nixish/os.top.hpp"  // IWYU pragma: export
#endif

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <vector>
#include <optional>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>

namespace mkn {
namespace kul {

class Dir;
class File;

namespace fsys {
class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class TimeStamps {
 private:
  uint64_t const _a, _c, _m;
  TimeStamps(uint64_t const& a, uint64_t const& c, uint64_t const& m) : _a(a), _c(c), _m(m) {}

 public:
  uint64_t const& accessed() const { return _a; }
  uint64_t const& created() const { return _c; }
  uint64_t const& modified() const { return _m; }
  friend class mkn::kul::Dir;
  friend class mkn::kul::File;
};

class Item {
 public:
  virtual ~Item() {}
  virtual fsys::TimeStamps timeStamps() const = 0;
  virtual std::string esc() const = 0;
  virtual std::string real() const = 0;
  virtual std::string mini() const = 0;
};

struct ListOpts {
  bool recursive = false;  // also list the contents of subdirectories
  bool hidden = false;     // include ".name" directories, and with recursive descend into them
};

namespace detail {
inline bool is_hidden(std::filesystem::path const& p) {
  auto const name = p.filename().string();
  return !name.empty() && name[0] == '.';
}

template <typename Fn>
void each_entry(std::filesystem::path const& dir, Fn&& fn) KTHROW(Exception) {
  std::error_code ec;
  if (!std::filesystem::is_directory(dir, ec))
    KEXCEPT(Exception, "Directory : \"" + dir.string() + "\" does not exist");
  std::filesystem::directory_iterator it(dir, ec), end;
  for (; !ec && it != end; it.increment(ec)) fn(*it);
  if (ec) KEXCEPT(Exception, "Directory : \"" + dir.string() + "\" could not be read");
}

inline void dirs(std::filesystem::path const& dir, ListOpts const& opts,
                 std::vector<std::filesystem::path>& out) KTHROW(Exception) {
  each_entry(dir, [&](auto const& entry) {
    std::error_code ec;
    if ((opts.hidden || !is_hidden(entry.path())) && entry.is_directory(ec)) {
      out.emplace_back(entry.path());
      if (opts.recursive) dirs(entry.path(), opts, out);
    }
  });
}

inline void files(std::filesystem::path const& dir, ListOpts const& opts,
                  std::vector<std::filesystem::path>& out) KTHROW(Exception) {
  each_entry(dir, [&](auto const& entry) {
    std::error_code ec;
    if (!entry.is_directory(ec))
      out.emplace_back(entry.path());
    else if (opts.recursive && (opts.hidden || !is_hidden(entry.path())))
      files(entry.path(), opts, out);
  });
}
}  // namespace detail

// the directories in dir, as dir / name, sorted
MKN_KUL_FN_SIG_RET inline std::vector<std::filesystem::path> dirs(std::filesystem::path const& dir,
                                                                  ListOpts const& opts = {})
    KTHROW(Exception) {
  std::vector<std::filesystem::path> out;
  detail::dirs(dir, opts, out);
  std::ranges::sort(out);
  return out;
}

// everything in dir that is not a directory (hidden files included), as dir / name, sorted
MKN_KUL_FN_SIG_RET inline std::vector<std::filesystem::path> files(std::filesystem::path const& dir,
                                                                   ListOpts const& opts = {})
    KTHROW(Exception) {
  std::vector<std::filesystem::path> out;
  detail::files(dir, opts, out);
  std::ranges::sort(out);
  return out;
}

// the platform's separators, "\\" to "/" on nix and "/" to "\\" on Windows
MKN_KUL_FN_SIG_RET inline std::string local_separators(std::string s);
// local_separators
MKN_KUL_FN_SIG_RET inline std::string locl(std::string s) { return local_separators(std::move(s)); }

namespace detail {
// for a shell, spaces escaped on nix and backslashes doubled on Windows
inline std::string esc(std::string s);

// the part of real after the working directory, or real if it's not under it
inline std::string trim_cwd(std::string const& real) {
  std::string const cwd = env::CWD();
  if (real.find(cwd) == std::string::npos) return real;
  return real == cwd ? real.substr(cwd.size()) : real.substr(cwd.size() + 1);
}
}  // namespace detail

// p absolute with "." / ".." resolved, p need not exist. Symlinks are resolved on nix (as
//  realpath), not on Windows (as _fullpath)
MKN_KUL_FN_SIG_RET inline std::string real(std::filesystem::path const& p) KTHROW(Exception) {
  std::error_code ec;
#if MKN_KUL_IS_WIN
  auto r = std::filesystem::absolute(p, ec).lexically_normal().string();
  if (r.size() && r.back() == '\\') r.pop_back();
#else
  auto r = std::filesystem::weakly_canonical(p, ec).string();
#endif
  if (ec) KEXCEPT(Exception, "Path \"" + p.string() + "\" could not be resolved");
  return r;
}

// real(p) relative to the working directory when under it, as File / Dir::mini()
MKN_KUL_FN_SIG_RET inline std::string cwd_relative(std::filesystem::path const& p)
    KTHROW(Exception) {
  return detail::trim_cwd(real(p));
}
// cwd_relative
MKN_KUL_FN_SIG_RET inline std::string mini(std::filesystem::path const& p) KTHROW(Exception) {
  return cwd_relative(p);
}

// p as given, for a shell, as File / Dir::esc()
MKN_KUL_FN_SIG_RET inline std::string escaped(std::filesystem::path const& p) {
  return detail::esc(local_separators(p.string()));
}
// escaped
MKN_KUL_FN_SIG_RET inline std::string esc(std::filesystem::path const& p) { return escaped(p); }

// real(p) for a shell, as File / Dir::escr() of an existing path
MKN_KUL_FN_SIG_RET inline std::string escaped_real(std::filesystem::path const& p)
    KTHROW(Exception) {
  return detail::esc(real(p));
}
// escaped_real
MKN_KUL_FN_SIG_RET inline std::string escr(std::filesystem::path const& p) KTHROW(Exception) {
  return escaped_real(p);
}

// cwd_relative(p) for a shell, as File / Dir::escm()
MKN_KUL_FN_SIG_RET inline std::string escaped_mini(std::filesystem::path const& p)
    KTHROW(Exception) {
  return detail::esc(cwd_relative(p));
}
// escaped_mini
MKN_KUL_FN_SIG_RET inline std::string escm(std::filesystem::path const& p) KTHROW(Exception) {
  return escaped_mini(p);
}

// last modification, seconds since the epoch, 0 if p can't be read
MKN_KUL_FN_SIG_RET inline std::uint64_t modified(std::filesystem::path const& p) {
  std::uint64_t a = 0, c = 0, m = 0;
  detail::timestamps(p.string().c_str(), a, c, m);
  return m;
}

}  // namespace fsys

class Dir : public fsys::Item {
 private:
  std::string _p;
  static fsys::TimeStamps TIMESTAMPS(std::string const& s) {
    uint64_t a = 0, c = 0, m = 0;
    fsys::detail::timestamps(s.c_str(), a, c, m);
    return fsys::TimeStamps(a, c, m);
  }
  static std::string LOCL(std::string s) { return fsys::locl(std::move(s)); }
  static std::string ESC(std::string s) { return fsys::detail::esc(std::move(s)); }
  static std::string PRNT(std::string const& s) {  // parent
    std::string const& p = s.substr(0, s.rfind(SEP()) + 1);
    return mkn::kul::Dir(p).root() ? p : s.substr(0, s.rfind(SEP()));
  }
  static std::string MINI(std::string const& a) { return fsys::detail::trim_cwd(a); }

 public:
  Dir() : _p() {}
  Dir(char const* p, bool m = false) KTHROW(fsys::Exception) : Dir(std::string(p), m) {}
  Dir(std::string const& p, bool m = false) KTHROW(fsys::Exception) : _p(Dir::LOCL(p)) {
#ifndef _WIN32
    if (p.size() && p[0] == '~') this->_p = (env::GET("HOME") + p.substr(1));
#endif
    if (m && !is() && !mk()) KEXCEPT(fsys::Exception, "Invalid directory path provided");
  }
  Dir(Dir const& d) : _p(d._p) {}
  Dir(std::string const& s, Dir const& d) : _p(d.join(s)) {}

  bool cp(Dir const& d) const;
  inline bool mv(Dir const& d) const;

  inline void rm() const;
  inline bool is() const;
  inline bool mk() const;
  inline bool root() const;

  std::string const join(std::string const& s) const {
    return _p.size() == 0 ? s : root() ? path() + s : JOIN(path(), s);
  }
  std::string const name() const {
    return root() ? path() : path().substr(path().rfind(SEP()) + 1);
  }
  std::string const& path() const { return _p; }

  std::string esc() const { return ESC(path()); }
  std::string escr() const { return ESC(is() ? real() : path()); }
  std::string escm() const { return ESC(mini()); }
  std::string locl() const { return LOCL(path()); }
  std::string real() const { return REAL(path()); }
  std::string mini() const { return MINI(real()); }

  fsys::TimeStamps timeStamps() const { return TIMESTAMPS(_p); }

  Dir parent() const { return Dir(PRNT(path())); }

  inline std::vector<Dir> dirs(bool incHidden = false) const KTHROW(fsys::Exception);
  inline std::vector<File> files(bool recursive = false) const KTHROW(fsys::Exception);

  static std::string JOIN(std::string const& a, std::string const& b) { return a + SEP() + b; }

  static inline std::string REAL(std::string const& s) KTHROW(fsys::Exception);
  static inline std::optional<std::string> REAL_OR_NULL(std::string const& s)
      KTHROW(fsys::Exception);

#ifdef _WIN32
  static std::string SEP() { return std::string("\\"); }
#else
  static const std::string SEP() { return std::string("/"); }
#endif
  friend class File;

  Dir& operator=(Dir const& d) = default;
  bool operator==(Dir const& d) const {
    if (is() && d.is()) return real().compare(d.real()) == 0;
    return path().compare(d.path()) == 0;
  }
  explicit operator bool() const { return is(); }

  std::string relative(mkn::kul::Dir const& r) const {
    auto const& l(*this);
    std::string left = l.real(), right = r.real();
    auto l_p = l, r_p = r;
    while (!l_p.root()) l_p = l_p.parent().real();
    while (!r_p.root()) r_p = r_p.parent().real();
    if (l_p.path() != r_p.path()) return right;
    l_p = l;
    r_p = r;
    size_t upsies = 0;
    while (true) {
      left = l_p.real();
      if (right.find(left) != std::string::npos) break;
      l_p = mkn::kul::Dir(l_p.real()).parent();
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

inline std::ostream& operator<<(std::ostream& s, Dir const& d) { return s << d.path(); }

class File : public fsys::Item {
 private:
  std::string _n;
  Dir _d;

 public:
  File() : _n(), _d() {}
  File(std::string const& n, bool m = false) : _n(Dir::LOCL(n)) {
    // _d starts empty so is() stats _n as-given (Dir::join returns it unprefixed),
    // correctly resolving both absolute and CWD-relative input before any CWD default is applied.
    if (is()) {
      try {
        this->_n = Dir::REAL(this->_n);
        this->_d = Dir(Dir::PRNT(this->_n), m);
        this->_n = this->_n.substr(_d.path().size() + 1);
      } catch (mkn::kul::fsys::Exception const& e) {
        std::cerr << e.debug() << std::endl;
      }
    } else if (n.find(Dir::SEP()) != std::string::npos) {
      this->_d = Dir(n.substr(0, n.rfind(Dir::SEP())));
      this->_n = this->_n.substr(n.rfind(Dir::SEP()) + 1);
    } else if (auto const& real = Dir::REAL_OR_NULL(this->_n)) {
      this->_d = Dir(Dir::PRNT(*real), m);
    } else {
      this->_d = Dir(env::CWD());
    }
  }
  File(char const* n, bool m = false) : File(std::string(n), m) {}
  File(std::string const& n, Dir const& d) : _n(n), _d(d) {}
  File(std::string const& n, char const* c) : _n(n), _d(c) {}
  File(std::string const& n, std::string const& d1) : _n(n), _d(d1) {}
  File(File const& f) : _n(f._n), _d(f._d) {}

  bool cp(Dir const& d) const {
    if (!d.is() && !d.mk())
      KEXCEPT(fsys::Exception, "Directory: \"" + _d.path() + "\" is not valid");
    return cp(mkn::kul::File(name(), d._p));
  }
  inline bool cp(File const& f) const;

  inline bool is() const;
  inline bool mk() const;
  inline bool rm() const;

  inline bool mv(File const& f) const;
  bool mv(Dir const& d) const { return mv(File(name(), d)); }

  std::string const& name() const { return _n; }

  std::string esc() const { return Dir::ESC(full()); }
  std::string escr() const { return Dir::ESC(is() ? real() : full()); }
  std::string escm() const { return Dir::ESC(mini()); }
  std::string full() const { return Dir::JOIN(_d.path(), _n); }
  std::string real() const { return Dir::JOIN(_d.real(), _n); }
  std::string mini() const { return Dir::MINI(real()); }

  inline uint64_t size() const;

  Dir const& dir() const { return _d; }
  fsys::TimeStamps timeStamps() const { return Dir::TIMESTAMPS(mini()); }

  File& operator=(File const& f) = default;
  bool operator==(File const& f) const {
    if (is() && f.is()) return real().compare(f.real()) == 0;
    return full().compare(f.full()) == 0;
  }
  explicit operator bool() const { return is(); }

  std::string relative(mkn::kul::Dir const& r) const { return this->dir().relative(r); }

  std::string relative(mkn::kul::File const& r) const {
    std::string rel = this->dir().relative(r.dir());
    rel += Dir::SEP() + r.name();
    return rel;
  }
};

inline bool mkn::kul::Dir::mv(Dir const& d) const {
  std::error_code ec;
  std::filesystem::rename(path(), d.path(), ec);
  return !ec;
}

inline void mkn::kul::Dir::rm() const {
  std::error_code ec;
  if (is()) std::filesystem::remove_all(path(), ec);
}

inline bool mkn::kul::Dir::is() const {
  std::error_code ec;
  return !path().empty() && std::filesystem::is_directory(path(), ec);
}

inline bool mkn::kul::Dir::mk() const {
  std::error_code ec;
  return !path().empty() && std::filesystem::create_directories(locl(), ec);
}

inline bool mkn::kul::Dir::root() const {
  if (!is()) return false;
  std::filesystem::path const p(real());
  return p == p.root_path();
}

inline std::vector<mkn::kul::Dir> mkn::kul::Dir::dirs(bool incHidden) const
    KTHROW(fsys::Exception) {
  if (!is()) KEXCEPT(fsys::Exception, "Directory : \"" + path() + "\" does not exist");
#if MKN_KUL_IS_WIN
  auto const& base = path();
#else
  auto const base = real();
#endif
  std::vector<Dir> dirs;
  for (auto const& p : fsys::dirs(path(), {.hidden = incHidden}))
    dirs.emplace_back(JOIN(base, p.filename().string()));
  return dirs;
}

inline std::vector<mkn::kul::File> mkn::kul::Dir::files(bool recursive) const
    KTHROW(fsys::Exception) {
  if (!is()) KEXCEPT(fsys::Exception, "Directory : \"" + path() + "\" does not exist");
  std::vector<File> files;
  for (auto const& p : fsys::files(path())) files.emplace_back(p.filename().string(), *this);
  if (recursive)
    for (auto const& d : dirs()) {
      auto const sub = d.files(true);
      files.insert(files.end(), sub.begin(), sub.end());
    }
  return files;
}

inline std::optional<std::string> mkn::kul::Dir::REAL_OR_NULL(std::string const& s)
    KTHROW(fsys::Exception) {
  std::error_code ec;
#if MKN_KUL_IS_WIN  // like _fullpath, the path need not exist
  auto const p = std::filesystem::absolute(s, ec).lexically_normal();
#else
  auto const p = std::filesystem::canonical(s, ec);
#endif
  if (ec) return std::nullopt;
  auto r = p.string();
#if MKN_KUL_IS_WIN
  if (r.size() && r.back() == '\\') r.pop_back();
#endif
  return r;
}

inline std::string mkn::kul::Dir::REAL(std::string const& s) KTHROW(fsys::Exception) {
  if (auto r = REAL_OR_NULL(s)) return *r;
  KEXCEPT(fsys::Exception, "Item: \"" + s + "\" does not exist");
}

inline bool mkn::kul::File::is() const {
  std::error_code ec;
  return !name().empty() && std::filesystem::is_regular_file(_d.join(_n), ec);
}

inline bool mkn::kul::File::rm() const {
  if (!is()) return false;
  std::error_code ec;
  std::filesystem::remove(_d.join(_n), ec);
  return true;
}

inline bool mkn::kul::File::mk() const { return static_cast<bool>(std::ofstream(full())); }

inline uint64_t mkn::kul::File::size() const {
  std::error_code ec;
  auto const s = std::filesystem::file_size(full(), ec);
  return ec ? 0 : s;
}

inline bool mkn::kul::File::cp(File const& f) const {
  std::error_code ec;
  std::filesystem::copy_file(_d.join(_n), f.dir().join(f.name()),
                             std::filesystem::copy_options::overwrite_existing, ec);
  return !ec;
}

inline bool mkn::kul::File::mv(File const& f) const {
  std::error_code ec;
  std::filesystem::rename(full(), f.full(), ec);
  return !ec;
}

inline bool mkn::kul::Dir::cp(Dir const& d) const {
  if (!d.is() && !d.mk()) KEXCEPT(fsys::Exception, "Directory: \"" + d.path() + "\" is not valid");
  Dir c(d.join(name()));
  c.mk();
  for (auto const& f : files()) f.cp(c);
  for (auto const& dd : dirs()) dd.cp(c);
  return 1;
}

inline std::ostream& operator<<(std::ostream& s, File const& d) { return s << d.full(); }

}  // namespace kul
}  // namespace mkn

namespace mkn {
namespace kul {
namespace os {

class PushDir {
 public:
  PushDir(std::filesystem::path const& d) {
    std::error_code ec;
    if (!std::filesystem::is_directory(d, ec))
      KEXCEPTION("PushDir directory does not exist: ", d.string());
    cwd = mkn::kul::env::CWD();
    mkn::kul::env::CWD(fsys::real(d));
  }
  template <std::same_as<mkn::kul::Dir> D>
  PushDir(D const& d) : PushDir(std::filesystem::path(d.path())) {}
  ~PushDir() { mkn::kul::env::CWD(cwd); }

 private:
  std::string cwd;
};

}  // namespace os

namespace env {

inline std::string WHERE(char const* c) {
  for (auto const& s : mkn::kul::String::SPLIT(env::GET("PATH"), mkn::kul::env::SEP())) {
    if (std::error_code ec; !std::filesystem::is_directory(s, ec)) continue;
    for (auto const& f : fsys::files(s))
      if (f.filename() == c) return fsys::real(f);
  }
  return "";
}

inline bool WHICH(char const* c) { return WHERE(c).size(); }
inline bool WHICH(std::string const& s) { return WHERE(s.c_str()).size(); }
}  // namespace env
}  // namespace kul
}  // namespace mkn

#if MKN_KUL_IS_WIN
#include "mkn/kul/os/win/os.bot.hpp"  // IWYU pragma: export
#else
#include "mkn/kul/os/nixish/os.bot.hpp"  // IWYU pragma: export
#endif

#endif /* MKN_KUL_OS_HPP_ */
