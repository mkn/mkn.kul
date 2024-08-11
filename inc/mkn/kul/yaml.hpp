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
/*
REQUIRES in mkn.yaml
    dep:
      - name: parse.yaml

or via cli
    mkn build -w parse.yaml
*/

#ifndef _MKN_KUL_YAML_HPP_
#define _MKN_KUL_YAML_HPP_

#include "mkn/kul/io.hpp"
#include "mkn/kul/dbg.hpp"
#include "mkn/kul/map.hpp"

#include "yaml-cpp/yaml.h"

#include <string>
#include <vector>
#include <cstdint>

namespace mkn {
namespace kul {
namespace yaml {

class Validator;
class NodeValidator;

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, std::uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

enum NodeType { NON = 0, STRING, LIST, MAP };

class NodeValidator {
 public:
  NodeValidator(std::string const& n, bool m = 0) : m(m), typ(STRING), nam(n) {}
  NodeValidator(std::string const& n, std::vector<NodeValidator> const& c, bool m,
                NodeType const& typ)
      : m(m), typ(typ), nam(n), kids(c) {}
  std::vector<NodeValidator> const& children() const { return this->kids; }
  bool mandatory() const { return this->m; }
  std::string const& name() const { return this->nam; }
  NodeType const& type() const { return this->typ; }

 private:
  bool const m;
  NodeType const typ;
  std::string const nam;
  std::vector<NodeValidator> kids;
};

class Item {
  friend class Validator;

 protected:
  YAML::Node r;  // root

  Item() {}
  Item(YAML::Node const& r) : r(r) {}

 public:
  virtual ~Item() {}
  YAML::Node const& root() const { return r; }

  static void VALIDATE(YAML::Node const& n, std::vector<NodeValidator> const& nvs)
      KTHROW(Exception) {
    mkn::kul::hash::set::String keys;
    for (auto const& nv : nvs)
      if (nv.name() == "*") return;

    for (YAML::const_iterator it = n.begin(); it != n.end(); ++it) {
      std::string const& key(it->first.as<std::string>());
      if (keys.count(key)) KEXCEPTION("Duplicate key detected: " + key);
      keys.insert(key);
      bool f = 0;
      for (auto const& nv : nvs) {
        if (nv.name() != key) continue;
        f = 1;
        if (nv.type() == 1 && it->second.Type() != 2) KEXCEPTION("String expected: " + nv.name());
        if (nv.type() == 2 && it->second.Type() != 3) KEXCEPTION("List expected: " + nv.name());
        if (nv.type() == 3 && it->second.Type() != 4) KEXCEPTION("Map expected: " + nv.name());
        if (nv.type() == 2)
          for (size_t i = 0; i < it->second.size(); i++) VALIDATE(it->second[i], nv.children());
        if (nv.type() == 3) VALIDATE(it->second, nv.children());
      }
      if (!f) KEXCEPTION("Unexpected key: " + key);
    }
    for (auto const& nv : nvs) {
      if (nv.mandatory() && !keys.count(nv.name())) KEXCEPTION("Key mandatory: : " + nv.name());
    }
  }
};

class Validator {
 public:
  Validator(std::vector<NodeValidator> const& kids) : kids(kids) {}
  std::vector<NodeValidator> const& children() const { return this->kids; }
  void validate(YAML::Node const& n) { Item::VALIDATE(n, children()); }

 private:
  const std::vector<NodeValidator> kids;
};

class String : public Item {
 public:
  String(std::string const& s) KTHROW(Exception) : s(s) {
    try {
      r = YAML::Load(s);
    } catch (std::exception const& e) {
      KEXCEPTION("YAML failed to parse\nError/String: " + std::string(e.what())) << "\n" << s;
    }
  }
  YAML::Node const& validate(Validator const&& v) KTHROW(Exception) {
    Item::VALIDATE(root(), v.children());
    return r;
  }

 private:
  std::string const s;
};

class File : public Item {
 public:
  template <class T>
  static T CREATE(std::string const& f) KTHROW(Exception) {
    T file(f);
    try {
      Item::VALIDATE(file.root(), file.validator().children());
    } catch (mkn::kul::yaml::Exception const& e) {
      KEXCEPTION("YAML error encountered in file: " + f);
    }
    return file;
  }
  virtual ~File() {}
  std::string const& file() const { return f; }
  virtual Validator validator() const = 0;

 protected:
  void reload() KTHROW(Exception) {
    try {
      r = YAML::LoadFile(f);
    } catch (std::exception const& e) {
      KEXCEPTION("YAML failed to parse\nFile: " + f);
    }
  }
  File(File const& f) : Item(f.r), f(f.f) {}
  File(mkn::kul::File const& f) KTHROW(Exception) : f(f.real()) { reload(); }
  File(std::string const& f) KTHROW(Exception) : f(f) { reload(); }

 private:
  std::string const f;  // file
};
}  // namespace yaml
}  // namespace kul
}  // namespace mkn
#endif /* _MKN_KUL_YAML_HPP_ */
