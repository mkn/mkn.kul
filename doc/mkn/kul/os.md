# `mkn/kul/os.hpp` — Filesystem Abstractions

**Namespace:** `mkn::kul` / `mkn::kul::fs`

## class `fs::Exception`

```cpp
namespace mkn::kul::fs {
  class Exception : public mkn::kul::Exception {
  public:
    Exception(char const* f, uint16_t const& l, std::string const& s);
  };
}
```

## class `fs::TimeStamps`

```cpp
class TimeStamps {
public:
  uint64_t const& accessed() const;
  uint64_t const& created () const;
  uint64_t const& modified() const;
};
```

## class `fs::Item` (abstract)

```cpp
class Item {
public:
  virtual ~Item();
  virtual fs::TimeStamps timeStamps() const = 0;
  virtual std::string    esc ()       const = 0;   // shell-escaped path
  virtual std::string    real()       const = 0;   // resolved real path
  virtual std::string    mini()       const = 0;   // minimal relative path
};
```

## class `Dir`

```cpp
class Dir : public fs::Item {
public:
  Dir();
  Dir(char const* p,        bool make = false) KTHROW(fs::Exception);
  Dir(std::string const& p, bool make = false) KTHROW(fs::Exception);
  Dir(Dir const& d);
  Dir(std::string const& s, Dir const& d);   // subdirectory of d named s

  bool cp(Dir const& d) const;   // copy into d
  bool mv(Dir const& d) const;   // move into d
  void rm()             const;   // remove recursively
  bool is()             const;   // exists?
  bool mk()             const;   // create (mkdir -p)
  bool root()           const;   // is filesystem root?

  std::string const join(std::string const& s) const;   // path + sep + s
  std::string const name()                     const;   // basename
  std::string const& path()                    const;   // full path

  std::string esc ()  const override;
  std::string escr()  const;   // shell-escaped, resolved
  std::string escm()  const;   // shell-escaped, minimal
  std::string locl()  const;   // OS-native separator form
  std::string real()  const override;
  std::string mini()  const override;

  fs::TimeStamps timeStamps() const override;

  Dir parent() const;

  std::vector<Dir>  dirs (bool incHidden  = false) const KTHROW(fs::Exception);
  std::vector<File> files(bool recursive  = false) const KTHROW(fs::Exception);

  static std::string JOIN(std::string const& a, std::string const& b);
  static std::string REAL(std::string const& s)        KTHROW(fs::Exception);
  static std::optional<std::string> REAL_OR_NULL(std::string const& s);
  static std::string SEP();   // OS path separator

  std::string relative(mkn::kul::Dir const& r) const;

  Dir& operator=(Dir const& d);
  bool operator==(Dir const& d) const;
  explicit operator bool() const;   // same as is()
};
```

## class `File`

```cpp
class File : public fs::Item {
public:
  File();
  File(std::string const& n, bool make = false);
  File(char const* n,        bool make = false);
  File(std::string const& n, Dir const& d);
  File(std::string const& n, char const* c);
  File(std::string const& n, std::string const& d);
  File(File const& f);

  bool cp(Dir  const& d) const;   // copy into directory
  bool cp(File const& f) const;   // copy to file path
  bool is()              const;   // exists?
  bool mk()              const;   // create (touch)
  bool rm()              const;   // delete
  bool mv(File const& f) const;
  bool mv(Dir  const& d) const;

  std::string const& name() const;   // filename without directory

  std::string esc ()  const override;
  std::string escr()  const;
  std::string escm()  const;
  std::string full()  const;    // full path including directory
  std::string real()  const override;
  std::string mini()  const override;

  uint64_t size() const;         // file size in bytes

  Dir const& dir()          const;
  fs::TimeStamps timeStamps() const override;

  std::string relative(mkn::kul::Dir  const& r) const;
  std::string relative(mkn::kul::File const& r) const;

  File& operator=(File const& f) = default;
  bool  operator==(File const& f) const;
  explicit operator bool() const;   // same as is()
};
```

## class `os::PushDir`

RAII directory change — saves and restores the working directory.

```cpp
namespace mkn::kul::os {
  class PushDir {
  public:
    PushDir(mkn::kul::Dir const& d);
    PushDir(std::string const& d);
    ~PushDir();   // restores previous working directory
  };
}
```
