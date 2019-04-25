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

template <typename T>
void do_math(){
  T beta = 2;
  T a[2] = {1 , 2}, b[2] = {3, 4};
  kul::math::dot_matrix_vector_incr(2, 2, 2, &a[0], &b[0], beta, &b[0]);
  kul::math::dot(2u, &a[0], &b[0]);
  kul::math::scale(2u, 2u, &a[0]);
  kul::math::mult_incr(2u, 2u, &a[0], &b[0]);
}

template <typename T>
void do_math_has_atomics(){
  T a[2] = {1 , 2};
  std::atomic<T> b[2]; b[0] = 3; b[1] = 4;
  kul::math::dot(2u, &a[0], &b[0]);
  kul::math::scale(2u, 2u, &a[0]);
  kul::math::mult_incr(2u, 2u, &a[0], &b[0]);
}

TEST(Math, double) {
  do_math<double>();
}
TEST(Math, atomic_double) {
  do_math_has_atomics<double>();
}
TEST(Math, float) {
  do_math<float>();
}
TEST(Math, atomic_float) {
  do_math_has_atomics<float>();
}

TEST(Math, more) {
  do_math<uint32_t>();
  do_math<int32_t>();
}
