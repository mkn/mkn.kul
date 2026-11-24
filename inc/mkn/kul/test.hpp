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
#ifndef MKN_KUL_TEST_HPP_
#define MKN_KUL_TEST_HPP_

// checks without a test framework: one Tester declared (MKN_KUL_TEST_DECLARE), then checks
//  anywhere in that file (MKN_KUL_TEST_CHECK). Each check records where it was made; at exit the
//  Tester reports the failures, and with any the process exits 1
//
//   MKN_KUL_TEST_DECLARE();
//   int main() {
//     MKN_KUL_TEST_CHECK(1 + 1 == 2);
//     if (!MKN_KUL_TEST_CHECK(ptr != nullptr, "no ptr")) return 0;  // reported at exit
//   }

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <source_location>
#include <string>
#include <vector>
#include <utility>

namespace mkn::kul {

struct Tester {
  struct Check {
    bool ok = false;
    std::string what;  // the condition, or why it was checked
    std::source_location where;
  };

  // exit_on_fail: a failure at destruction exits the process with 1, for a static Tester whose
  //  destruction is the end of the program
  explicit Tester(bool const exit = true) : exit_on_fail{exit} {}

  Tester(Tester const&) = delete;
  Tester& operator=(Tester const&) = delete;

  ~Tester() {
    report();
    // destroyed during exit (a static): std::exit again would be undefined, _Exit ends here
    if (exit_on_fail && failed()) {
      std::fflush(nullptr);
      std::_Exit(1);
    }
  }

  bool check(bool const ok, std::string what,
             std::source_location const where = std::source_location::current()) {
    std::lock_guard<std::mutex> lock{mutex};
    checks.emplace_back(Check{ok, std::move(what), where});
    return ok;
  }

  // for MKN_KUL_TEST_CHECK: text is the macro's arguments, reported unless why is given
  template <typename Condition>
  bool check_text(char const* const text, Condition const& condition, std::string why = {},
                  std::source_location const where = std::source_location::current()) {
    return check(static_cast<bool>(condition), why.empty() ? text : std::move(why), where);
  }

  std::size_t failed() const {
    std::lock_guard<std::mutex> lock{mutex};
    std::size_t n = 0;
    for (auto const& c : checks) n += !c.ok;
    return n;
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lock{mutex};
    return checks.size();
  }

  // every check in order, passed or failed, then a summary, on stderr
  void report() const {
    std::lock_guard<std::mutex> lock{mutex};
    std::size_t fails = 0;
    for (auto const& c : checks) {
      fails += !c.ok;
      std::fprintf(stderr, "%s: %s:%u %s: %s\n", c.ok ? "PASSED" : "FAILED", c.where.file_name(),
                   static_cast<unsigned>(c.where.line()), c.where.function_name(), c.what.c_str());
    }
    std::fprintf(stderr, "%zu checks, %zu passed, %zu failed\n", checks.size(),
                 checks.size() - fails, fails);
  }

  bool const exit_on_fail;

 private:
  mutable std::mutex mutex;
  std::vector<Check> checks;
};

}  // namespace mkn::kul

// the file's Tester, at namespace scope
#define MKN_KUL_TEST_DECLARE() static mkn::kul::Tester _mkn_kul_tester

// a check of the declared Tester, (condition) or (condition, why), true if it passed
#define MKN_KUL_TEST_CHECK(...) _mkn_kul_tester.check_text(#__VA_ARGS__, __VA_ARGS__)

#endif /* MKN_KUL_TEST_HPP_ */
