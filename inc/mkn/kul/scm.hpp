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
#ifndef MKN_KUL_SCM_HPP_
#define MKN_KUL_SCM_HPP_

#include "mkn/kul/os.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/string.hpp"
#include "mkn/kul/env.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <typeinfo>
#include <vector>

namespace mkn {
namespace kul {
namespace scm {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class NotFoundException : public mkn::kul::Exception {
 public:
  NotFoundException(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};
}  // namespace scm

class SCM {
 protected:
  SCM() {}

 public:
  virtual ~SCM() {}
  std::string type() { return typeid(*this).name(); }
  virtual std::string co(std::string const& d, std::string const& r, std::string const& v) const
      KTHROW(Exception) = 0;
  virtual void up(std::string const& d, std::string const& r, std::string const& v) const
      KTHROW(Exception) = 0;
  virtual std::string origin(std::string const& d) const = 0;
  virtual std::string localVersion(std::string const& d, std::string const& b) const = 0;
  virtual std::string remoteVersion(std::string const& url, std::string const& branch) const
      KTHROW(Exception) = 0;

  virtual bool hasChanges(std::string const& d) const = 0;
  virtual void status(std::string const& d, bool full = 1) const = 0;
  virtual void diff(std::string const& d) const = 0;

  virtual std::string defaultRemoteBranch(std::string const& repo) const = 0;
};

namespace scm {

class Git : public SCM {
 public:
  std::string defaultRemoteBranch(std::string const& repo) const override {
    mkn::kul::Process p("git");
    mkn::kul::ProcessCapture pc(p);
    p << "ls-remote"
      << "--symref" << repo << "HEAD";
    try {
      p.start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KEXCEPT(Exception, "SCM ERROR - Checking local branch", p.toString());
    }
    auto const lines = mkn::kul::String::LINES(pc.outs());
    if (lines.empty()) return {};
    // "ref: refs/heads/master\tHEAD" -> "master"
    auto ret = mkn::kul::String::SPLIT(lines[0], "/").back();
    auto const tab = ret.find('\t');
    return tab != std::string::npos ? ret.substr(0, tab) : ret;
    // e.g. git ls-remote --symref git@github.com:user/repo HEAD
  }

  std::string branch(std::filesystem::path const& d) const {
    mkn::kul::os::PushDir pushd(d.string());
    mkn::kul::Process p("git");
    mkn::kul::ProcessCapture pc(p);
    p << "branch"
      << "--show-current";
    try {
      p.start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KEXCEPT(Exception, "SCM ERROR - Checking local branch");
    }
    return mkn::kul::String::LINES(pc.outs())[0];
  }

  template <std::same_as<mkn::kul::Dir> D>
  MKN_KUL_DEPRECATED("use std::filesystem::path")
  std::string branch(D const& d) const {
    return branch(std::filesystem::path(d.path()));
  }

  std::string co(std::string const& d, std::string const& r, std::string const& v) const
      KTHROW(Exception) override {
    std::error_code ec;
    std::filesystem::create_directories(d, ec);
    if (!std::filesystem::is_directory(d, ec))
      KEXCEPT(Exception, "SCM ERROR - could not create directory: " + d);
    mkn::kul::Process p("git");
    std::string const defaults = "--depth 10 --recursive --shallow-submodules";
    p << "clone" << mkn::kul::env::GET("MKN_KUL_GIT_CO", defaults) << r;
    if (!v.empty()) p.arg("-b").arg(v);
    try {
      p.arg(d).start();
    } catch (mkn::kul::proc::ExitException const& e) {
      std::filesystem::remove_all(d, ec);
      KEXCEPT(Exception, "SCM ERROR - Check remote dependency location / version");
    }
    return p.toString();
  }
  // a branch checkout is moved to the remote's branch: fast-forwarded, or reset when the branch was
  //  rebuilt (a rolling one, e.g. latest) and the clone has nothing of its own, no local commits or
  //  changes, else an error and nothing touched; a tag or commit checkout is pinned, never moved
  void up(std::string const& d, std::string const& r, std::string const& v) const
      KTHROW(Exception) override {
    if (std::error_code ec; !std::filesystem::is_directory(d, ec)) {
      co(d, r, v);
      return;
    }
    auto const git = [&](std::vector<std::string> const& args, bool const must = true) {
      mkn::kul::Process p("git", d);
      mkn::kul::ProcessCapture pc(p);
      for (auto const& a : args) p.arg(a);
      try {
        p.start();
      } catch (mkn::kul::proc::ExitException const& e) {
        if (must) KEXCEPT(Exception, "SCM ERROR - git " + args[0] + " in " + d + ": " + pc.errs());
        return std::string{"\n"};  // never a commit
      }
      auto out = pc.outs();
      while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
      return out;
    };
    auto const b = git({"symbolic-ref", "-q", "--short", "HEAD"}, false);
    if (b == "\n") return;  // detached: a tag or a commit
    auto const tracking = "refs/remotes/origin/" + b;
    auto const before = git({"rev-parse", "-q", "--verify", tracking}, false);
    git({"fetch", r.empty() ? "origin" : r, "+refs/heads/" + b + ":" + tracking});
    if (git({"merge", "--ff-only", tracking}, false) != "\n") return;
    if (!git({"status", "--porcelain", "--untracked-files=no"}).empty())
      KEXCEPT(Exception, "SCM ERROR - " + d + " has local changes, " + b + " was not updated");
    if (git({"rev-parse", "HEAD"}) != before)
      KEXCEPT(Exception, "SCM ERROR - " + d + " has local commits, " + b + " was not updated");
    git({"reset", "--hard", tracking});
  }
  std::string origin(std::string const& d) const override {
    mkn::kul::Process p("git", d);
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("remote").arg("-v").start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    if (pc.outs().empty())
      KEXCEPT(Exception, "SCM ERROR: Directory may not be git repository : " + d);
    std::vector<std::string> lines;
    mkn::kul::String::LINES(pc.outs(), lines);
    for (auto& line : lines) mkn::kul::String::REPLACE_ALL(line, "\t", " ");
    for (auto& line : lines) mkn::kul::String::REPLACE_ALL(line, "  ", " ");
    if (lines.size()) return mkn::kul::String::SPLIT(lines[0], ' ')[1];
    KEXCEPT(Exception, "SCM ERROR - Check remote dependency location / version");
  }
  std::string localVersion(std::string const& d, std::string const& b) const override {
    mkn::kul::Process p("git", d);
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("rev-parse").arg(b.empty() ? "HEAD" : b).start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    if (pc.outs().empty())
      KEXCEPT(Exception, "SCM ERROR: Directory may not be git repository : " + d);
    return mkn::kul::String::LINES(pc.outs())[0];
  }

