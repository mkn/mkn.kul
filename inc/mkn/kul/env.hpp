/**
Copyright (c) 2023, Philip Deegan.
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
#ifndef _MKN_KUL_ENV_HPP_
#define _MKN_KUL_ENV_HPP_

#include "mkn/kul/string.hpp"

#if KUL_IS_WIN
#include "mkn/kul/os/win/env.hpp"
#else
#include "mkn/kul/os/nixish/env.hpp"
#endif /*KUL_IS_WIN*/

namespace mkn {
namespace kul {
namespace env {

class Var {
 public:
  enum Mode { APPE = 0, PREP, REPL };

  Var(const std::string _n, const std::string _v, const Mode _m) : n(_n), v(_v), m(_m) {}
  Var(const Var &e) : n(e.n), v(e.v), m(e.m) {}
  char const *name() const { return n.c_str(); }
  char const *value() const { return v.c_str(); }
  Mode mode() const { return m; }
  const std::string toString() const {
    std::string var(value());
    mkn::kul::String::REPLACE_ALL(var, EOL(), "");
    mkn::kul::String::TRIM(var);
    const std::string ev(env::GET(name()));
    if (!ev.empty()) {
      if (mode() == Mode::PREP)
        var = var + mkn::kul::env::SEP() + ev;
      else if (mode() == Mode::APPE)
        var = ev + mkn::kul::env::SEP() + var;
    }
    return var;
  }
  Var &operator=(Var &&e) {
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

}  // namespace env
}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_ENV_HPP_ */
