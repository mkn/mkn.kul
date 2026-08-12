# `mkn/kul/log.hpp` — Logging

**Namespace:** `mkn::kul` / `mkn::kul::log`

## enum `log::mode`

```cpp
namespace mkn::kul::log {
  enum mode { OFF = -1, NON = 0, INF, ERR, DBG, OTH, TRC };
}
```

## class `log::Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, uint16_t const& l, std::string const& s);
};
```

## class `Logger`

Low-level logger with replaceable output/error sinks.

```cpp
class Logger {
public:
  virtual ~Logger();

  void str(char const* f, char const* fn, uint16_t const& l,
           std::string const& s, log::mode const& m, std::string& out_str);

  virtual void err(std::string const& s);
  virtual void out(std::string const& s);

  void log(char const* f, char const* fn, uint16_t const& l,
           std::string const& s, log::mode const& m);

  void setOut(std::function<void(std::string const&)> o);
  void setErr(std::function<void(std::string const&)> e);
};
```

## class `ALogMan`

Abstract logger manager; subclass or use the concrete `LogMan` singleton.

```cpp
class ALogMan {
public:
  virtual ~ALogMan();

  void setMode(log::mode const& m);

  bool inf();   // true if INF messages are active
  bool err();
  bool dbg();

  void log(char const* f, char const* fn, uint16_t const& l,
           log::mode const& m, std::string const& s);

  void out(log::mode const& m, std::string const& s);
  void err(std::string const& s);

  std::string str(char const* f, char const* fn, uint16_t const& l,
                  log::mode const& m,
                  std::string const& s   = "",
                  std::string const  fmt = /* __MKN_KUL_LOG_FRMT__ */);

  void setOut(std::function<void(std::string const&)> o);
  void setErr(std::function<void(std::string const&)> e);
};
```

## class `LogMan`

Concrete singleton.

```cpp
class LogMan : public ALogMan {
public:
  static LogMan& INSTANCE();
};
```

## Logging macros

```cpp
// Through LogMan — respects active log level
KLOG(NON) << "message";
KLOG(INF) << "message";
KLOG(ERR) << "message";
KLOG(DBG) << "message";   // debug builds only
KLOG(OTH) << "message";   // debug builds only
KLOG(TRC) << "message";   // debug builds only

// Direct output — bypasses level check
KOUT(NON) << "message";
KOUT(INF) << "message";
KOUT(ERR) << "message";
KOUT(DBG) << "message";
KOUT(OTH) << "message";
KOUT(TRC) << "message";

// Direct stderr
KERR << "message";
```

## Log format placeholders

Configurable via `__MKN_KUL_LOG_FRMT__`:

| Placeholder | Value |
|-------------|-------|
| `%M` | Mode string |
| `%T` | Thread ID |
| `%D` | Date/time |
| `%F` | Source file |
| `%N` | Function name |
| `%L` | Line number |
| `%S` | Message text |

## Environment variable

`KLOG` — sets active log level at runtime: `0`=off, `1`=INF, `2`=ERR, `3`=DBG, `4`=OTH, `5`=TRC.
