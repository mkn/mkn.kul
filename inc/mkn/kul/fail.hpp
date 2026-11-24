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
#ifndef MKN_KUL_FAIL_HPP_
#define MKN_KUL_FAIL_HPP_

// what an unrecoverable failure does (e.g. a GPU API error): fail() calls the installed handler,
//  by default printing where it happened and aborting. A handler can report more first, e.g. a
//  declared mkn::kul::Tester (test.hpp) adds it to its checks and prints them, then exits 1

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string>
#include <utility>

namespace mkn::kul {

struct Failure {
  std::string what;
  std::source_location where;
};

// must not return: fail() aborts if it does
using FailHandler = void (*)(Failure const&);

void inline print(Failure const& f) {
  std::fprintf(stderr, "FAILED: %s:%u %s: %s\n", f.where.file_name(),
               static_cast<unsigned>(f.where.line()), f.where.function_name(), f.what.c_str());
}

namespace detail {
void inline default_fail_handler(Failure const& f) {
  print(f);
  std::fflush(nullptr);
  std::abort();
}

std::atomic<FailHandler> inline fail_handler{default_fail_handler};
}  // namespace detail

// installs handler, returns the previous one
FailHandler inline set_fail_handler(FailHandler const handler) {
  return detail::fail_handler.exchange(handler ? handler : detail::default_fail_handler);
}

[[noreturn]] void inline fail(std::string what,
                              std::source_location const where = std::source_location::current()) {
  detail::fail_handler.load()(Failure{std::move(what), where});
  std::abort();
}

}  // namespace mkn::kul

#endif /* MKN_KUL_FAIL_HPP_ */
