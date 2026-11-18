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
// IWYU pragma: private, include "mkn/kul/sys.hpp"
// IWYU pragma: friend ".*mkn/kul/os/(nix|nixish|bsd)/.*"

#ifndef MKN_KUL_OS_NIXISH_SYS_HPP_
#define MKN_KUL_OS_NIXISH_SYS_HPP_

#include "mkn/kul/os.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <dlfcn.h>
#include <concepts>
#include <filesystem>
#include <string>
#include <system_error>

#ifndef MKN_KUL_SYS_DLOPEN
#define MKN_KUL_SYS_DLOPEN RTLD_NOW | RTLD_GLOBAL
#endif  // MKN_KUL_SYS_DLOPEN

namespace mkn {
namespace kul {
namespace sys {

class SharedLibrary {
  template <class F>
  friend class SharedFunction;

 private:
  bool _loaded = 0;
  void* _handle;
  std::filesystem::path const _p;

 public:
  SharedLibrary(std::filesystem::path const& p) KTHROW(Exception) : _p(p) {
    std::error_code ec;
    if (!std::filesystem::is_regular_file(_p, ec))
      KEXCEPT(Exception, "Library attempted to be loaded does not exist: ", _p.string());
    _handle = dlopen(std::filesystem::canonical(_p, ec).string().c_str(), MKN_KUL_SYS_DLOPEN);
    if (!_handle) KEXCEPT(Exception, "Cannot load library: ", _p.string(), " - Error: ", dlerror());
    _loaded = 1;
  }
  template <std::same_as<mkn::kul::File> F>
  MKN_KUL_DEPRECATED("use std::filesystem::path")
  SharedLibrary(F const& f) KTHROW(Exception) : SharedLibrary(f.full()) {}
  ~SharedLibrary() {
    if (_loaded) dlclose(_handle);
    dlerror();
  }
  std::filesystem::path const& path() const { return _p; }
  MKN_KUL_DEPRECATED("use std::filesystem::path") mkn::kul::File file() const {
    return mkn::kul::File(_p.string());
  }
};

template <class F>
class SharedFunction {
 private:
  F* _funcP;
  SharedLibrary& _lib;

 public:
  SharedFunction(SharedLibrary& lib, std::string const& f) KTHROW(Exception) : _lib(lib) {
    _funcP = (F*)dlsym(_lib._handle, f.c_str());
    char const* dlsym_error = dlerror();
    if (dlsym_error) KEXCEPT(Exception, "Cannot load symbol create ", dlsym_error);
  }
  ~SharedFunction() { dlerror(); }
  F* pointer() { return _funcP; }
};

template <class T>
class SharedClass {
  typedef T* construct_t();
  typedef void destruct_t(T* t);

 private:
  SharedLibrary _lib;
  SharedFunction<construct_t> _c;
  SharedFunction<destruct_t> _d;

 public:
  SharedClass(std::filesystem::path const& p, std::string const& c, std::string const& d)
      KTHROW(Exception)
      : _lib(p), _c(_lib, c), _d(_lib, d) {}
  template <std::same_as<mkn::kul::File> F>
  MKN_KUL_DEPRECATED("use std::filesystem::path")
  SharedClass(F const& f, std::string const& c, std::string const& d) KTHROW(Exception)
      : SharedClass(f.full(), c, d) {}
  virtual ~SharedClass() {}

 protected:
  void construct(T*& t) KTHROW(Exception) {
    t = _c.pointer()();
    if (!t) KEXCEPT(Exception, "Dynamically loaded class was not created");
  }
  void destruct(T*& t) {
    _d.pointer()(t);
    t = nullptr;
    if (t) KEXCEPT(Exception, "Dynamically loaded class was not destroyed");
  }
};

}  // namespace sys
}  // namespace kul
}  // namespace mkn

#endif /* MKN_KUL_OS_NIXISH_SYS_HPP_ */
