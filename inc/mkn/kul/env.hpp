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
#ifndef MKN_KUL_ENV_HPP_
#define MKN_KUL_ENV_HPP_

#include "mkn/kul/defs.hpp"
#include "mkn/kul/string.hpp"

#include <string>
#include <sstream>
#include <utility>
#include <vector>

#if MKN_KUL_IS_WIN
#include "mkn/kul/os/win/env.hpp"
#else
#include "mkn/kul/os/nixish/env.hpp"
#endif /*MKN_KUL_IS_WIN*/

namespace mkn {
namespace kul {
namespace env {

template <typename T>
auto GET_AS(std::string const& s, T const& def) {
  if (EXISTS(s.c_str())) {
    T t;
    std::stringstream ss(GET(s.c_str()));
    if (ss.fail()) KEXCEPTION("mkn::kul::env::GET_AS failed");
    ss >> t;
    return t;
  }
  return def;
}

class Var {
 public:
  enum Mode { APPE = 0, PREP, REPL };

  Var(std::string const _n, std::string const _v, Mode const _m) : n(_n), v(_v), m(_m) {}
  Var(Var const& e) : n(e.n), v(e.v), m(e.m) {}
  char const* name() const { return n.c_str(); }
  char const* value() const { return v.c_str(); }
  Mode mode() const { return m; }
  std::string const toString() const {
    std::string var(value());
    mkn::kul::String::REPLACE_ALL(var, EOL(), "");
    mkn::kul::String::TRIM(var);
    std::string const ev(env::GET(name()));
    if (!ev.empty()) {
      if (mode() == Mode::PREP)
        var = var + mkn::kul::env::SEP() + ev;
      else if (mode() == Mode::APPE)
        var = ev + mkn::kul::env::SEP() + var;
    }
    return var;
  }
  Var& operator=(Var&& e) {
    std::swap(m, e.m);
    std::swap(n, e.n);
    std::swap(v, e.v);
    return *this;
  }

 private:
  std::string n;
  std::string v;
  Mode m;
};

// RAII in-process env var override. Saves whatever each var held (or that it
// was unset) on construction, restores that exact state on destruction - for
// mutating the current process's env (e.g. before dlopen/LoadLibrary), as
// opposed to mkn::kul::Process::var(), which only affects a spawned child.
class PushEnv {
  struct Entry {
    std::string name, old;
    bool had = false;
  };

 public:
  // set name to value
  PushEnv(std::string name, std::string const& value) { push(std::move(name), value.c_str()); }
  // set var per its own mode (PREP/APPE/REPL against whatever it currently holds)
  explicit PushEnv(Var const& var) { push(var.name(), var.toString().c_str()); }
  explicit PushEnv(std::vector<Var> const& vars) {
    for (auto const& var : vars) push(var.name(), var.toString().c_str());
  }
  // set each name to its paired value, e.g. Runner::RUN's envies vector
  explicit PushEnv(std::vector<std::pair<std::string, std::string>> const& vars) {
    for (auto const& name_value : vars) push(name_value.first, name_value.second.c_str());
  }
  // temporarily unset a single var / a batch of vars
  explicit PushEnv(std::string name) { push(std::move(name), nullptr); }
  explicit PushEnv(std::vector<std::string> const& names) {
    for (auto const& name : names) push(name, nullptr);
  }

  PushEnv(PushEnv const&) = delete;
  PushEnv& operator=(PushEnv const&) = delete;
  PushEnv& operator=(PushEnv&&) = delete;

  PushEnv(PushEnv&& o) noexcept : entries(std::move(o.entries)) { o.active = false; }

  ~PushEnv() {
    if (!active) return;
    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
      SET(it->name.c_str(), it->had ? it->old.c_str() : nullptr);
  }

 private:
  void push(std::string name, char const* val) {
    Entry e;
    e.had = EXISTS(name.c_str());
    if (e.had) e.old = GET(name.c_str());
    e.name = std::move(name);
    SET(e.name.c_str(), val);
    entries.push_back(std::move(e));
  }

  std::vector<Entry> entries;
  bool active = true;
};

}  // namespace env
}  // namespace kul
}  // namespace mkn

#endif /* MKN_KUL_ENV_HPP_ */
