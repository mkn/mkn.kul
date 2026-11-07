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
// IWYU pragma: private, include "mkn/kul/mem.hpp"
// IWYU pragma: friend ".*mkn/kul/os/(nix|nixish|bsd)/.*"

#ifndef MKN_KUL_OS_NIX_MEM_HPP_
#define MKN_KUL_OS_NIX_MEM_HPP_

#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <bits/types/struct_rusage.h>
#include <cstdint>

namespace mkn {
namespace kul {
namespace mem {
namespace detail {

inline std::uint64_t page_size() {
  static std::uint64_t const ps = [] {
    long const v = sysconf(_SC_PAGESIZE);
    return v > 0 ? static_cast<std::uint64_t>(v) : 4096ull;
  }();
  return ps;
}

// /proc/self/statm: "size resident shared text lib data dt" in pages
//  raw read(2) - no stdio locking/allocation, so it's cheap enough for hot paths
inline bool statm(std::uint64_t& size, std::uint64_t& resident) {
  int const fd = open("/proc/self/statm", O_RDONLY | O_CLOEXEC);
  if (fd < 0) return false;
  char buf[128];
  auto const n = read(fd, buf, sizeof(buf) - 1);
  close(fd);
  if (n <= 0) return false;
  buf[n] = '\0';

  char const* p = buf;
  auto const next = [&](std::uint64_t& v) {
    while (*p == ' ') ++p;
    if (*p < '0' || *p > '9') return false;
    for (v = 0; *p >= '0' && *p <= '9'; ++p) v = v * 10 + static_cast<std::uint64_t>(*p - '0');
    return true;
  };
  return next(size) && next(resident);
}

// process high-water mark as reported by the OS, unclamped
inline std::uint64_t raw_peak_bytes() {
  struct rusage ru;
  if (getrusage(RUSAGE_SELF, &ru) != 0 || ru.ru_maxrss <= 0) return 0;
  return static_cast<std::uint64_t>(ru.ru_maxrss) * 1024;  // KiB
}

}  // namespace detail

inline std::uint64_t rss_bytes() {
  std::uint64_t size = 0, resident = 0;
  return detail::statm(size, resident) ? resident * detail::page_size() : 0;
}

inline std::uint64_t virtual_bytes() {
  std::uint64_t size = 0, resident = 0;
  return detail::statm(size, resident) ? size * detail::page_size() : 0;
}

}  // namespace mem
}  // namespace kul
}  // namespace mkn

#endif  // MKN_KUL_OS_NIX_MEM_HPP_