  std::string remoteVersion(std::string const& url, std::string const& b) const
      KTHROW(Exception) override {
    mkn::kul::Process p("git");
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("ls-remote").arg(url).arg(b).start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    if (pc.outs().empty())
      KEXCEPT(Exception, "SCM ERROR URL OR BRANCH MAY NOT EXIST: " + url + " / " + b);
    std::string s(mkn::kul::String::LINES(pc.outs())[0]);
    mkn::kul::String::TRIM(s);
    return s.substr(0, s.find('\t'));
  }

  bool hasChanges(std::string const& d) const override {
    mkn::kul::Process p("git", d);
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("status").arg("-sb").start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    return mkn::kul::String::LINES(pc.outs()).size() > 1;
  }
  void status(std::string const& d, bool full = 1) const override {
    mkn::kul::Process p("git", d);
    try {
      p << "status";
      if (!full) p << "--short";
      p.start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
  }
  void diff(std::string const& d) const override {
    mkn::kul::Process p("git", d);
    try {
      p.arg("diff").start();
    } catch (mkn::kul::proc::ExitException const& e) {
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
  }
};

/*
class Svn : public SCM {
 public:
  std::string co(std::string const&d, std::string const&r, const
std::string &v) const KTHROW(Exception) { Dir dr(d, true); mkn::kul::Process p("svn",
d); p.arg("checkout").arg(mkn::kul::env::GET("MKN_KUL_SVN_CO")); if (v.empty()) p.arg(r);
    else
      p.arg(r + "/" + v);
    try {
      p.arg(".").start();
    } catch (mkn::kul::proc::ExitException const& e) {
      dr.rm();
      KEXCEPT(Exception, "SCM ERROR - Check remote dependency location /
version");
    }
    return p.toString();
  }
  void up(std::string const&d, std::string const&r, std::string const&v)
const KTHROW(Exception) { if (!Dir(d).is()) co(d, r, v); else KEXCEPT(Exception,
"SCM ERROR - SVN NOT IMPLEMENTED");
  }
  // svn info . | lines | split | ifeq "Repository Root" go |
  // http://svnbook.red-bean.com/en/1.6/svn.ref.svn.c.info.html
  std::string origin(std::string const&d) const {
    KEXCEPT(Exception, "SCM ERROR - SVN NOT IMPLEMENTED");
  }
  std::string localVersion(std::string const&d, std::string const&b)
const { KEXCEPT(Exception, "SCM ERROR - SVN NOT IMPLEMENTED");
  }
  std::string remoteVersion(std::string const&url, std::string const&b)
const KTHROW(Exception) { KEXCEPT(Exception, "SCM ERROR - SVN NOT IMPLEMENTED");
  }

  bool hasChanges(std::string const&d) const {
    KEXCEPT(Exception, "SCM ERROR - SVN NOT IMPLEMENTED");
  }
  void status(std::string const&d, bool full) const { KEXCEPT(Exception, "SCM
ERROR - SVN NOT IMPLEMENTED"); } void diff(std::string const&d) const {
KEXCEPT(Exception, "SCM ERROR - SVN NOT IMPLEMENTED"); }
};
*/

}  // namespace scm
}  // namespace kul
}  // namespace mkn
#endif /* MKN_KUL_SCM_HPP_ */
