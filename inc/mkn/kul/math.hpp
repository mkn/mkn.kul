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
#ifndef _MKN_KUL_MATH_HPP_
#define _MKN_KUL_MATH_HPP_

#include <math.h>
#include <numeric>
#include <algorithm>

#include "mkn/kul/dbg.hpp"

namespace mkn {
namespace kul {
namespace math {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const *f, uint16_t const &l, std::string const &s)
      : mkn::kul::Exception(f, l, s) {}
};

template <class T>
T abs(const T &f) {
  return f < 0 ? f * -1 : f;
}

template <class T = float>
T pow(const float &f, const int16_t &e = 2) {
  T r = f < 0 ? -1 : 1;
  for (uint16_t i = 0; i < abs(e); i++) r *= f;
  return e < 0 ? 1 / r : r;
}

template <class T = float>
T root(const float &f, const int16_t &r = 2, uint16_t const &it = 6, T g = 0) {
  if (r < 1) KEXCEPT(Exception, "Invalid root quotient, must be greater than abs(1)");
  if (g == 0) g = r >= f || r + r >= f ? 1 : (int)f / (r + r);
  for (uint16_t i = 0; i < it; i++) g = ((float)1 / r) * (((r - 1) * g) + (f / pow<T>(g, r - 1)));
  return g;
}



template<typename Container, typename Multiplies = typename Container::value_type>
Multiplies product(Container const& container, Multiplies mul = 1)
{
    return std::accumulate(container.begin(), container.end(), mul, std::multiplies<Multiplies>());
}


template<typename Container, typename Return = typename Container::value_type>
Return sum(Container const& container, Return r = 0)
{
    return std::accumulate(container.begin(), container.end(), r);
}





}  // namespace math
}  // namespace kul
}  // namespace mkn

#include <algorithm>
#include <numeric>
#include <type_traits>

#include "mkn/kul/math/noop.hpp"

#if defined(_MKN_KUL_USE_MKL)
#include "mkl.h"
#elif defined(_MKN_KUL_USE_CBLAS)
#if defined(__APPLE__)
#include <Accelerate/Accelerate.h>
#else
extern "C" {
#include <cblas.h>
}
#endif  // defined(__APPLE__)
#endif
#if defined(_MKN_KUL_USE_MKL) || defined(_MKN_KUL_USE_CBLAS)
#include "mkn/kul/math/blas.hpp"
#else
#include "mkn/kul/math/noblas.hpp"
#endif
#endif  // _MKN_KUL_MATH_HPP_
