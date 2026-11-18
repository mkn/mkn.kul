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
#ifndef MKN_KUL_IO_HPP_
#define MKN_KUL_IO_HPP_

#include "mkn/kul/except.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/defs.hpp"
#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <cstdio>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>

namespace mkn {
namespace kul {
namespace io {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, size_t const& l, std::string const& s) : mkn::kul::Exception(f, l, s) {}
};

inline std::streamsize checkedStreamSize(size_t const& l) KTHROW(Exception) {
  if (l > static_cast<size_t>((std::numeric_limits<std::streamsize>::max)()))
    KEXCEPT(Exception, "Length exceeds maximum stream size");
  return static_cast<std::streamsize>(l);
}
inline std::streamoff checkedStreamOff(size_t const& l) KTHROW(Exception) {
  if (l > static_cast<size_t>((std::numeric_limits<std::streamoff>::max)()))
    KEXCEPT(Exception, "Length exceeds maximum stream size");
  return static_cast<std::streamoff>(l);
}

class AReader {
 public:
  std::ifstream const& buffer() const { return f; }
  std::ifstream& buffer() { return f; }

  AReader(std::filesystem::path const& path, std::ios_base::openmode const flags) : f(path, flags) {
    if (!f) KEXCEPT(Exception, "FileException : file \"" + path.string() + "\" not found");
  }
  virtual ~AReader() {}
  virtual char const* readLine() = 0;
  // reads up to l bytes into c, not null terminated, returns the number read
  virtual size_t read(char* c, size_t const& l) = 0;
  virtual void seek(size_t const& l) = 0;
  static void seek(std::ifstream& _f, size_t const& _l) { _f.seekg(checkedStreamOff(_l)); }

 protected:
  char const* readLine(std::ifstream& _f) {
    s1.clear();
    if (_f.good()) {
      std::stringstream ss;
      std::istream::sentry sen(_f, true);
      std::streambuf* sb = _f.rdbuf();
      for (;;) {
        int c = sb->sbumpc();
        switch (c) {
          case '\n':
            return (s1 = ss.str()).c_str();
          case '\r':
            if (sb->sgetc() == '\n') sb->sbumpc();
            return (s1 = ss.str()).c_str();
          case EOF:
            s1 = ss.str();
            if (s1.empty()) _f.setstate(std::ios::eofbit);
            return s1.empty() ? 0 : s1.c_str();
          default:
            ss << (char)c;
        }
      }
    }
    return 0;
  }
  // reads up to l bytes into c, not null terminated, returns the number read
  size_t read(char* c, std::ifstream& _f, size_t const& l) {
    if (!_f.good()) return 0;
    _f.read(c, checkedStreamSize(l));
    return static_cast<size_t>(_f.gcount());
  }
  std::ifstream f;

 private:
  std::string s1;
};
class Reader : public AReader {
 public:
  Reader(std::filesystem::path const& p) : AReader(p, std::ios::in) {}
  template <std::same_as<File> F>
  MKN_KUL_DEPRECATED("use std::filesystem::path")
  Reader(F const& c) : Reader(c.full()) {}
  ~Reader() { f.close(); }
  char const* readLine() { return AReader::readLine(f); }
  size_t read(char* c, size_t const& s) { return AReader::read(c, f, s); }
  void seek(size_t const& l) { AReader::seek(f, l); }
};
class BinaryReader : public AReader {
 private:
 public:
  BinaryReader(std::filesystem::path const& p) : AReader(p, std::ios::in | std::ios::binary) {
    f.exceptions(std::ifstream::badbit | std::ifstream::failbit);
  }
  template <std::same_as<File> F>
  MKN_KUL_DEPRECATED("use std::filesystem::path")
  BinaryReader(F const& c) : BinaryReader(c.full()) {}
  ~BinaryReader() { f.close(); }
  char const* readLine() { return AReader::readLine(f); }
#ifdef _WIN32
  size_t read(char* c, size_t const& s) { return AReader::read(c, f, s); }
#else
  size_t read(char* c, size_t const& s) {
    size_t red = 0;
    try {
      red = f.readsome(c, checkedStreamSize(s));
      KLOG(INF) << red;
    } catch (std::ios_base::failure const& e) {
      KLOG(ERR) << e.what();
    }
    return red;
  }
  size_t read(uint8_t* c, size_t const& s) {
    size_t red = 0;
    try {
      red = f.readsome((char*)c, checkedStreamSize(s));
    } catch (std::ios_base::failure const& e) {
      KLOG(ERR) << e.what();
    }
    return red;
  }
#endif
  void seek(size_t const& s) { AReader::seek(f, s); }
};

class AWriter {
 public:
  virtual ~AWriter() {
    if (f.is_open()) f.close();
  }
  void close() { f.close(); }

  AWriter& write(char const* c, bool nl = false) {
    if (nl)
      f << c << mkn::kul::os::EOL();
    else
      f << c;
    return *this;
  }
  AWriter& write(char const* c, size_t len) {
    f.write(c, checkedStreamSize(len));
    return *this;
  }
  AWriter& write(uint8_t const* c, size_t len) {
    f.write((char const*)(c), checkedStreamSize(len));
    return *this;
  }
  template <class T>
  AWriter& operator<<(T const& s) {
    f << s;
    return *this;
  }
  AWriter& operator<<(std::ostream& (*os)(std::ostream&)) {
    (void)os;
    f << std::flush;
    return *this;
  }
  AWriter& flush() {
    f << std::flush;
    return *this;
  }

 protected:
  std::ofstream f;
};
class Writer : public AWriter {
 public:
  Writer(std::filesystem::path const& p, bool a = 0) {
    f.open(p, a ? std::ios::out | std::ios::app : std::ios::out);
    if (!f) KEXCEPT(Exception, "FileException : file \"" + p.string() + "\" not found");
  }
  template <std::same_as<File> F>
  MKN_KUL_DEPRECATED("use std::filesystem::path")
  Writer(F const& c, bool a = 0) : Writer(c.full(), a) {}
  ~Writer() {}
};
class BinaryWriter : public AWriter {
 public:
  BinaryWriter(std::filesystem::path const& p) {
    f.open(p, std::ios::out | std::ios::binary);
    if (!f) KEXCEPT(Exception, "FileException : file \"" + p.string() + "\" not found");
    f.unsetf(std::ios_base::skipws);
  }
  template <std::same_as<File> F>
  MKN_KUL_DEPRECATED("use std::filesystem::path")
  BinaryWriter(F const& c) : BinaryWriter(c.full()) {}
  ~BinaryWriter() {
    f << std::flush;
    f.close();
  }
};
}  // namespace io
}  // namespace kul
}  // namespace mkn
#endif /* MKN_KUL_IO_HPP_ */
