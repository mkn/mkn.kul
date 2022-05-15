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
#ifndef _MKN_KUL_SCM_HPP_
#define _MKN_KUL_SCM_HPP_

#include "mkn/kul/map.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/string.hpp"

namespace mkn {
namespace kul {
namespace scm {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const *f, uint16_t const &l, std::string const &s)
      : mkn::kul::Exception(f, l, s) {}
};

class NotFoundException : public mkn::kul::Exception {
 public:
  NotFoundException(char const *f, uint16_t const &l, std::string const &s)
      : mkn::kul::Exception(f, l, s) {}
};
}  // namespace scm

class SCM {
 protected:
  SCM() {}

 public:
  virtual ~SCM() {}
  std::string type() { return typeid(*this).name(); }
  virtual std::string co(std::string const &d, std::string const &r, std::string const &v) const
      KTHROW(Exception) = 0;
  virtual void up(std::string const &d, std::string const &r, std::string const &v) const
      KTHROW(Exception) = 0;
  virtual std::string origin(std::string const &d) const = 0;
  virtual std::string localVersion(std::string const &d, std::string const &b) const = 0;
  virtual std::string remoteVersion(std::string const &url, std::string const &branch) const
      KTHROW(Exception) = 0;

  virtual bool hasChanges(std::string const &d) const = 0;
  virtual void status(std::string const &d, bool full = 1) const = 0;
  virtual void diff(std::string const &d) const = 0;

  virtual std::string defaultRemoteBranch(std::string const &repo) const = 0;
};

// review https://gist.github.com/aleksey-bykov/1273f4982c317c92d532
namespace scm {
class Git : public SCM {
 public:
  std::string defaultRemoteBranch(std::string const &repo) const override {
    mkn::kul::Process p("git");
    mkn::kul::ProcessCapture pc(p);
    p << "ls-remote"
      << "--symref" << repo << "HEAD";
    ;
    try {
      KLOG(DBG) << mkn::kul::String::LINES(pc.outs())[0];
      p.start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KEXCEPT(Exception, "SCM ERROR - Checking local branch") << p.toString();
    }
    auto ret = mkn::kul::String::SPLIT(mkn::kul::String::LINES(pc.outs())[0], "/").back();
    return ret.substr(0, ret.size() - 5);  // HEAD+tab
    // e.g. git ls-remote --symref git@github.com:user/repo HEAD
  };

  std::string branch(mkn::kul::Dir const &dr) const {
    mkn::kul::os::PushDir pushd(dr);
    mkn::kul::Process p("git");
    mkn::kul::ProcessCapture pc(p);
    p << "branch"
      << "--show-current";
    try {
      p.start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KEXCEPT(Exception, "SCM ERROR - Checking local branch");
    }
    return mkn::kul::String::LINES(pc.outs())[0];
  }

  std::string branch(std::string const &d) const { return branch(mkn::kul::Dir(d)); }

  std::string co(std::string const &d, std::string const &r, std::string const &v) const
      KTHROW(Exception) override {
    Dir dr(d, true);
    mkn::kul::Process p("git");
    p << "clone" << mkn::kul::env::GET("KUL_GIT_CO") << r;
    if (!v.empty()) p.arg("-b").arg(v);
    try {
      p.arg(d).start();
    } catch (mkn::kul::proc::ExitException const &e) {
      dr.rm();
      KEXCEPT(Exception, "SCM ERROR - Check remote dependency location / version");
    }
    return p.toString();
  }
  void up(std::string const &d, std::string const &r, std::string const &v) const
      KTHROW(Exception) override {
    if (!Dir(d).is())
      co(d, r, v);
    else {
      mkn::kul::Process p("git", d);
      p.arg("pull");
      if (!r.empty()) p.arg(r);
      if (!r.empty() && !v.empty()) p.arg(v);
      try {
        p.start();
      } catch (mkn::kul::proc::ExitException const &e) {
        KEXCEPT(Exception, "SCM ERROR - Check remote dependency location / version");
      }
    }
  }
  std::string origin(std::string const &d) const override {
    mkn::kul::Process p("git", d);
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("remote").arg("-v").start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    if (pc.outs().empty())
      KEXCEPT(Exception, "SCM ERROR: Directory may not be git repository : " + d);
    std::vector<std::string> lines;
    mkn::kul::String::LINES(pc.outs(), lines);
    for (auto &line : lines) mkn::kul::String::REPLACE_ALL(line, "\t", " ");
    for (auto &line : lines) mkn::kul::String::REPLACE_ALL(line, "  ", " ");
    if (lines.size()) return mkn::kul::String::SPLIT(lines[0], ' ')[1];
    KEXCEPT(Exception, "SCM ERROR - Check remote dependency location / version");
  }
  std::string localVersion(std::string const &d, std::string const &b) const override {
    mkn::kul::Process p("git", d);
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("rev-parse").arg(b.empty() ? "HEAD" : b).start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    if (pc.outs().empty())
      KEXCEPT(Exception, "SCM ERROR: Directory may not be git repository : " + d);
    return mkn::kul::String::LINES(pc.outs())[0];
  }

  std::string remoteVersion(std::string const &url, std::string const &b) const
      KTHROW(Exception) override {
    mkn::kul::Process p("git");
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("ls-remote").arg(url).arg(b).start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    if (pc.outs().empty())
      KEXCEPT(Exception, "SCM ERROR URL OR BRANCH MAY NOT EXIST: " + url + " / " + b);
    std::string s(mkn::kul::String::LINES(pc.outs())[0]);
    mkn::kul::String::TRIM(s);
    return s.substr(0, s.find('\t'));
  }

  bool hasChanges(std::string const &d) const override {
    mkn::kul::Process p("git", d);
    mkn::kul::ProcessCapture pc(p);
    try {
      p.arg("status").arg("-sb").start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KERR << pc.errs();
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
    return mkn::kul::String::LINES(pc.outs()).size() > 1;
  }
  void status(std::string const &d, bool full = 1) const override {
    mkn::kul::Process p("git", d);
    try {
      p << "status";
      if (!full) p << "--short";
      p.start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
  }
  void diff(std::string const &d) const override {
    mkn::kul::Process p("git", d);
    try {
      p.arg("diff").start();
    } catch (mkn::kul::proc::ExitException const &e) {
      KEXCEPT(Exception, "SCM ERROR " + std::string(e.what()));
    }
  }
};

/*
class Svn : public SCM {
 public:
  std::string co(std::string const&d, std::string const&r, const
std::string &v) const KTHROW(Exception) { Dir dr(d, true); mkn::kul::Process p("svn",
d); p.arg("checkout").arg(mkn::kul::env::GET("KUL_SVN_CO")); if (v.empty()) p.arg(r);
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
#endif /* _MKN_KUL_SCM_HPP_ */
