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

// Standalone process memory queries - system headers only, no other kul includes,
//  so it can be included (or copied) by other projects without pulling in kul.
//
//  All values are in bytes. 0 means unsupported/failed on this platform.
//
//   rss_bytes()      current resident set size
//   peak_rss_bytes() process lifetime resident set high-water mark
//   virtual_bytes()  virtual size (Windows: private committed bytes)
//   usage()          {rss, peak} together, reading current rss only once

#ifndef MKN_KUL_OS_MEM_HPP_
#define MKN_KUL_OS_MEM_HPP_

#include <cstdint>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>  // K32GetProcessMemoryInfo lives in kernel32, no psapi.lib needed
#else
#include <sys/resource.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <mach/mach.h>
#elif defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#elif defined(__linux__)
#include <fcntl.h>
#endif
#endif

namespace mkn {
namespace kul {
namespace os {
namespace mem {

namespace detail {

#if defined(_WIN32)

inline bool counters(PROCESS_MEMORY_COUNTERS_EX& pmc) {
  return K32GetProcessMemoryInfo(GetCurrentProcess(),
                                 reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));
}

#else

inline std::uint64_t page_size() {
  static std::uint64_t const ps = [] {
    long const v = sysconf(_SC_PAGESIZE);
    return v > 0 ? static_cast<std::uint64_t>(v) : 4096ull;
  }();
  return ps;
}

#if defined(__APPLE__)

inline bool task_basic(mach_task_basic_info& info) {
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  return KERN_SUCCESS == task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                                   reinterpret_cast<task_info_t>(&info), &count);
}

#elif defined(__FreeBSD__)

inline bool kinfo(struct kinfo_proc& kp) {
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(getpid())};
  size_t len = sizeof(kp);
  return sysctl(mib, 4, &kp, &len, nullptr, 0) == 0 && len == sizeof(kp);
}

#elif defined(__linux__)

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

#endif  // platform

#endif  // _WIN32

}  // namespace detail

inline std::uint64_t rss_bytes() {
#if defined(_WIN32)
  PROCESS_MEMORY_COUNTERS_EX pmc;
  return detail::counters(pmc) ? static_cast<std::uint64_t>(pmc.WorkingSetSize) : 0;
#elif defined(__APPLE__)
  mach_task_basic_info info;
  return detail::task_basic(info) ? static_cast<std::uint64_t>(info.resident_size) : 0;
#elif defined(__FreeBSD__)
  struct kinfo_proc kp;
  return detail::kinfo(kp) ? static_cast<std::uint64_t>(kp.ki_rssize) * detail::page_size() : 0;
#elif defined(__linux__)
  std::uint64_t size = 0, resident = 0;
  return detail::statm(size, resident) ? resident * detail::page_size() : 0;
#else
  return 0;
#endif
}

namespace detail {

// process high-water mark as reported by the OS, unclamped
inline std::uint64_t raw_peak_bytes() {
#if defined(_WIN32)
  PROCESS_MEMORY_COUNTERS_EX pmc;
  return counters(pmc) ? static_cast<std::uint64_t>(pmc.PeakWorkingSetSize) : 0;
#else
  struct rusage ru;
  if (getrusage(RUSAGE_SELF, &ru) != 0 || ru.ru_maxrss <= 0) return 0;
  auto const peak = static_cast<std::uint64_t>(ru.ru_maxrss);
#if defined(__APPLE__)
  return peak;  // bytes on darwin
#else
  return peak * 1024;  // KiB elsewhere
#endif
#endif
}

}  // namespace detail

struct Usage {
  std::uint64_t rss = 0, peak = 0;
};

// on Linux, getrusage and statm are computed from approximate per-cpu counters
//  and can disagree slightly, so peak is clamped to be at least the current rss
inline Usage usage() {
  Usage u;
  u.rss = rss_bytes();
  u.peak = detail::raw_peak_bytes();
  if (u.rss > u.peak) u.peak = u.rss;
  return u;
}

inline std::uint64_t peak_rss_bytes() { return usage().peak; }

inline std::uint64_t virtual_bytes() {
#if defined(_WIN32)
  PROCESS_MEMORY_COUNTERS_EX pmc;
  return detail::counters(pmc) ? static_cast<std::uint64_t>(pmc.PrivateUsage) : 0;
#elif defined(__APPLE__)
  mach_task_basic_info info;
  return detail::task_basic(info) ? static_cast<std::uint64_t>(info.virtual_size) : 0;
#elif defined(__FreeBSD__)
  struct kinfo_proc kp;
  return detail::kinfo(kp) ? static_cast<std::uint64_t>(kp.ki_size) : 0;
#elif defined(__linux__)
  std::uint64_t size = 0, resident = 0;
  return detail::statm(size, resident) ? size * detail::page_size() : 0;
#else
  return 0;
#endif
}

}  // namespace mem
}  // namespace os
}  // namespace kul
}  // namespace mkn

#endif  // MKN_KUL_OS_MEM_HPP_
