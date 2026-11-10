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
#ifndef MKN_KUL_LANG_CPP_NVCC_HPP
#define MKN_KUL_LANG_CPP_NVCC_HPP

#include "mkn/kul/lang/cpp/gcc.hpp"

#include <string>

namespace mkn::kul::lang::cpp {

// nvcc takes gcc's compile and link steps, but not gcc's options: host compiler options go
//  through -Xcompiler, linker options through -Xlinker
class NvccCompiler : public GccCompiler {
 public:
  NvccCompiler(int const& v = 0) : GccCompiler(v) {
    m_optimise_c = {{0, ""},
                    {1, "-O1"},
                    {2, "-O2"},
                    {3, "-O3"},
                    {4, "-O3 -Xcompiler -funroll-loops"},
                    {5, "-O3 -Xcompiler -funroll-loops"},
                    {6, "-O3 -Xcompiler -funroll-loops"},
                    {7, "-O3 -Xcompiler -funroll-loops"},
                    {8, "-O3 -Xcompiler -funroll-loops"},
                    {9, "-O3 -Xcompiler -funroll-loops,-march=native,-mtune=native"}};
    m_debug_c = {
        {0, "-DNDEBUG"},     {1, "-g -lineinfo"}, {2, "-g -lineinfo"}, {3, "-g -lineinfo"},
        {4, "-g -lineinfo"}, {5, "-g -lineinfo"}, {6, "-g -lineinfo"}, {7, "-g -lineinfo"},
        {8, "-g -lineinfo"}, {9, "-g -G"}};  // -G: device debug, disables device optimisation
    m_warn_c = {{0, "-w"},
                {1, "-Xcompiler -Wall"},
                {2, "-Xcompiler -Wall"},
                {3, "-Xcompiler -Wall"},
                {4, "-Xcompiler -Wall"},
                {5, "-Xcompiler -Wall"},
                {6, "-Xcompiler -Wall"},
                {7, "-Xcompiler -Wall"},
                {8, "-Xcompiler -Wall,-Wextra"},
                {9, "-Xcompiler -Wall,-Wextra,-Werror -Werror all-warnings"}};
  }

  CCompiler_Type type() const override { return CCompiler_Type::NVCC; }
  std::string linkerOption(std::string const& option) const override {
    return "-Xlinker=" + option;
  }

 protected:
  std::string ccBinary() const override { return CC("nvcc"); }
  std::string cxxBinary() const override { return CXX("nvcc"); }
};

}  // namespace mkn::kul::lang::cpp

#endif  // MKN_KUL_LANG_CPP_NVCC_HPP
