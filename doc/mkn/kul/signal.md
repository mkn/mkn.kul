# `mkn/kul/signal.hpp` — Signal Handling

**Namespace:** `mkn::kul`, `mkn::kul::this_thread`

Platform-specific implementations are selected automatically (`os/nixish/signal.hpp` on Unix/BSD, `os/win/signal.hpp` on Windows). The public API is identical across platforms.

## Stack trace helpers

```cpp
namespace mkn::kul::this_thread {
  std::vector<std::string> stacktrace();      // returns demangled call stack
  void                     print_stacktrace(); // prints to stderr
}
```

## class `Signal`

Registers callbacks for process-level signals. Multiple callbacks can be registered per signal; all are invoked.

```cpp
class Signal {
public:
  Signal();

  Signal& abrt(std::function<void(int16_t)> const& f);   // SIGABRT / abort
  Signal& intr(std::function<void(int16_t)> const& f);   // SIGINT  / Ctrl-C
  Signal& segv(std::function<void(int16_t)> const& f);   // SIGSEGV / access violation

  void quiet();   // suppress default signal output (stack trace etc.)
};
```

**Usage:**
```cpp
mkn::kul::Signal sig;
sig.segv([](int16_t s){ /* handle crash */ })
   .intr([](int16_t s){ /* handle Ctrl-C */ });
```
