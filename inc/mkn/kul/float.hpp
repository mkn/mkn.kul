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
#ifndef MKN_KUL_FLOAT_HPP_
#define MKN_KUL_FLOAT_HPP_

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>

namespace mkn::kul {

auto inline float_equals(float const a, float const b, float const diff = 1e-6) {
  return std::abs(a - b) < diff;
}

auto inline float_equals(double const a, double const b, double const diff = 1e-12) {
  return std::abs(a - b) < diff;
}

template <std::size_t S>
bool inline float_equals(std::array<double, S> const& a, std::array<double, S> const& b,
                         double const diff = 1e-15) {
  for (std::size_t i = 0; i < S; ++i)
    if (!float_equals(a[i], b[i], diff)) return false;
  return true;
}

auto inline float_not_equals(double const a, double const b, double const diff = 1e-12) {
  return std::abs(a - b) > diff;
}

// float_equals of floats or doubles in any mix, diff for doubles, scaled up 1e6 for a float
template <typename F0, typename F1>
auto any_float_eq(F0 const f0, F1 const f1, double diff = 1e-12) {
  if constexpr (sizeof(F0) == sizeof(F1)) {
    if (sizeof(F0) == 4) diff *= 1e6;
    return float_equals(f0, f1, diff);
  } else {
    return float_equals(static_cast<double>(f0), static_cast<double>(f1), diff * 1e6);
  }
}

// v0[i] == v1[i] within diff over [start, end), keeping the largest difference seen
template <typename V0, typename V1>
struct FloatComparator {
  bool operator()() {
    for (std::size_t i = start; i < end; ++i) {
      auto const ret = std::abs(v0[i] - v1[i]);
      eq &= ret <= diff;
      max_diff = ret > max_diff ? ret : max_diff;
    }
    return eq;
  }

  V0 const& v0;
  V1 const& v1;
  double const diff = 1e-15;
  std::size_t start = 0, end;

  bool eq = 1;
  double max_diff = 0;
};

}  // namespace mkn::kul

#endif /* MKN_KUL_FLOAT_HPP_ */
