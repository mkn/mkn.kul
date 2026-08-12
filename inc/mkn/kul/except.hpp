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
#ifndef MKN_KUL_EXCEPT_HPP_
#define MKN_KUL_EXCEPT_HPP_

#include "mkn/kul/defs.hpp"

#include <cstdint>
#include <memory>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace mkn::kul {

// For function signatures
#if __cplusplus > 199711L
#ifndef KNOTHROW
#define KNOTHROW noexcept(true)
#endif  // KNOTHROW
#ifndef KTHROW
#define KTHROW(x) noexcept(false)
#endif  // KTHROW
#endif  //__cplusplus

#ifndef KNOTHROW
#define KNOTHROW
#endif  // KNOTHROW
#ifndef KTHROW
#define KTHROW(x)
#endif  // KTHROW

class Exception : public std::runtime_error {
 public:
  virtual ~Exception() KNOTHROW {}

  Exception(char const* f, uint16_t const& l, std::string const& s = "")
      : std::runtime_error(s),
        _f(f),
        _l(l),
        _ep(std::current_exception()),
        err(std::make_shared<std::string const>(s)) {}

  Exception(Exception const& e) = default;
  Exception(Exception&& e) = default;

  std::string debug() const {
    std::stringstream ss;
    ss << (_f ? _f : "<UNKNOWN FILE>") << " : " << _l << " : " << *err;
    return ss.str();
  }

  char const* what() const noexcept override { return err->c_str(); }
  std::string const& str() const noexcept { return *err; }

  char const* file() const { return _f; }
  uint16_t const& line() const { return _l; }
  std::exception_ptr const& cause() const { return _ep; }
  std::string const stack() const {
    std::stringstream ss;
    if (_ep) {
      try {
        std::rethrow_exception(_ep);
      } catch (mkn::kul::Exception const& e) {
        ss << e.stack() << std::endl;
      } catch (std::exception const& e) {
        ss << e.what() << std::endl;
      } catch (...) {
        ss << "UNKNOWN EXCEPTION TYPE" << std::endl;
      }
    }
    ss << debug();
    return ss.str();
  }

 protected:
  char const* _f;
  uint16_t const _l;
  std::exception_ptr _ep;
  std::shared_ptr<std::string const> err;

  Exception& operator=(Exception& e) = delete;
  Exception& operator=(Exception&& e) = delete;
  Exception& operator=(Exception const& e) = delete;
  Exception& operator=(Exception const&& e) = delete;

  friend std::ostream& operator<<(std::ostream&, Exception const&);
};

inline std::ostream& operator<<(std::ostream& s, Exception const& e) {
  std::cout << __FILE__ << " " << __LINE__ << " " << e.str();
  return s << e.str();
}

class Exit : public Exception {
 public:
  Exit(char const* f, uint16_t const& l, std::string const& s, int const& e)
      : Exception(f, l, s), _e(e) {}
  Exit(Exit const& e) = default;
  Exit(Exit&& e) = default;

  int const& code() const { return _e; }

 private:
  int const _e;

  Exit& operator=(Exit& e) = delete;
  Exit& operator=(Exit&& e) = delete;
  Exit& operator=(Exit const& e) = delete;
  Exit& operator=(Exit const&& e) = delete;
};

namespace except {
template <class... Args>
inline std::string str(Args const&... args) {
  std::ostringstream ss;
  (ss << ... << args);
  return ss.str();
}
}  // namespace except

}  // namespace mkn::kul

#define KEXCEPT(e, ...) throw e(__FILE__, __LINE__, ::mkn::kul::except::str(__VA_ARGS__))
#define KEXCEPTSTR(e) throw e(__FILE__, __LINE__, "")
#define KEXCEPTION(...) \
  throw ::mkn::kul::Exception(__FILE__, __LINE__, ::mkn::kul::except::str(__VA_ARGS__))

#define KEXIT(e, m) throw mkn::kul::Exit(__FILE__, __LINE__, m, e)

#endif /* MKN_KUL_EXCEPT_HPP_ */
