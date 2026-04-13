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
#ifndef _MKN_KUL_DBG_HPP_
#define _MKN_KUL_DBG_HPP_

#include "mkn/kul/log.hpp"
#include "mkn/kul/defs.hpp"

namespace mkn {
namespace kul {
namespace dbg {

class StackTrace {
 private:
  std::vector<std::string> _stk;

 public:
  StackTrace() = delete;
  StackTrace(StackTrace const&) = delete;
  StackTrace& operator=(StackTrace const&) = delete;
  StackTrace& operator=(StackTrace const&&) = delete;
  StackTrace(std::string const& s) { _stk.emplace_back(s); }
  StackTrace(StackTrace const&& that) {
    if (this != &that) this->_stk = that._stk;
  }
  std::vector<std::string> const& stack() const { return _stk; }
};

#if !defined(NDEBUG) || defined(MKN_KUL_FORCE_TRACE)

#define __MKN_KUL_TRACE__ 1

#ifndef MKN_KUL_TRACE_OR_VOID
#define MKN_KUL_TRACE_OR_VOID mkn::kul::dbg::StackTrace
#endif  // MKN_KUL_TRACE_OR_VOID

#ifndef MKN_KUL_STACK_TRACE
#define MKN_KUL_STACK_TRACE return mkn::kul::dbg::StackTrace(__PRETTY_FUNCTION__)
#endif  // MKN_KUL_STACK_TRACE

#ifndef MKN_KUL_LOG_SCOPE
#define MKN_KUL_LOG_SCOPE(s)                                              \
  mkn::kul::dbg::FunctionScope MKN_KUL_STR_CAT(_mkn_dbg_scope, __LINE__)( \
      __FILE__, std::string{__func__} + ":" + s, __LINE__)
#endif  // MKN_KUL_LOG_SCOPE

#ifndef MKN_KUL_DBG_FUNC_ENTER
#define MKN_KUL_DBG_FUNC_ENTER                                                                \
  mkn::kul::dbg::FunctionScope MKN_KUL_STR_CAT(_mkn_func_scope, __LINE__)(__FILE__, __func__, \
                                                                          __LINE__)
#endif  // MKN_KUL_DBG_FUNC_ENTER

#else  //

#define __MKN_KUL_TRACE__ 0
#define MKN_KUL_TRACE_OR_VOID void
#define MKN_KUL_STACK_TRACE
#define MKN_KUL_DBG_FUNC_ENTER
#define MKN_KUL_LOG_SCOPE(...)

#endif  // defined(MKN_KUL_FORCE_TRACE) || !defined(NDEBUG)

#if __MKN_KUL_TRACE__

class FunctionScope {
 public:
  FunctionScope(char const* fi, std::string const& fu, uint16_t const& li)
      : m_start(mkn::kul::Now::MICROS()), m_li(li), m_fi(fi), m_fu(fu) {
#if MKN_KUL_TRACE_SHOW_ENTRY
    KOUT(TRC) << mkn::kul::LogMan::INSTANCE().str(
        m_fi, m_fu.c_str(), m_li, mkn::kul::log::mode::TRC, "", "[%M]: %T - %D : %F:%L fn(%N)");
#endif
  }
  ~FunctionScope() {
    KOUT(TRC) << mkn::kul::LogMan::INSTANCE().str(m_fi, m_fu.c_str(), m_li,
                                                  mkn::kul::log::mode::TRC, "",
                                                  "[%M]: %T - %D : %F:%L fn(%N)")
              << " - Function time: " << (mkn::kul::Now::MICROS() - m_start) << " μs";
  }

 private:
  uint64_t m_start = 0, m_li = 0;
  char const* m_fi = nullptr;
  std::string m_fu;
};

#endif  // __MKN_KUL_TRACE__

}  // namespace dbg
}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_DBG_HPP_ */
