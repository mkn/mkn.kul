# `mkn/kul/env.hpp` — Environment Variables

**Namespace:** `mkn::kul::env`

## enum `Mode`

```cpp
enum Mode { APPE = 0, PREP, REPL };
```

## class `Var`

Describes a named environment variable and how it should be applied.

```cpp
class Var {
public:
  Var(std::string const n, std::string const v, Mode const m);
  Var(Var const& e);

  char const* name () const;
  char const* value() const;
  Mode        mode () const;

  std::string const toString() const;
  Var& operator=(Var&& e);
};
```

## Free functions

The following are available on all platforms (implemented in the platform-specific headers included by `env.hpp`):

```cpp
// Get an environment variable, returning def if not set
template <typename T>
auto GET_AS(std::string const& s, T const& def);

// Raw string access
inline std::string GET(char const* c, std::string default_ = "");

// Set or unset (pass nullptr to unset)
inline void SET(char const* var, char const* val = nullptr);

// Test whether a variable exists / has a non-empty value
inline bool EXISTS(char const* c);

// Current working directory
inline std::string CWD();
inline bool        CWD(std::string const& c);   // change CWD, returns true on success

// Platform path separator (':' on Unix, ';' on Windows)
inline char SEP();

// Platform end-of-line ("\n" on Unix, "\r\n" on Windows)
inline std::string EOL();
```

## Executable search helpers (from `os.hpp`)

```cpp
inline std::string WHERE(char const* c);   // full path of executable on PATH
inline bool        WHICH(char const* c);   // true if executable is on PATH
inline bool        WHICH(std::string const& s);
```
