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
/*
REQUIRES
    dep:
      - name: parse.yaml
        version: master
        scm: http://github.com/mkn/parse.yaml
*/


#ifndef _KUL_YAML_HPP_
#define _KUL_YAML_HPP_

#include "kul/io.hpp"
#include "kul/map.hpp"
#include "kul/dbg.hpp"

#include "yaml-cpp/yaml.h"

namespace kul{ namespace yaml{

class Validator;
class NodeValidator;

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

enum NodeType { NON = 0, STRING, LIST, MAP };

class NodeValidator{
    private:        
        bool m;
        NodeType typ;
        std::string nam;
        std::vector<NodeValidator> kids;
    public:
        NodeValidator(const std::string n, bool m = 0) : m(m), typ(STRING), nam(n){}
        NodeValidator(const std::string n, 
                const std::vector<NodeValidator>& c,
                bool m, const NodeType& typ) :
                    m(m), typ(typ), nam(n), kids(c){}
        const std::vector<NodeValidator>&   children()  const { return this->kids; }
        bool                                mandatory() const { return this->m; }
        const std::string&                  name()      const { return this->nam; }
        const NodeType&                     type()      const { return this->typ; }
};

class Item{
    friend class Validator;
    protected:
        YAML::Node r;

        Item(){}
        Item(const YAML::Node& r) : r(r){}

        static void VALIDATE(const YAML::Node& n, const std::vector<NodeValidator>& nvs) KTHROW(Exception){
            KUL_DBG_FUNC_ENTER
            kul::hash::set::String keys;
            for(const auto& nv : nvs) if(nv.name() == "*") return;

            for(YAML::const_iterator it = n.begin(); it != n.end(); ++it){
                const std::string& key(it->first.as<std::string>());
                if(keys.count(key)) KEXCEPTION("Duplicate key detected: " + key);
                keys.insert(key);
                bool f = 0;
                for(const auto& nv : nvs){
                    if(nv.name() != key) continue;
                    f = 1;
                    if(nv.type() == 1 && it->second.Type() != 2) KEXCEPTION("String expected: " + nv.name());
                    if(nv.type() == 2 && it->second.Type() != 3) KEXCEPTION("List expected: " + nv.name());
                    if(nv.type() == 3 && it->second.Type() != 4) KEXCEPTION("Map expected: " + nv.name());
                    if(nv.type() == 2)
                        for(size_t i = 0; i < it->second.size(); i++)
                            VALIDATE(it->second[i], nv.children());
                    if(nv.type() == 3) VALIDATE(it->second, nv.children());
                }
                if(!f) KEXCEPTION("Unexpected key: " + key);
            }
            for(const auto& nv : nvs){
                if(nv.mandatory() && !keys.count(nv.name()))
                    KEXCEPTION("Key mandatory: : " + nv.name());
            }
        }
    public:
        virtual ~Item(){}
        const YAML::Node&       root() const { return r; }
};

class Validator{
    private:
        const std::vector<NodeValidator> kids;
    public:
        Validator(const std::vector<NodeValidator>& kids) : kids(kids){}
        const std::vector<NodeValidator>&   children()  const { return this->kids; }
        void validate(const YAML::Node& n){
            Item::VALIDATE(n, children());
        }
};

class String : public Item{
    private:
        const std::string s;
    public:
        String(const std::string& s) KTHROW(Exception) : s(s) {
            try{
                r = YAML::Load(s); 
            }catch(const std::exception& e){ KEXCEPTION("YAML failed to parse\nString: "+s); }
        }
        const YAML::Node& validate(const Validator&& v) KTHROW(Exception) {
            Item::VALIDATE(root(), v.children());
            return r;
        }
};

class File : public Item{
    private:
        const std::string f;
    protected:
        void reload() KTHROW(Exception){
            try{
                r = YAML::LoadFile(f); 
            }catch(const std::exception& e){ KEXCEPTION("YAML failed to parse\nFile: "+f); }
        }
        File(const File& f) : Item(f.r), f(f.f){}
        File(const std::string& f) KTHROW(Exception) : f(f) {
            reload();
        }
    public:
        template <class T> 
        static T CREATE(const std::string& f) KTHROW(Exception) {
            T file(f);
            try{
                Item::VALIDATE(file.root(), file.validator().children());
            }catch(const kul::yaml::Exception& e){
                KEXCEPTION("YAML error encountered in file: " + f);
            }
            return file;
        }
        virtual ~File(){}
        const std::string& file() const  { return f; }
        const virtual Validator validator() const = 0;
};


}}
#endif /* _KUL_YAML_HPP_ */
