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
#ifndef _MKN_KUL_BON_HPP_
#define _MKN_KUL_BON_HPP_

#include "mkn/kul/cli.hpp"
#include "mkn/kul/yaml.hpp"

#include <string>
#include <cstdint>
#include <functional>

namespace mkn {
namespace kul {
namespace bon {  // Better Object Notation (basically json but without quotes)
class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, std::size_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

enum class TYPE { OBJ = 0, STR, INT };

struct BonParsableNode {
  BonParsableNode(BonParsableNode* const _p = nullptr) : p{_p} {}

  BonParsableNode& emplace_back() { return nodes.emplace_back(this); }

  BonParsableNode* const p = nullptr;
  std::stringstream ss{};
  std::vector<BonParsableNode> nodes;
};

class ob {
 public:
  static ob BUILD(BonParsableNode const& root) {
    ob o;
    BUILD(root, o);
    return o;
  }

  YAML::Node to_yaml() const {
    YAML::Node node;
    for (auto const& oc : c) recurse(node, oc);
    YAML::Emitter out;
    out << node;
    return node;
  }

  std::string v, a;
  std::vector<ob> c;
  ob* p = nullptr;

 private:
  static void BUILD(BonParsableNode const& node, ob& o) {
    KLOG(INF) << node.ss.str() << " " << o.p;

    if (!node.nodes.size())
      o.a = node.ss.str();
    else
      o.v = node.ss.str();

    mkn::kul::String::TRIM(o.a);
    mkn::kul::String::TRIM(o.v);
    if (o.v.back() == ':')) {
        o.v = o.v.substr(0, o.v.size() - 1);
        mkn::kul::String::TRIM(o.v);
      }

    for (auto const& n : node.nodes) {
      o.c.emplace_back().p = &o;
      BUILD(n, o.c.back());
    }
  }

  void recurse(YAML::Node& n, ob const& o) const {
    if (o.a.empty() && o.v.empty() && o.c.empty()) return;

    if (o.c.size()) {
      auto n1 = n[o.v];
      for (auto const& oc : o.c) recurse(n1, oc);
      return;  // children or values
    }

    auto vals = mkn::kul::String::ESC_SPLIT(o.a, ',');
    if (o.a.find(":") == std::string::npos) {  // list

      for (std::size_t i = 0; i < vals.size(); ++i) {
        auto p = mkn::kul::String::ESC_SPLIT(vals[i], ':');
        KLOG(INF) << p[0];
        if (p.size() == 1) {
          mkn::kul::String::TRIM(p);
          n.push_back(p[0]);
        } else
          KEXCEPTION("FAIL");
      }
    } else  // map
      for (auto const& v : vals) {
        auto p = mkn::kul::String::ESC_SPLIT(v, ':');
        if (p.size() == 2) {
          mkn::kul::String::TRIM(p);
          n[p[0]] = p[1];
        } else
          KEXCEPTION("FAIL");
      }
  };
};

YAML::Node from(std::string const& s) {
  if (s[0] != '{' || s[s.size() - 1] != '}') KEXCEPTION("FAIL");

  std::string r = s;
  std::vector<ob> obs;
  BonParsableNode root;
  std::vector<YAML::Node> nodes;
  std::size_t open = 0;
  BonParsableNode* curr = &root;

  auto subtree = [&]() {
    curr = &curr->emplace_back();
    ++open;
    return true;
  };

  auto c_check = [&]() {
    --open;

    if (open) {
      curr = curr->p;
      return true;
    }
    obs.emplace_back(ob::BUILD(root));
    return true;
  };

  auto next = [&]() {
    if (curr->p->nodes.size()) curr = &curr->p->emplace_back();
    return curr->p->nodes.size() > 0;
  };

  for (size_t i = 0; i < r.size(); i++) {
    if (r[i] == '}' && c_check()) continue;
    if (r[i] == '{' && subtree()) continue;
    if (r[i] == ',' && next()) continue;
    if (curr->nodes.size())
      curr->nodes.back().ss << r[i];
    else
      curr->ss << r[i];
  }

  if (open) c_check();
  for (auto const& o : obs) nodes.emplace_back(o.to_yaml());
  auto const ret = YAML::Node(nodes);
  YAML::Emitter out;
  out << ret;
  KLOG(INF) << out.c_str();
  return ret;
}

}  // namespace bon
}  // namespace kul
}  // namespace mkn

#endif  // _MKN_KUL_BON_HPP_
