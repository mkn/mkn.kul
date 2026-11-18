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
#ifndef MKN_KUL_RANDOM_HPP_
#define MKN_KUL_RANDOM_HPP_

// Reproducible hashing and random streams from plain 64-bit integer arithmetic: the same bits on
//  every platform and compiler (unsigned overflow wraps modulo 2^64 by definition in C++)

#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <limits>
#include <cstdint>
#include <string_view>

namespace mkn::kul {

// 2^64 / golden ratio, rounded to an odd integer ("golden gamma"). Adding it repeatedly modulo
//  2^64 visits all 2^64 values before coming back (any odd step does), and the golden ratio
//  spreads consecutive values far apart. The SplitMix64 increment and the hash_combine offset
inline constexpr std::uint64_t golden_gamma = 0x9e3779b97f4a7c15ULL;

// SplitMix64 finalizer: scrambles a 64-bit integer into a random-looking one. Close inputs (n and
//  n+1) give unrelated outputs, flipping one input bit flips about half the output bits.
//  Two rounds of "xor with a shifted copy, multiply by a large odd constant": the xor-shift folds
//  high bits down, the multiply pushes low bits up. Bijective (both steps are invertible), so
//  different inputs never collide. mix(0) == 0, which is why callers add golden_gamma first.
//  D. Stafford's "Mix13" variant of the MurmurHash3 64-bit finalizer, as in Java's
//  SplittableRandom and S. Vigna's splitmix64.c
MKN_KUL_FN_SIG_RET constexpr std::uint64_t splitmix64_mix(std::uint64_t z) {
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

// folds value into key, for a key built from several integers one after the other:
//   key = hash_combine(hash_combine(hash_combine(seed, a), b), c)
//  order matters, (3, 7) and (7, 3) give different keys. For a fixed key two different values
//  never give the same result (each step is bijective), any other coincidence is about 2^-64.
//  value is mixed before the xor so small values don't only flip a few low bits of the key
MKN_KUL_FN_SIG_RET constexpr std::uint64_t hash_combine(std::uint64_t const key,
                                                        std::uint64_t const value) {
  return splitmix64_mix(key ^ splitmix64_mix(value + golden_gamma));
}

// FNV-1a 64-bit hash of a string. Unlike std::hash it's fixed by its definition, the same in
//  every execution and standard library. Not a strong hash: feed it through hash_combine
MKN_KUL_FN_SIG_RET constexpr std::uint64_t fnv1a64(std::string_view const str) {
  std::uint64_t hash = 0xcbf29ce484222325ULL;  // FNV-1a 64-bit offset basis
  for (char const c : str) {
    hash ^= static_cast<std::uint8_t>(c);
    hash *= 0x100000001b3ULL;  // FNV-1a 64-bit prime
  }
  return hash;
}

// SplitMix64, a UniformRandomBitGenerator with an 8-byte state: a counter advancing by
//  golden_gamma, output scrambled by splitmix64_mix. The n-th number from seed k is
//  mix(k + n * golden_gamma), period 2^64. Cheap to construct, unlike std::mt19937_64 (~2.5 kB of
//  state and a costly seeding), so one per small work item is fine. Passes TestU01 BigCrush.
//  Two streams overlap only if their seeds are fewer than m steps of golden_gamma apart, m being
//  the numbers drawn from each.
//   G. L. Steele, D. Lea, C. H. Flood, "Fast splittable pseudorandom number generators",
//    OOPSLA 2014, doi:10.1145/2660193.2660195
//   S. Vigna, https://prng.di.unimi.it/splitmix64.c
class SplitMix64 {
 public:
  using result_type = std::uint64_t;

  MKN_KUL_FN_SIG constexpr explicit SplitMix64(std::uint64_t const seed) : state_{seed} {}

  MKN_KUL_FN_SIG_RET static constexpr result_type(min)() {
    return (std::numeric_limits<result_type>::min)();
  }
  MKN_KUL_FN_SIG_RET static constexpr result_type(max)() {
    return (std::numeric_limits<result_type>::max)();
  }

  MKN_KUL_FN_SIG constexpr result_type operator()() {
    state_ += golden_gamma;
    return splitmix64_mix(state_);
  }

 private:
  std::uint64_t state_;
};

}  // namespace mkn::kul

#endif /* MKN_KUL_RANDOM_HPP_ */
