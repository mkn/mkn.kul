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
// Process memory queries. All values are in bytes, 0 means unsupported/failed.
//
//   rss_bytes()      current resident set size
//   peak_rss_bytes() process lifetime resident set high-water mark
//   virtual_bytes()  virtual size (Windows: private committed bytes)
//   usage()          {rss, peak} together, reading current rss only once

#ifndef MKN_KUL_MEM_HPP_
#define MKN_KUL_MEM_HPP_

#include "mkn/kul/defs.hpp"

#if MKN_KUL_IS_NIX
#include "mkn/kul/os/nix/mem.hpp"  // IWYU pragma: export
#elif MKN_KUL_IS_BSD
#include "mkn/kul/os/bsd/mem.hpp"  // IWYU pragma: export
#elif MKN_KUL_IS_WIN
#include "mkn/kul/os/win/mem.hpp"  // IWYU pragma: export
#else
#error unresolved
#endif

namespace mkn {
namespace kul {
namespace mem {

struct Usage {
  std::uint64_t rss = 0, peak = 0;
};

// the OS peak and current rss can come from different counters (e.g. on Linux getrusage and
//  statm are computed from approximate per-cpu counters) and disagree slightly,
//  so peak is clamped to be at least the current rss
inline Usage usage() {
  Usage u;
  u.rss = rss_bytes();
  u.peak = detail::raw_peak_bytes();
  if (u.rss > u.peak) u.peak = u.rss;
  return u;
}

inline std::uint64_t peak_rss_bytes() { return usage().peak; }

}  // namespace mem
}  // namespace kul
}  // namespace mkn

#endif  // MKN_KUL_MEM_HPP_
