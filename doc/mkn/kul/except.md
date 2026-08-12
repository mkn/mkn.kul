# `mkn/kul/except.hpp` — Exception Handling

**Namespace:** `mkn::kul`

## Macros

| Macro | Expands to |
|-------|-----------|
| `KEXCEPT(e, m)` | `throw e(__FILE__, __LINE__, m)` |
| `KEXCEPTSTR(e)` | `throw e(__FILE__, __LINE__, "")` |
| `KEXCEPTION(m)` | `throw Exception(__FILE__, __LINE__, m)` |
| `KEXCEPSTREAM` | `throw Exception(__FILE__, __LINE__, "")` — chain via `<<` |
| `KEXIT(e, m)` | `throw mkn::kul::Exit(__FILE__, __LINE__, m, e)` |
| `KNOTHROW` | `noexcept(true)` (or empty, platform-dependent) |
| `KTHROW(x)` | `noexcept(false)` (or empty, platform-dependent) |

## class `Exception`

Base exception carrying source location, message, and optional nested cause.

```cpp
class Exception : public std::runtime_error {
public:
  Exception(char const* f, uint16_t const& l, std::string const& s = "");
  Exception(Exception const& e);
  Exception(Exception const&& e);
  virtual ~Exception() KNOTHROW;

  std::string        debug() const;          // "file:line: message"
  char const*        what()  const noexcept override;
  std::string        str()   const noexcept;
  char const*        file()  const;
  uint16_t const&    line()  const;
  std::exception_ptr const& cause() const;
  std::string const  stack() const;

  template <class T>
  Exception& operator<<(T const& s);        // append to message
};

std::ostream& operator<<(std::ostream& s, Exception const& e);
```

## class `Exit`

Exception carrying an integer exit code, for controlled program termination.

```cpp
class Exit : public Exception {
public:
  Exit(char const* f, uint16_t const& l, std::string const& s, uint16_t const& e);
  Exit(Exit const& e);

  uint16_t const& code() const;
};
```
