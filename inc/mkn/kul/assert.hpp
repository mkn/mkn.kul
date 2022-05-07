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
#ifndef _MKN_KUL_ASSERT_HPP_
#define _MKN_KUL_ASSERT_HPP_

#include <cstdlib>

#include "mkn/kul/signal.hpp"

namespace mkn {
namespace kul {
struct Assert {
  template <typename T>
  Assert(T t) {
#if !defined(NDEBUG)
    if (!(t)) {
      auto tid = mkn::kul::this_thread::id();
      KOUT(NON) << tid << " : Stacktrace:";
      for (auto const& s : mkn::kul::this_thread::stacktrace()) KOUT(NON) << tid << " : " << s;
      exit(111);
    }
#endif
  }
};

void abort_if(bool b) {
  if (b) std::abort();
}

}  // namespace kul
}  // namespace mkn

#if !defined(KASSERT)
#define KASSERT(b) \
  mkn::kul::Assert { (b) }
#endif

#if defined(KASSERT_REPLACE_ASSERT)
#ifdef assert
#undef assert
#endif
#define assert(b) KASSERT(b)
#endif

#endif /* _MKN_KUL_ASSERT_HPP_ */
