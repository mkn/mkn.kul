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
#ifndef _KUL_IO_HPP_
#define _KUL_IO_HPP_

#include <time.h>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>

#include "kul/except.hpp"
#include "kul/log.hpp"
#include "kul/os.hpp"
#include "kul/string.hpp"

namespace kul {
namespace io {

class Exception : public kul::Exception {
 public:
  Exception(const char *f, const size_t &l, const std::string &s) : kul::Exception(f, l, s) {}
};

class AReader {
 public:
  std::ifstream const &buffer() const { return f; }
  std::ifstream &buffer() { return f; }

  AReader(char const *const path, std::ios_base::openmode const flags) : f(path, flags) {
    if (!f) KEXCEPT(Exception, "FileException : file \"" + std::string(path) + "\" not found");
  }
  virtual ~AReader() {}
  virtual const char *readLine() = 0;
  virtual size_t read(char *c, const size_t &l) = 0;
  virtual void seek(const size_t &l) = 0;
  virtual void seek(std::ifstream &f, const size_t &l) { f.seekg(l); }

 protected:
  const char *readLine(std::ifstream &f) {
    s1.clear();
    if (f.good()) {
      std::stringstream ss;
      std::istream::sentry sen(f, true);
      std::streambuf *sb = f.rdbuf();
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
            if (s1.empty()) f.setstate(std::ios::eofbit);
            return s1.empty() ? 0 : s1.c_str();
          default:
            ss << (char)c;
        }
      }
    }
    return 0;
  }
  size_t read(char *c, std::ifstream &f, const size_t &l) {
    s1.clear();
    if (f.good()) {
      std::vector<char> v;
      v.resize(l);
      f.read(&v[0], l);
      v.resize((size_t)f.gcount());
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
  Reader(const char *c) : AReader(c, std::ios::in) {}
  Reader(const File &c) : Reader(c.full().c_str()) {}
  ~Reader() { f.close(); }
  const char *readLine() { return AReader::readLine(f); }
  size_t read(char *c, const size_t &s) { return AReader::read(c, f, s); }
  void seek(const size_t &l) { AReader::seek(f, l); }
};
class BinaryReader : public AReader {
 private:
 public:
  BinaryReader(const char *c) : AReader(c, std::ios::in | std::ios::binary) {
    f.exceptions(std::ifstream::badbit | std::ifstream::failbit);
  }
  BinaryReader(const File &c) : BinaryReader(c.full().c_str()) {}
  ~BinaryReader() { f.close(); }
  const char *readLine() { return AReader::readLine(f); }
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

  AWriter &write(const char *c, bool nl = false) {
    if (nl)
      f << c << kul::os::EOL();
    else
      f << c;
    return *this;
  }
  AWriter &write(const char *c, size_t len) {
    f.write(c, len);
    return *this;
  }
  AWriter &write(const uint8_t *c, size_t len) {
    f.write((const char *)(c), len);
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
  Writer(const char *c, bool a = 0) {
    if (a)
      f.open(c, std::ios::out | std::ios::app);
    else
      f.open(c, std::ios::out);
    if (!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
  }
  Writer(const File &c, bool a = 0) : Writer(c.full().c_str(), a) {}
  ~Writer() {}
};
class BinaryWriter : public AWriter {
 public:
  BinaryWriter(const char *c) {
    f.open(c, std::ios::out | std::ios::binary);
    if (!f) KEXCEPT(Exception, "FileException : file \"" + std::string(c) + "\" not found");
    f.unsetf(std::ios_base::skipws);
  }
  BinaryWriter(const File &c) : BinaryWriter(c.full().c_str()) {}
  ~BinaryWriter() {
    f << std::flush;
    f.close();
  }
};
}  // namespace io
}  // namespace kul
#endif /* _KUL_IO_HPP_ */
