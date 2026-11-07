# mkn::kul — C++ API Reference

**mkn::kul** (`Make Know Utility Library`) is a modern C++20 utility library providing cross-platform abstractions for threading, I/O, processes, logging, containers, and more.

All public symbols live under the `mkn::kul` namespace (or sub-namespaces noted per section).

---

## Table of Contents

1. [Core Definitions (`defs.hpp`)](#1-core-definitions-defshpp)
2. [Exception Handling (`except.hpp`)](doc/mkn/kul/except.md)
3. [Assertions (`assert.hpp`)](doc/mkn/kul/assert.md)
4. [Type Trait Helpers (`decltype.hpp`)](#4-type-trait-helpers-decltypehpp)
5. [Span Containers (`span.hpp`)](doc/mkn/kul/span.md)
6. [Custom Allocators (`alloc.hpp`)](doc/mkn/kul/alloc.md)
7. [Vectors with Custom Allocators (`vector.hpp`)](doc/mkn/kul/vector.md)
8. [Byte Utilities (`byte.hpp`)](doc/mkn/kul/byte.md)
9. [String Utilities (`string.hpp`)](doc/mkn/kul/string.md)
10. [Wide String Utilities (`wstring.hpp`)](doc/mkn/kul/wstring.md)
11. [Floating-Point Utilities (`float.hpp`)](doc/mkn/kul/float.md)
12. [Hashing (`hash.hpp`)](doc/mkn/kul/hash.md)
13. [Hash Maps and Sets (`map.hpp`)](doc/mkn/kul/map.md)
14. [Time and Date (`time.hpp`)](doc/mkn/kul/time.md)
15. [Logging (`log.hpp`)](doc/mkn/kul/log.md)
16. [Threading (`threads.hpp`)](doc/mkn/kul/threads.md)
17. [File I/O (`io.hpp`)](doc/mkn/kul/io.md)
18. [Environment Variables (`env.hpp`)](doc/mkn/kul/env.md)
19. [Filesystem Abstractions (`os.hpp`)](doc/mkn/kul/os.md)
20. [Process Management (`proc.hpp`)](doc/mkn/kul/proc.md)
21. [Signal Handling (`signal.hpp`)](doc/mkn/kul/signal.md)
22. [Command-Line Interface (`cli.hpp`)](doc/mkn/kul/cli.md)
23. [Debugging and Stack Traces (`dbg.hpp`)](doc/mkn/kul/dbg.md)
24. [Compile-Time Iteration (`for.hpp`)](doc/mkn/kul/for.md)
25. [Tuple Utilities (`tuple.hpp`)](doc/mkn/kul/tuple.md)
26. [Aggregate Comparison (`all.hpp`)](doc/mkn/kul/all.md)
27. [Zip Iterator (`zip.hpp`)](doc/mkn/kul/zip.md)
28. [Math Utilities (`math.hpp`)](doc/mkn/kul/math.md)
29. [YAML Parsing (`yaml.hpp`)](doc/mkn/kul/yaml.md)
30. [Better Object Notation (`bon.hpp`)](doc/mkn/kul/bon.md)
31. [Source Control Management (`scm.hpp`)](doc/mkn/kul/scm.md)
32. [CPU Utilities (`cpu.hpp`)](doc/mkn/kul/cpu.md)
33. [Inter-Process Communication (`ipc.hpp`)](doc/mkn/kul/ipc.md)
34. [System Utilities (`sys.hpp`)](doc/mkn/kul/sys.md)

---

## 1. Core Definitions (`defs.hpp`)

Platform detection and utility macros.

### Macros

| Macro | Description |
|-------|-------------|
| `MKN_KUL_STR(x)` | Stringify `x` |
| `MKN_KUL_STR_CAT(x, y)` | Token-paste `x` and `y` |
| `MKN_KUL_PUBLISH` | Mark a symbol for DLL export / `visibility("default")` |
| `MKN_KUL_PRIVATE` | Mark a symbol as `visibility("hidden")` |
| `MKN_KUL_IS_WIN` | `1` on Windows, `0` otherwise |
| `MKN_KUL_IS_NIX` | `1` on Linux, `0` otherwise |
| `MKN_KUL_IS_BSD` | `1` on BSD, `0` otherwise |
| `MKN_KUL_DEBUG_DO(...)` | Expands to `__VA_ARGS__` in debug builds; empty in release |
| `MKN_KUL_DEBUG_DO_ELSE(...)` | Inverse of `MKN_KUL_DEBUG_DO` |

---

## Environment Variables

| Variable | Default | Effect |
|----------|---------|--------|
| `KLOG` | `0` | Log level, number or name: `-1`/`OFF`, `0`/`NON`, `1`/`INF`, `2`/`ERR`, `3`/`DBG`, `4`/`OTH`, `5`/`TRC`. Levels `3`+ are compiled out with `NDEBUG`, see [log](doc/mkn/kul/log.md) |
| `MKN_KUL_GIT_CO` | `--depth 10 --recursive --shallow-submodules` | Extra args to `git clone`, see [scm](doc/mkn/kul/scm.md) |
| `MKN_KUL_SVN_CO` | empty | Extra args to `svn checkout`, see [scm](doc/mkn/kul/scm.md) |

---

## Compile Switches

| Define | Default | OS | Effect |
|--------|---------|----|--------|
| `MKN_KUL_LOG_FRMT` | `"[%M]: %T - %D : %F fn(%N)#%L - %S"` | all | Log line format: `%M` mode, `%T` thread id, `%D` date time, `%F` file, `%N` function, `%L` line, `%S` message, see [log](doc/mkn/kul/log.md) |
| `MKN_KUL_LOG_TIME_FRMT` | `"%Y-%m-%d-%H:%M:%S:%i"` | all | `%D` format for [strftime](https://en.cppreference.com/w/cpp/chrono/c/strftime), `%i` is milliseconds |
| `MKN_KUL_DISABLE_KLOG_DEF` | undefined | all | Do not define the `KLOG*` / `KOUT*` / `KERR` macros |
| `KUL_FORCE_DBG_LOGS` | undefined | all | Keep `KLOG(DBG/OTH/TRC)` with `NDEBUG` |
| `KUL_FORCE_DEBUG_DO` | undefined | all | Keep `MKN_KUL_DEBUG_DO` with `NDEBUG` |
| `MKN_KUL_FORCE_TRACE` | undefined | all | Keep `MKN_KUL_LOG_SCOPE` / function tracing with `NDEBUG`, see [dbg](doc/mkn/kul/dbg.md) |
| `KASSERT_REPLACE_ASSERT` | undefined | all | Replace `assert` with `KASSERT`, see [assert](doc/mkn/kul/assert.md) |
| `MKN_KUL_MAX_PATH` | `PATH_MAX` / `_MAX_PATH` | all | Max path length for filesystem calls |
| `MKN_KUL_PROCESS_BUFFER` | `64000`, Windows `1024` | all | Process output read buffer size, see [proc](doc/mkn/kul/proc.md) |
| `MKN_KUL_PROC_LOOP_NSLEEP` | `1000000` | all | Nanoseconds slept between reads of a running process's output, `0` disables |
| `MKN_KUL_PROC_BLOCK_ERR` | undefined | nix/bsd | Check errors of `fcntl(fd, F_SETFL, O_NONBLOCK)` when starting a process, can fail when starting many processes rapidly |
| `MKN_KUL_PROC_DUP_RETRY` | `3` | nix/bsd | Retries for `dup` in the forked process |
| `MKN_KUL_IPC_UUID_PREFIX` | `"/tmp/pipe"`, Windows `"\\\\.\\pipe\\"` | all | Prefix for IPC pipe paths, see [ipc](doc/mkn/kul/ipc.md) |
| `MKN_KUL_SYS_DLOPEN` | `RTLD_NOW \| RTLD_GLOBAL` | nix/bsd | `dlopen` flags, see [sys](doc/mkn/kul/sys.md) |
| `MKN_WITH_GOOGLE_SPARSEHASH` | undefined | all | Use google sparsehash for `hash::map` / `hash::set`, needs `dep: google.sparsehash`, see [map](doc/mkn/kul/map.md) |
| `MKN_KUL_USE_MKL` / `MKN_KUL_USE_CBLAS` | undefined | all | BLAS backend for [math](doc/mkn/kul/math.md) |

OSX is BSD for the OS column.

---

## Platform Notes

| Header | Windows | Linux | BSD |
|--------|---------|-------|-----|
| `cpu.hpp` | `os/win/cpu.hpp` | `os/nix/cpu.hpp` | `os/bsd/cpu.hpp` |
| `env.hpp` | `os/win/env.hpp` | `os/nixish/env.hpp` | `os/nixish/env.hpp` |
| `ipc.hpp` | `os/win/ipc.hpp` | `os/nixish/ipc.hpp` | `os/nixish/ipc.hpp` |
| `signal.hpp` | `os/win/signal.hpp` | `os/nixish/signal.hpp` | `os/nixish/signal.hpp` |
| `sys.hpp` | `os/win/sys.hpp` | `os/nixish/sys.hpp` | `os/nixish/sys.hpp` |
| `proc.hpp` | `os/win/proc.hpp` | `os/nixish/proc.hpp` + `os/nix/proc.os.hpp` | similar |
| `threads/def.hpp` | `os/win/threads/def.hpp` | `os/nixish/threads/def.hpp` | `os/nixish/threads/def.hpp` |
