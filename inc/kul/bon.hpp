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
#ifndef _KUL_BON_HPP_
#define _KUL_BON_HPP_

#include "kul/cli.hpp"
#include "kul/yaml.hpp"
// #include <vector>


namespace kul {
namespace bon {
class Exception : public kul::Exception {
 public:
  Exception(const char *f, const size_t &l, const std::string &s) : kul::Exception(f, l, s) {}
};

enum class TYPE { OBJ = 0, STR, INT };

class ob{
  private:
    static void BUILD(std::stringstream *tree, size_t size, ob &o){
      o.v = tree[0].str();
      if(size == 2) o.a = tree[1].str();
      else{
        o.c.emplace_back();
        BUILD(tree, size - 1, o.c.back());
      }
    }
    static ob BUILD(std::stringstream *tree, size_t size){
      ob o;
      BUILD(tree, size, o);
      return o;
    }
  public:
    std::string v, a;
    std::vector<ob> c;
    static ob BUILD(std::vector<std::stringstream> &tree){ return BUILD(tree.data(), tree.size()); }
    std::string to_string() const {
      std::stringstream ss;
      ss << v;
      std::function<void(const ob &)> recurse = [&](const ob &o) {
        if(o.c.empty()) ss << " : " << o.a;
        for(const auto & oc : o.c) recurse(oc);
      };
      recurse(*this);
      return ss.str();
    }
    YAML::Node to_yaml() const {
      YAML::Node node;
      YAML::Node *ptr = &node;
      std::vector<std::string> keys;
      std::function<YAML::Node(YAML::Node, size_t)>
       get_with_keys_for_node = [&](YAML::Node n, size_t i){
        if(i < keys.size())
          return get_with_keys_for_node(n[keys[i]], i+1);
        return n;
      };
      auto get_with_keys = [&](){
        return get_with_keys_for_node(node[keys.back()], 1);
      };
      const ob *o = this;

      std::function<void(const ob &)> recurse = [&](const ob &o) {
        keys.emplace_back(o.v);
        if(o.c.empty()) {
          auto val = get_with_keys();
          auto vals = kul::String::ESC_SPLIT(o.a, ',');
          if(vals.empty()) val = o.a;
          for(const auto & v : vals){
            auto p = kul::String::ESC_SPLIT(v, ':');
            if(p.size() != 2) KEXIT(1, "no");
            kul::String::TRIM(p);
            val[p[0]] = p[1];
          }
        }
        for(const auto & oc : o.c) recurse(oc);
      };
      recurse(*this);
      YAML::Emitter out;
      out << node;
      return node;
    }
};

YAML::Node from(std::string s) {
  if(s[0] != '{' || s[s.size() - 1] != '}') KEXCEPTION("FAIL");
  std::string r = s.substr(1, s.size() - 2);
  bool l1 = 0, r1 = 0;
  std::vector<ob> obs;
  std::vector<std::stringstream> tree(1);
  size_t pos = 0, tri = 0;
  auto subtree = [&](){
    tree.emplace_back();
    tri++;
    return true;
  };
  auto c_check = [&](){
    obs.emplace_back(ob::BUILD(tree));
    tree.clear(); tri = -1;
    subtree();
    return true;
  };
  for(size_t i = 0; i < r.size() - 1; i++){
    if(r[i] == '}' && c_check()) continue;
    if(r[i] == '{' && subtree()) continue;
    tree[tri] << r[i];
  }
  c_check();
  std::vector<YAML::Node> nodes;
  for(const auto &o : obs) nodes.emplace_back(o.to_yaml());
  return YAML::Node(nodes);
}


}
}

#endif  // _KUL_BON_HPP_