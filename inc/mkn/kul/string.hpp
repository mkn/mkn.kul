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
#ifndef _MKN_KUL_STRING_HPP_
#define _MKN_KUL_STRING_HPP_

#include "mkn/kul/except.hpp"

#include <cmath>
#include <cctype>
#include <cerrno>
#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <utility>
#include <algorithm>
#include <unordered_map>

namespace mkn {
namespace kul {

enum STR_INT_RET { IS_SUCCESS = 0, IS_OVERFLOW, IS_UNDERFLOW, IS_INCONVERTIBLE };

class StringException : public mkn::kul::Exception {
 public:
  StringException(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class String;
class StringOpHelper {
  friend class String;

 private:
  std::unordered_map<uint16_t, std::string> STR_INT_STR;
  std::string getStrForRet(const STR_INT_RET& ret) { return (*STR_INT_STR.find(ret)).second; }
  StringOpHelper() {
    STR_INT_STR.insert(std::make_pair(0, "SUCCESS"));
    STR_INT_STR.insert(std::make_pair(1, "OVERFLOW"));
    STR_INT_STR.insert(std::make_pair(2, "UNDERFLOW"));
    STR_INT_STR.insert(std::make_pair(3, "INCONVERTIBLE"));
  }

 public:
  static StringOpHelper& INSTANCE() {
    static StringOpHelper i;
    return i;
  }
};

class String {
 public:
  static void REPLACE(std::string& s, std::string const& f, std::string const& r) {
    size_t p = 0;
    if ((p = s.find(f)) != std::string::npos) s.replace(p, f.size(), r);
  }
  static void REPLACE_ALL(std::string& s, std::string const& f, std::string const& r) {
    size_t p = s.find(f);
    while (p != std::string::npos) {
      s.replace(s.find(f, p), f.size(), r);
      p = s.find(f, p + r.size());
    }
  }
  static void TRIM_LEFT(std::string& s, char const& delim = ' ') {
    while (s.find(delim) == 0) s.erase(0, 1);
  }
  static void TRIM_RIGHT(std::string& s, char const& delim = ' ') {
    while (s.rfind(delim) == s.size() - 1) s.pop_back();
  }
  static void TRIM(std::string& s) {
    while (s.find(' ') == 0 || s.find('\t') == 0) s.erase(0, 1);
    if (s.size() == 0) return;
    while (s.rfind(' ') == s.size() - 1 || s.rfind('\t') == s.size() - 1) s.pop_back();
  }

  template <typename V>
  static void TRIM(V& strs) {
    for (auto& v : strs) TRIM(v);
  }

  static void PAD(std::string& s, uint16_t const& p) {
    while (s.size() < p) s += " ";
  }
  static std::vector<std::string> SPLIT(std::string const& s, char const& d) {
    std::vector<std::string> v;
    SPLIT(s, d, v);
    return v;
  }
  static void SPLIT(std::string const& s, char const& d, std::vector<std::string>& v) {
    if (s.find(d) != std::string::npos) {
      std::string l;
      std::stringstream stream(s);
      while (std::getline(stream, l, d))
        if (l.compare("") != 0) v.push_back(l);
    } else
      v.push_back(s);
  }
  static std::vector<std::string> SPLIT(std::string const& s, std::string const& d) {
    std::vector<std::string> v;
    SPLIT(s, d, v);
    return v;
  }
  static void SPLIT(std::string const& s, std::string const& d, std::vector<std::string>& v) {
    std::string l = s;
    size_t pos = 0;
    while ((pos = l.find(d)) != std::string::npos) {
      v.push_back(l.substr(0, pos));
      l = l.substr(pos + 1);
    }
    v.push_back(l);
  }
  static std::vector<std::string> ESC_SPLIT(std::string const& s, char const& d,
                                            char const& e = '\\') {
    std::vector<std::string> v;
    ESC_SPLIT(s, d, v, e);
    return v;
  }
  static void ESC_SPLIT(std::string const& s, char const& d, std::vector<std::string>& v,
                        char const& e = '\\') {
    std::string l = s, ds = std::string(1, d), es = std::string(1, e);
    std::string tmp = l;
    size_t pos = 0, esc = 0;
    while ((pos = l.find(d, esc)) != std::string::npos) {
      if (pos && l[pos - 1] == e) {
        esc = pos + 1;
        tmp = l.substr(esc);
        continue;
      }
      if (tmp.find(d, esc) == std::string::npos) break;
      std::string t = l.substr(0, pos);
      REPLACE_ALL(t, es, "");
      v.push_back(t);
      l = l.substr(pos + 1);
      esc = 0;
      tmp = l;
    }
    REPLACE_ALL(l, es, "");
    v.push_back(l);
  }
  static bool NO_CASE_CMP(std::string a, std::string b) {
    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
    std::transform(b.begin(), b.end(), b.begin(), ::tolower);
    return (a == b);
  }
  static std::vector<std::string> LINES(std::string const& s) {
    std::vector<std::string> v;
    LINES(s, v);
    return v;
  }
  static void LINES(std::string const& s, std::vector<std::string>& v) {
    if (s.find("\n") != std::string::npos) {
      std::string l;
      std::stringstream ss(s);
      while (std::getline(ss, l))
        if (!l.empty()) v.push_back(l);
    } else
      v.push_back(s);
  }

  static bool BOOL(std::string s) {
    TRIM(s);
    std::vector<std::string> const& pos{"yes", "y", "true", "1"};
    std::vector<std::string> const& neg{"no", "n", "false", "0"};
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    if (std::find(pos.begin(), pos.end(), s) != pos.end()) return true;
    if (std::find(neg.begin(), neg.end(), s) != neg.end()) return false;
    KEXCEPT(StringException, "input not bool-able, " + s);
  }

  static uint16_t UINT16(std::string const& str) KTHROW(StringException) {
    auto lambda = [](char const* s, uint32_t& lresult) {
      char* end;
      errno = 0;
      lresult = strtoul(s, &end, 10);
      if ((errno == ERANGE) || lresult > UINT16_MAX) return IS_OVERFLOW;
      if (*s == '\0' || *end != '\0') return IS_INCONVERTIBLE;
      return IS_SUCCESS;
    };
    uint32_t lresult = 0;
    STR_INT_RET ret = lambda(str.c_str(), lresult);
    if (ret != IS_SUCCESS)
      KEXCEPT(StringException,
              "UINT16 conversion failed, reason: " + StringOpHelper::INSTANCE().getStrForRet(ret));
    uint16_t result = lresult;
    if (result != lresult) KEXCEPT(StringException, "UINT16 conversion failed");
    return result;
  }
  static int16_t INT16(std::string const& str) KTHROW(StringException) {
    auto lambda = [](char const* s, int32_t& lresult) {
      char* end;
      errno = 0;
      lresult = strtol(s, &end, 10);
      if ((errno == ERANGE) || lresult > INT16_MAX) return IS_OVERFLOW;
      if ((errno == ERANGE) || lresult < INT16_MIN) return IS_UNDERFLOW;
      if (*s == '\0' || *end != '\0') return IS_INCONVERTIBLE;
      return IS_SUCCESS;
    };
    int32_t lresult = 0;
    STR_INT_RET ret = lambda(str.c_str(), lresult);
    if (ret != IS_SUCCESS)
      KEXCEPT(StringException,
              "INT16 conversion failed, reason: " + StringOpHelper::INSTANCE().getStrForRet(ret));
    int16_t result = lresult;
    if (result != lresult) KEXCEPT(StringException, "INT16 conversion failed");
    return result;
  }

  static uint32_t UINT32(std::string const& str) KTHROW(StringException) {
    auto lambda = [](char const* s, uint64_t& lresult) {
      char* end;
      errno = 0;
      lresult = strtoull(s, &end, 10);
      if ((errno == ERANGE) || lresult > UINT32_MAX) return IS_OVERFLOW;
      if (*s == '\0' || *end != '\0') return IS_INCONVERTIBLE;
      return IS_SUCCESS;
    };
    uint64_t lresult = 0;
    STR_INT_RET ret = lambda(str.c_str(), lresult);
    if (ret != IS_SUCCESS)
      KEXCEPT(StringException,
              "UINT32 conversion failed, reason: " + StringOpHelper::INSTANCE().getStrForRet(ret));
    uint32_t result = lresult;
    if (result != lresult) KEXCEPT(StringException, "UINT32 conversion failed");
    return result;
  }
  static int32_t INT32(std::string const& str) KTHROW(StringException) {
    auto lambda = [](char const* s, int64_t& lresult) {
      char* end;
      errno = 0;
      lresult = strtoll(s, &end, 10);
      if ((errno == ERANGE) || lresult > INT32_MAX) return IS_OVERFLOW;
      if ((errno == ERANGE) || lresult < INT32_MIN) return IS_UNDERFLOW;
      if (*s == '\0' || *end != '\0') return IS_INCONVERTIBLE;
      return IS_SUCCESS;
    };
    int64_t lresult = 0;
    STR_INT_RET ret = lambda(str.c_str(), lresult);
    if (ret != IS_SUCCESS)
      KEXCEPT(StringException,
              "INT32 conversion failed, reason: " + StringOpHelper::INSTANCE().getStrForRet(ret));
    int32_t result = lresult;
    if (result != lresult) KEXCEPT(StringException, "INT32 conversion failed");
    return result;
  }

  static uint64_t UINT64(std::string const& str) KTHROW(StringException) {
    auto lambda = [](char const* s, uint64_t& lresult) {
      char* end;
      errno = 0;
      lresult = strtoull(s, &end, 10);
      if ((errno == ERANGE) || lresult > UINT64_MAX) return IS_OVERFLOW;
      if (*s == '\0' || *end != '\0') return IS_INCONVERTIBLE;
      return IS_SUCCESS;
    };
    uint64_t lresult = 0;
    STR_INT_RET ret = lambda(str.c_str(), lresult);
    if (ret != IS_SUCCESS)
      KEXCEPT(StringException,
              "UINT64 conversion failed, reason: " + StringOpHelper::INSTANCE().getStrForRet(ret));
    uint64_t result = lresult;
    if (result != lresult) KEXCEPT(StringException, "UINT64 conversion failed");
    return result;
  }

  static int64_t INT64(std::string const& str) KTHROW(StringException) {
    auto lambda = [](char const* s, int64_t& lresult) {
      char* end;
      errno = 0;
      lresult = strtoll(s, &end, 10);
      if ((errno == ERANGE) || lresult > INT64_MAX) return IS_OVERFLOW;
      if ((errno == ERANGE) || lresult < 0) return IS_UNDERFLOW;
      if (*s == '\0' || *end != '\0') return IS_INCONVERTIBLE;
      return IS_SUCCESS;
    };
    int64_t lresult = 0;
    STR_INT_RET ret = lambda(str.c_str(), lresult);
    if (ret != IS_SUCCESS)
      KEXCEPT(StringException,
              "INT64 conversion failed, reason: " + StringOpHelper::INSTANCE().getStrForRet(ret));
    int64_t result = lresult;
    if (result != lresult) KEXCEPT(StringException, "INT64 conversion failed");
    return result;
  }
};
}  // namespace kul
}  // namespace mkn
#endif /* _MKN_KUL_STRING_HPP_ */
