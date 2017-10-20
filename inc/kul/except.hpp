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
#ifndef _KUL_EXCEPT_HPP_
#define _KUL_EXCEPT_HPP_

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <kul/def.hpp>

namespace kul {

// For function signatures
#if __cplusplus > 199711L
#ifndef KNOTHROW
#define KNOTHROW noexcept(true)
#endif // KNOTHROW
#ifndef KTHROW
#define KTHROW(x) noexcept(false)
#endif // KTHROW
#endif //__cplusplus

#ifndef KNOTHROW
#define KNOTHROW
#endif // KNOTHROW
#ifndef KTHROW
#define KTHROW(x)
#endif // KTHROW

class Exception : public std::runtime_error
{
  friend std::ostream& operator<<(std::ostream&, const Exception&);

protected:
  const char* _f = nullptr;
  const uint16_t _l = 0;
  const std::exception_ptr _ep;
  std::stringstream msg;

public:
  ~Exception() KNOTHROW {}
  Exception(const char* f, const uint16_t& l, const std::string& s = "")
    : std::runtime_error(s)
    , _f(f)
    , _l(l)
    , _ep(std::current_exception())
  {}
  Exception(const Exception& e)
    : std::runtime_error(e)
    , _f(e.file())
    , _l(e.line())
    , _ep(e._ep)
  {
    msg << e.msg.str();
  }
  Exception& operator=(const Exception& e) = default;

  std::string debug() const
  {
    std::stringstream ss;
    ss << (_f ? _f : "<UNKNOWN FILE>") << " : " << _l << " : " << what()
       << msg.str();
    return ss.str();
  }
  std::string message() const
  {
    std::stringstream ss;
    ss << what() << msg.str();
    return ss.str();
  }

  const char* file() const { return _f; }
  const uint16_t& line() const { return _l; }
  const std::exception_ptr& cause() const { return _ep; }
  const std::string stack() const
  {
    std::stringstream ss;
    if (_ep) {
      try {
        std::rethrow_exception(_ep);
      } catch (const kul::Exception& e) {
        ss << e.stack() << std::endl;
      } catch (const std::exception& e) {
        ss << e.what() << std::endl;
      } catch (...) {
        ss << "UNKNOWN EXCEPTION TYPE" << std::endl;
      }
    }
    ss << debug();
    return ss.str();
  }
  template<class T>
  Exception& operator<<(const T& s)
  {
    msg << s;
    return *this;
  }
};

inline std::ostream&
operator<<(std::ostream& s, const Exception& e)
{
  return s << e.message();
}

class Exit : public Exception
{
private:
  const uint16_t _e;

public:
  Exit(const char* f,
       const uint16_t& l,
       const std::string& s,
       const uint16_t& e)
    : Exception(f, l, s)
    , _e(e)
  {}
  Exit(const Exit& e)
    : Exception(e)
    , _e(e._e)
  {}
  Exit& operator=(const Exit& e) = default;

  const uint16_t& code() const { return _e; }

  template<class T>
  Exit& operator<<(const T& s)
  {
    msg << s;
    return *this;
  }
};

#define KEXCEPT(e, m) throw e(__FILE__, __LINE__, m)
#define KEXCEPTSTR(e) throw e(__FILE__, __LINE__, "")
#define KEXCEPTION(m) throw Exception(__FILE__, __LINE__, m)
#define KEXCEPSTREAM throw Exception(__FILE__, __LINE__, "")

#define KEXIT(e, m) throw kul::Exit(__FILE__, __LINE__, m, e)

} // end namespace kul
#endif /* _KUL_EXCEPT_HPP_ */
