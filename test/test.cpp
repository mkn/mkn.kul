/**
Copyright (c) 2023, Philip Deegan.
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "mkn/kul/assert.hpp"
#include "mkn/kul/cli.hpp"
#include "mkn/kul/io.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/math.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/threads.hpp"
#include "mkn/kul/span.hpp"
#include "mkn/kul/tuple.hpp"

#ifdef _WIN32
#define bzero ZeroMemory
#endif

auto tryCatch = [](std::vector<std::function<void()>> funcs, bool katch) {
  for (const auto& func : funcs) try {
      func();
      ASSERT_TRUE(!katch);
    } catch (const mkn::kul::Exception& e) {
      if (!katch) KOUT(NON) << e.debug();
      ASSERT_TRUE(katch);
    }
};

#include "test/cli.ipp"
#include "test/except.ipp"
#include "test/io.ipp"
#include "test/math.ipp"
#include "test/os.ipp"
#include "test/proc.ipp"
#include "test/string.ipp"
#include "test/span.ipp"

int main(int argc, char* argv[]) {
  KOUT(NON) << __FILE__;
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
