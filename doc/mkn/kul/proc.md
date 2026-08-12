# `mkn/kul/proc.hpp` — Process Management

**Namespaces:** `mkn::kul::this_proc`, `mkn::kul::proc`, `mkn::kul`

## `this_proc` — current process

Available on all platforms:

```cpp
namespace mkn::kul::this_proc {
  int32_t  id();                  // PID of the current process
  void     kill(uint32_t const& exit_code);

  uint64_t virtualMemory ();      // bytes
  uint64_t physicalMemory();      // bytes
  uint64_t totalMemory   ();      // bytes
  uint16_t cpuLoad       ();      // percentage 0–100
}
```

## class `proc::Exception`

```cpp
namespace mkn::kul::proc {
  class Exception : public mkn::kul::Exception {
  public:
    Exception(char const* f, uint16_t const& l, std::string const& s);
  };
}
```

## class `proc::ExitException`

```cpp
class ExitException : public mkn::kul::proc::Exception {
public:
  ExitException(char const* f, uint16_t const& l, short ec, std::string const& s);
  short const& code() const;
};
```

## class `proc::Call`

Synchronous subprocess invocation with optional environment overrides and working directory.

```cpp
namespace mkn::kul::proc {
  class Call {
  public:
    Call(std::string const& cmd, std::string const& workdir = "");
    Call(std::string const& cmd, mkn::kul::hash::map::S2S const& envVars,
         std::string const& workdir = "");
    ~Call();

    int run();   // returns exit code
  };
}
```

## class `AProcess`

Base class for launched subprocesses. Platform-specific `Process` derives from this.

```cpp
class AProcess {
public:
  template <class T>
  AProcess& arg(T const& a);
  AProcess& arg (std::string const& a);
  AProcess& args(std::string const& a);   // split and add multiple args
  AProcess& var (std::string const& n, std::string const& v);   // set env var
  AProcess& set (std::vector<mkn::kul::env::Var> const& in);

  virtual void        start   () KTHROW(mkn::kul::Exception);
  virtual std::string toString() const;

  int32_t const& pid      () const;
  bool           started  () const;
  bool           finished () const;
  int32_t const& exitCode ();

  AProcess& operator<<(std::string const& arg);

  void setOut(std::function<void(std::string const&)> o);
  void setErr(std::function<void(std::string const&)> e);
};
```

## class `Process`

Concrete subprocess (platform-provided). Inherits all `AProcess` methods.

```cpp
class Process : public mkn::kul::AProcess {
public:
  Process(std::string const& cmd, bool const& waitForExit = true);
  Process(std::string const& cmd, std::string const& workdir,
          bool const& waitForExit = true);
  Process(std::string const& cmd, mkn::kul::Dir const& workdir,
          bool const& waitForExit = true);

  bool kill(int16_t signal = 6);
};
```

## class `ProcessCapture`

Captures stdout and stderr from an `AProcess`.

```cpp
class ProcessCapture {
public:
  ProcessCapture();
  ProcessCapture(AProcess& p);
  ProcessCapture(ProcessCapture const& pc);
  virtual ~ProcessCapture();

  std::string const outs() const;   // captured stdout
  std::string const errs() const;   // captured stderr

  void setProcess(AProcess& p);

protected:
  virtual void out(std::string const& s);
  virtual void err(std::string const& s);
};
```
