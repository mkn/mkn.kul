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

#include "mkn/kul/cli.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/threads.hpp"

#if __has_include("benchmark/benchmark.h")
#include "benchmark/benchmark.h"
#else
#include "benchmark/benchmark_api.h"
#endif

void createDeleteFile(benchmark::State &state) {
  while (state.KeepRunning()) {
    mkn::kul::File f("tmp.tmp");
    f.mk();
    f.rm();
  }
}
BENCHMARK(createDeleteFile)->Unit(benchmark::kMicrosecond);

void parseStringAsCommandLineArguments(benchmark::State &state) {
  while (state.KeepRunning()) {
    std::vector<std::string> v;
    mkn::kul::cli::asArgs("/path/to \"words in quotes\" words\\ not\\ in\\ quotes end", v);
  }
}
BENCHMARK(parseStringAsCommandLineArguments)->Unit(benchmark::kMicrosecond);

void splitStringByChar(benchmark::State &state) {
  while (state.KeepRunning()) {
    std::vector<std::string> v;
    mkn::kul::String::SPLIT("split - by - char - dash", '-', v);
  }
}
BENCHMARK(splitStringByChar)->Unit(benchmark::kMicrosecond);

void splitStringByString(benchmark::State &state) {
  while (state.KeepRunning()) {
    std::vector<std::string> v;
    mkn::kul::String::SPLIT("split - by - char - dash", "-", v);
  }
}
BENCHMARK(splitStringByString)->Unit(benchmark::kMicrosecond);

void splitStringByEscapedChar(benchmark::State &state) {
  while (state.KeepRunning()) {
    std::vector<std::string> v;
    mkn::kul::String::ESC_SPLIT("split \\- by - char - dash", '-', v);
  }
}
BENCHMARK(splitStringByEscapedChar)->Unit(benchmark::kMicrosecond);

auto lambda = [](uint a, uint b) {
  auto c = (a + b);
  (void)c;
};

void concurrentThreadPool(benchmark::State &state) {
  while (state.KeepRunning()) {
    mkn::kul::ConcurrentThreadPool<> ctp(3, 1);
    for (size_t i = 0; i < 10000; i++) ctp.async(std::bind(lambda, 2, 4));
    ctp.block().finish().join();
  }
}
BENCHMARK(concurrentThreadPool)->Unit(benchmark::kMicrosecond);

void chroncurrentThreadPool(benchmark::State &state) {
  while (state.KeepRunning()) {
    mkn::kul::ChroncurrentThreadPool<> ctp(3, 1);
    for (size_t i = 0; i < 10000; i++) ctp.async(std::bind(lambda, 2, 4));
    ctp.block().finish().join();
  }
}
BENCHMARK(chroncurrentThreadPool)->Unit(benchmark::kMicrosecond);

int main(int argc, char **argv) {
  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}
