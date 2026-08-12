# `mkn/kul/io.hpp` — File I/O

**Namespace:** `mkn::kul::io`

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, size_t const& l, std::string const& s);
};
```

## class `AReader` (abstract)

```cpp
class AReader {
public:
  std::ifstream const& buffer() const;
  std::ifstream&       buffer();

  AReader(char const* const path, std::ios_base::openmode flags);
  virtual ~AReader();

  virtual char const* readLine() = 0;
  virtual size_t      read(char* c, size_t const& l) = 0;
  virtual void        seek(size_t const& l) = 0;

  static void seek(std::ifstream& f, size_t const& l);
};
```

## class `Reader`

Text file reader.

```cpp
class Reader : public AReader {
public:
  Reader(char const* c);
  Reader(File const& c);
  ~Reader();

  char const* readLine();
  size_t      read(char* c, size_t const& s);
  void        seek(size_t const& l);
};
```

## class `BinaryReader`

Binary file reader.

```cpp
class BinaryReader : public AReader {
public:
  BinaryReader(char const* c);
  BinaryReader(File const& c);
  ~BinaryReader();

  char const* readLine();
  size_t      read(char*    c, size_t const& s);
  size_t      read(uint8_t* c, size_t const& s);
  void        seek(size_t const& s);
};
```

## class `AWriter` (abstract)

```cpp
class AWriter {
public:
  virtual ~AWriter();
  void close();

  AWriter& write(char const*    c, bool nl = false);
  AWriter& write(char const*    c, size_t len);
  AWriter& write(uint8_t const* c, size_t len);

  template <class T>
  AWriter& operator<<(T const& s);

  AWriter& operator<<(std::ostream& (*os)(std::ostream&));   // e.g. std::endl
  AWriter& flush();
};
```

## class `Writer`

Text file writer.

```cpp
class Writer : public AWriter {
public:
  Writer(char const* c, bool append = false);
  Writer(File const& c, bool append = false);
  ~Writer();
};
```

## class `BinaryWriter`

Binary file writer.

```cpp
class BinaryWriter : public AWriter {
public:
  BinaryWriter(char const* c);
  BinaryWriter(File const& c);
  ~BinaryWriter();
};
```
