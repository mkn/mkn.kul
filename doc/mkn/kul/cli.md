# `mkn/kul/cli.hpp` — Command-Line Interface

**Namespace:** `mkn::kul::cli`

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, uint16_t const& l, std::string const& s);
};
```

## class `ArgNotFoundException`

```cpp
class ArgNotFoundException : public Exception {
public:
  ArgNotFoundException(char const* f, uint16_t const& l, std::string const& s);
};
```

## enum `ArgType`

```cpp
enum ArgType { FLAG = 0, STRING, MAYBE };
```

## class `Cmd`

Named command token.

```cpp
class Cmd {
public:
  Cmd(char const* c);
  char const* command() const;
};
```

## class `Arg`

Command-line argument descriptor (single-dash and/or double-dash forms).

```cpp
class Arg : public Cmd {
public:
  Arg(char const d, char const* dd, ArgType t, bool mandatory = false);
  Arg(char const d, char const* dd,            bool mandatory = false);
  Arg(char const* dd,             ArgType t,   bool mandatory = false);
  Arg(char const* dd,                          bool mandatory = false);

  bool           mandatory() const;
  char           dash     () const;    // single-char flag, e.g. 'v'
  char const*    dashdash () const;    // long name, e.g. "verbose"
  ArgType const& type     () const;
};
```

## class `Args`

Parses `argv` against a set of registered commands and arguments.

```cpp
class Args {
public:
  Args();
  Args(std::vector<Cmd> const& cmds, std::vector<Arg> const& args);

  void arg(Arg const& a);
  void cmd(Cmd const& c);

  Cmd const& commands    (char const* c) const;
  Arg const& dashes      (char const  c) const;
  Arg const& doubleDashes(char const* c) const;

  std::vector<Cmd> const& commands () const;
  std::vector<Arg> const& arguments() const;

  std::string const& get(std::string const& s) const;
  bool               has(std::string const& s) const;
  bool               empty()                   const;
  size_t             size()                    const;
  bool               erase(std::string const& key);
  auto&              rest()                    const;   // unrecognised tokens

  void process(uint16_t const& argc, char* argv[],
               uint16_t first = 1) KTHROW(ArgNotFoundException);
};
```

## Free functions

```cpp
// Read a line from stdin with an optional prompt
inline std::string const receive(std::string const& prompt = "");

// Split a shell-style command string into tokens
inline void                       asArgs(std::string const& cmd,
                                         std::vector<std::string>& args);
inline std::vector<std::string>   asArgs(std::string const& cmd);
```
