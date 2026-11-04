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

#ifndef MKN_KUL_OS_BSD_MEM_HPP_
#define MKN_KUL_OS_BSD_MEM_HPP_

#include <cstdint>

#include <sys/resource.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <mach/mach.h>
#elif defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#endif

namespace mkn {
namespace kul {
namespace mem {
namespace detail {

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

#endif

// process high-water mark as reported by the OS, unclamped
inline std::uint64_t raw_peak_bytes() {
  struct rusage ru;
  if (getrusage(RUSAGE_SELF, &ru) != 0 || ru.ru_maxrss <= 0) return 0;
  auto const peak = static_cast<std::uint64_t>(ru.ru_maxrss);
#if defined(__APPLE__)
  return peak;  // bytes on darwin
#else
  return peak * 1024;  // KiB
#endif
}

}  // namespace detail

inline std::uint64_t rss_bytes() {
#if defined(__APPLE__)
  mach_task_basic_info info;
  return detail::task_basic(info) ? static_cast<std::uint64_t>(info.resident_size) : 0;
#elif defined(__FreeBSD__)
  struct kinfo_proc kp;
  return detail::kinfo(kp) ? static_cast<std::uint64_t>(kp.ki_rssize) * getpagesize() : 0;
#else
  return 0;
#endif
}

inline std::uint64_t virtual_bytes() {
#if defined(__APPLE__)
  mach_task_basic_info info;
  return detail::task_basic(info) ? static_cast<std::uint64_t>(info.virtual_size) : 0;
#elif defined(__FreeBSD__)
  struct kinfo_proc kp;
  return detail::kinfo(kp) ? static_cast<std::uint64_t>(kp.ki_size) : 0;
#else
  return 0;
#endif
}

}  // namespace mem
}  // namespace kul
}  // namespace mkn

#endif  // MKN_KUL_OS_BSD_MEM_HPP_
