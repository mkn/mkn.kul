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
#ifndef _MKN_KUL_CLI_HPP_
#define _MKN_KUL_CLI_HPP_

#include <iostream>
#include <string>
#include <vector>

#include "mkn/kul/env.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/map.hpp"
#include "mkn/kul/os/cli.hpp"
#include "mkn/kul/serial.hpp"
#include "mkn/kul/string.hpp"

namespace mkn {
namespace kul {
namespace cli {

inline const std::string receive(std::string const &t = "") {
  if (!t.empty()) std::cout << t << std::endl;
  std::string s;
  std::getline(std::cin, s);
  return s;
}

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const *f, uint16_t const &l, std::string const &s)
      : mkn::kul::Exception(f, l, s) {}
};

class ArgNotFoundException : public Exception {
 public:
  ArgNotFoundException(char const *f, uint16_t const &l, std::string const &s)
      : Exception(f, l, s) {}
};

class Cmd {
 private:
  char const *c;

 public:
  Cmd(char const *_c) : c(_c) {}
  char const *command() const { return c; }
};

// deprecated : use mkn::kul::env::Var::Mode
enum EnvVarMode { APPE = 0, PREP, REPL };

// deprecated : use mkn::kul::env::Var
class EnvVar {
 public:
  EnvVar(const std::string _n, const std::string _v, const EnvVarMode _m) : n(_n), v(_v), m(_m) {}
  EnvVar(const EnvVar &e) : n(e.n), v(e.v), m(e.m) {}
  char const *name() const { return n.c_str(); }
  char const *value() const { return v.c_str(); }
  EnvVarMode mode() const { return m; }
  const std::string toString() const {
    std::string var(value());
    mkn::kul::String::REPLACE_ALL(var, mkn::kul::env::EOL(), "");
    mkn::kul::String::TRIM(var);
    const std::string ev(env::GET(name()));
    if (!ev.empty()) {
      if (mode() == EnvVarMode::PREP)
        var = var + mkn::kul::env::SEP() + ev;
      else if (mode() == EnvVarMode::APPE)
        var = ev + mkn::kul::env::SEP() + var;
    }
    return var;
  }
  EnvVar &operator=(EnvVar &&e) {
    std::swap(m, e.m);
    std::swap(n, e.n);
    std::swap(v, e.v);
    return *this;
  }

 private:
  std::string n;
  std::string v;
  EnvVarMode m;
};

enum ArgType { FLAG = 0, STRING, MAYBE };

class Arg : public Cmd {
  static constexpr char INVALID_CHAR = ' ';

 public:
  Arg(const char _d, char const *dd, ArgType _t, bool m = false) : Cmd(dd), man(m), d(_d), t(_t) {}
  Arg(const char _d, char const *dd, bool m = false) : Cmd(dd), man(m), d(_d) {}

  Arg(char const *dd, ArgType _t, bool m = false) : Cmd(dd), man(m), t(_t) {}
  Arg(char const *dd, bool m = false) : Cmd(dd), man(m) {}

  bool mandatory() const { return man; }
  char dash() const { return d; }
  char const *dashdash() const { return command(); }
  const ArgType &type() const { return t; }

 private:
  bool man;
  const char d = ' ';
  ArgType t = ArgType::FLAG;
};

class Args {
 public:
  Args() {}
  Args(const std::vector<Cmd> &_cmds, const std::vector<Arg> &_args) : cmds(_cmds), args(_args) {}

