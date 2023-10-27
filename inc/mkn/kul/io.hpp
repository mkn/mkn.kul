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
#ifndef _MKN_KUL_IO_HPP_
#define _MKN_KUL_IO_HPP_

#include <time.h>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>

#include "mkn/kul/except.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/string.hpp"

namespace mkn {
namespace kul {
namespace io {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const *f, const size_t &l, std::string const &s) : mkn::kul::Exception(f, l, s) {}
};

class AReader {
 public:
  std::ifstream const &buffer() const { return f; }
  std::ifstream &buffer() { return f; }

  AReader(char const *const path, std::ios_base::openmode const flags) : f(path, flags) {
    if (!f) KEXCEPT(Exception, "FileException : file \"" + std::string(path) + "\" not found");
  }
  virtual ~AReader() {}
  virtual char const *readLine() = 0;
  virtual size_t read(char *c, const size_t &l) = 0;
  virtual void seek(const size_t &l) = 0;
  static void seek(std::ifstream &_f, const size_t &_l) { _f.seekg(_l); }

 protected:
  char const *readLine(std::ifstream &_f) {
    s1.clear();
    if (_f.good()) {
      std::stringstream ss;
      std::istream::sentry sen(_f, true);
      std::streambuf *sb = _f.rdbuf();
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
  size_t read(char *c, std::ifstream &_f, const size_t &l) {
    s1.clear();
    if (_f.good()) {
      std::vector<char> v;
      v.resize(l);
      _f.read(&v[0], l);
      v.resize((size_t)_f.gcount());
      s1 = std::string(v.begin(), v.end());
      std::strcpy(c, s1.c_str());
      return s1.size();
    }
    return 0;
  }
  std::ifstream f;

 private:
  std::string s1;
};
class Reader : public AReader {
 public:
  Reader(char const *c) : AReader(c, std::ios::in) {}
  Reader(File const &c) : Reader(c.full().c_str()) {}
  ~Reader() { f.close(); }
  char const *readLine() { return AReader::readLine(f); }
  size_t read(char *c, const size_t &s) { return AReader::read(c, f, s); }
  void seek(const size_t &l) { AReader::seek(f, l); }
};
class BinaryReader : public AReader {
 private:
 public:
  BinaryReader(char const *c) : AReader(c, std::ios::in | std::ios::binary) {
    f.exceptions(std::ifstream::badbit | std::ifstream::failbit);
  }
  BinaryReader(File const &c) : BinaryReader(c.full().c_str()) {}
  ~BinaryReader() { f.close(); }
  char const *readLine() { return AReader::readLine(f); }
#ifdef _WIN32
  size_t read(char *c, const size_t &s) { return AReader::read(c, f, s); }
#else
  size_t read(char *c, const size_t &s) {
    size_t red = 0;
    try {
      red = f.readsome(c, s);
      KLOG(INF) << red;
    } catch (const std::ios_base::failure &e) {
      KLOG(ERR) << e.what();
    }
    return red;
  }
  size_t read(uint8_t *c, const size_t &s) {
    size_t red = 0;
    try {
      red = f.readsome((char *)c, s);
    } catch (const std::ios_base::failure &e) {
      KLOG(ERR) << e.what();
    }
    return red;
  }
#endif
  void seek(const size_t &s) { AReader::seek(f, s); }
};

class AWriter {
 public:
  virtual ~AWriter() {
    if (f.is_open()) f.close();
  }
  void close() { f.close(); }

  AWriter &write(char const *c, bool nl = false) {
    if (nl)
      f << c << mkn::kul::os::EOL();
    else
      f << c;
    return *this;
  }
  AWriter &write(char const *c, size_t len) {
    f.write(c, len);
    return *this;
  }
  AWriter &write(const uint8_t *c, size_t len) {
    f.write((char const *)(c), len);
    return *this;
  }
  template <class T>
  AWriter &operator<<(const T &s) {
    f << s;
    return *this;
  }
  AWriter &operator<<(std::ostream &(*os)(std::ostream &)) {
    (void)os;
    f << std::flush;
    return *this;
  }
  AWriter &flush() {
    f << std::flush;
    return *this;
  }

 protected:
  std::ofstream f;
};
class Writer : public AWriter {
 public:
  Writer(char const *c, bool a = 0) {
    if (a)
      f.open(c, std::ios::out | std::ios::app);
    else
      f.open(c, std::ios::out);
    if (!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
  }
  Writer(File const &c, bool a = 0) : Writer(c.full().c_str(), a) {}
  ~Writer() {}
};
class BinaryWriter : public AWriter {
 public:
  BinaryWriter(char const *c) {
    f.open(c, std::ios::out | std::ios::binary);
    if (!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
    f.unsetf(std::ios_base::skipws);
  }
  BinaryWriter(File const &c) : BinaryWriter(c.full().c_str()) {}
  ~BinaryWriter() {
    f << std::flush;
    f.close();
  }
};
}  // namespace io
}  // namespace kul
}  // namespace mkn
#endif /* _MKN_KUL_IO_HPP_ */
