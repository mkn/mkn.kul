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
#ifndef _MKN_KUL_WSTRING_HPP_
#define _MKN_KUL_WSTRING_HPP_

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace mkn {
namespace kul {

class WString {
 public:
  static std::string toString(std::wstring const& ws) { return std::string(ws.begin(), ws.end()); }
  static std::wstring toWString(std::string const& s) { return std::wstring(s.begin(), s.end()); }
  static void replace(std::wstring& s, std::wstring const& f, std::wstring const& r) {
    s = s.substr(0, s.find(f)) + r + s.substr(s.find(f) + f.size());
  }
  static void replaceAll(std::wstring& s, std::wstring const& f, std::wstring const& r) {
    while (s.find(f) < s.size()) replace(s, f, r);
  }
  static void leftTrim(std::wstring& s, wchar_t const& d = ' ') {
    while (s.find(d) == 0) s = s.substr(1);
  }
  static void rightTrim(std::wstring& s, wchar_t const& d = ' ') {
    while (s.rfind(d) == s.size()) s = s.substr(0, s.size() - 2);
  }
  static void trim(std::wstring& s) {
    while (s.find(' ') == 0 || s.find('\t') == 0) s = s.substr(1);
    while (s.rfind(' ') == s.size() - 1 || s.rfind('\t') == s.size() - 1)
      s = s.substr(0, s.size() - 2);
  }
  static std::vector<std::wstring> split(std::wstring const& s, wchar_t const& d) {
    std::vector<std::wstring> ss;
    std::wstring l = s;
    uint16_t pos = 0;
    while ((pos = s.find(d, pos + 1)) < l.size()) {
      ss.push_back(l.substr(0, pos));
      l = l.substr(pos + 1);
    }
    ss.push_back(l);
    return ss;
  }
  static std::vector<std::wstring> split(std::wstring const& s, std::wstring const& d) {
    std::vector<std::wstring> ss;
    std::wstring l = s;
    uint16_t pos = 0;
    while ((pos = s.find(d, pos + 1)) < l.size()) {
      ss.push_back(l.substr(0, pos));
      l = l.substr(pos + 1);
    }
    ss.push_back(l);
    return ss;
  }
};
}  // namespace kul
}  // namespace mkn
#endif /* _MKN_KUL_WSTRING_HPP_ */