  void arg(const Arg &a) { args.push_back(a); }
  void cmd(const Cmd &c) { cmds.push_back(c); }
  const Cmd &commands(char const *c) const {
    for (const Cmd &cmd : cmds)
      if (strcmp(cmd.command(), c) == 0) return cmd;
    KEXCEPT(ArgNotFoundException, "No command " + std::string(c) + " found");
  }
  const Arg &dashes(const char c) const {
    for (const Arg &a : arguments())
      if (a.dash() == c) return a;
    KEXCEPT(ArgNotFoundException, "No argument " + std::string(1, c) + " found");
  }
  const Arg &doubleDashes(char const *c) const {
    for (const Arg &a : arguments())
      if (strcmp(a.command(), c) == 0) return a;
    KEXCEPT(ArgNotFoundException, "No argument " + std::string(c) + " found");
  }
  const std::vector<Cmd> &commands() const { return cmds; }
  const std::vector<Arg> &arguments() const { return args; }
  std::string const &get(std::string const &s) const {
    if (has(s)) return vals.at(s);
    KEXCEPT(ArgNotFoundException, "No value " + s + " found");
  }
  bool empty() const { return vals.size() == 0; }
  bool has(std::string const &s) const { return vals.count(s); }
  size_t size() const { return vals.size(); }
  bool erase(std::string const &key) { return vals.erase(key); }
  void process(uint16_t const &argc, char *argv[], uint16_t first = 1)
      KTHROW(ArgNotFoundException) {
    for (const Arg &a1 : arguments())
      for (const Arg &a2 : arguments()) {
        if (&a1 == &a2) continue;
        if ((a1.dash() != ' ') &&
            (a1.dash() == a2.dash() || strcmp(a1.dashdash(), a2.dashdash()) == 0))
          KEXCEPT(Exception, "Duplicate argument detected");
      }
    for (const Cmd &c1 : commands())
      for (const Cmd &c2 : commands()) {
        if (&c1 == &c2) continue;
        if (strcmp(c1.command(), c2.command()) == 0)
          KEXCEPT(Exception, "Duplicate argument detected");
      }

    Arg *arg = 0;
    uint16_t valExpected = 0;
    std::string valExpectedFor, c, t;

    for (size_t j = first; j < argc; j++) {
      c = argv[j];
      t = c;

      if (c.compare("---") == 0) KEXCEPT(Exception, "Illegal argument ---");
      if (c.compare("--") == 0) KEXCEPT(Exception, "Illegal argument --");
      if (c.compare("-") == 0) KEXCEPT(Exception, "Illegal argument -");
      if (valExpected == 1 || (valExpected == 2 && c.find("-") != 0)) {
        valExpected = 0;
        vals[arg->dashdash()] = c;
        arg = 0;
        continue;
      } else if (valExpected == 2)
        valExpected = 0;

      if (c.find("--") == 0) {
        c = c.substr(c.find("--") + 2);
        valExpectedFor = c;
        if (c.find("=") == std::string::npos) {
          arg = const_cast<Arg *>(&doubleDashes(c.c_str()));
          valExpected = arg->type();
          if (!valExpected) vals[arg->dashdash()] = "";
          continue;
        }
        valExpectedFor = c.substr(0, c.find("="));
        arg = const_cast<Arg *>(&doubleDashes(valExpectedFor.c_str()));
        valExpected = arg->type();
        if (valExpected == 0)
          KEXCEPT(Exception, "Found = when no value is expected for arg " + valExpectedFor);
        c = c.substr(c.find("=") + 1);
        vals[arg->dashdash()] = c;
      } else if (c.find("-") == 0) {
        valExpectedFor = c;
        c = c.substr(c.find("-") + 1);
        if (c.find("=") != std::string::npos) {
          if (c.substr(0, c.find("=")).size() > 1)
            KEXCEPT(Exception, "Cannot mix flag and non-flag arguments");
          arg = const_cast<Arg *>(&dashes(c.at(0)));
          vals[arg->dashdash()] = c.substr(c.find("=") + 1);
          valExpected = 0;
        } else if (c.length() > 1) {
          std::string a = c;
          for (size_t i = 0; i < c.length(); i++) {
            arg = const_cast<Arg *>(&dashes(a.at(0)));
            if (i + 1 == c.length())
              valExpected = arg->type();
            else if (arg->type() == ArgType::STRING)
              KEXCEPT(Exception, "Cannot mix flag and non-flag arguments");
            vals[arg->dashdash()] = "";
            if (a.length() > 1) a = a.substr(1);
          }
        } else {
          arg = const_cast<Arg *>(&dashes(c.at(0)));
          valExpected = arg->type();
          vals[arg->dashdash()] = "";
        }
        continue;
      }
      if (valExpected == 1) {
        valExpected = 0;
        continue;
      }
      commands(c.c_str());
      vals[c] = "";
    }
    if (valExpected == 1)
      KEXCEPT(Exception, "Value expected for argument: \"" + valExpectedFor + "\"");
    for (const Arg &a : args)
      if (a.mandatory()) get(a.dashdash());
  }
#if defined(_MKN_WITH_IO_CEREAL_)
  friend class cereal::access;
#endif  //  _MKN_WITH_IO_CEREAL_
 private:
  std::vector<Cmd> cmds;
  std::vector<Arg> args;
  hash::map::S2S vals;

 public:
#include "mkn/kul/serial/cli.arg.end.hpp"
};

inline void asArgs(std::string const &cmd, std::vector<std::string> &args);

inline std::vector<std::string> asArgs(std::string const &cmd) {
  std::vector<std::string> args;
  asArgs(cmd, args);
  return args;
}

}  // namespace cli
}  // namespace kul
}  // namespace mkn

#ifndef _MKN_KUL_COMPILED_LIB_
#include "mkn/kul/src/cli/asArgs.ipp"
#endif

#include "mkn/kul/serial/cli.bottom.hpp"

#endif /* _MKN_KUL_CLI_HPP_ */
