# `mkn/kul/dbg.hpp` — Debugging and Stack Traces

**Namespace:** `mkn::kul::dbg`

## class `StackTrace`

Captures the current call stack at construction.

```cpp
class StackTrace {
public:
  StackTrace(std::string const& s);
  StackTrace(StackTrace const&& that);

  std::vector<std::string> const& stack() const;

  // Non-copyable, non-assignable
  StackTrace() = delete;
  StackTrace(StackTrace const&) = delete;
  StackTrace& operator=(StackTrace const&) = delete;
  StackTrace& operator=(StackTrace const&&) = delete;
};
```

## class `FunctionScope`

RAII scope tracker that logs function entry and exit in debug builds.

```cpp
class FunctionScope {
public:
  FunctionScope(char const* file, std::string const& func, uint16_t const& line);
  ~FunctionScope();
};
```

## Macros

| Macro | Description |
|-------|-------------|
| `MKN_KUL_TRACE` | `1` if tracing is active, `0` otherwise |
| `MKN_KUL_TRACE_OR_VOID` | `mkn::kul::dbg::StackTrace` when tracing, `void` otherwise |
| `MKN_KUL_STACK_TRACE` | `return mkn::kul::dbg::StackTrace(__PRETTY_FUNCTION__)` |
| `MKN_KUL_LOG_SCOPE(s)` | Log entry/exit of the current scope with label `s` |
| `MKN_KUL_DBG_FUNC_ENTER` | Log function entry (file, function, line) |
