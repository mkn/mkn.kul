# mkn::kul API Reference

**mkn::kul** (`Make Know Utility Library`) is a modern C++20 header-only utility library providing cross-platform abstractions for threading, I/O, processes, logging, containers, and more. All public symbols live under the `mkn::kul` namespace (or sub-namespaces as noted in each page).

## Modules

| Header | Module | Description |
|--------|--------|-------------|
| [`alloc.hpp`](alloc.md) | Memory | Custom allocators: standard, non-constructing, aligned, huge-page |
| [`all.hpp`](all.md) | Utilities | Aggregate comparison (`All<>`), `for_each`, `any_of`, `all_of`, `compare_to` |
| [`assert.hpp`](assert.md) | Diagnostics | Runtime assertions with stack-trace capture |
| [`bon.hpp`](bon.md) | Parsing | Better Object Notation — compact notation convertible to YAML |
| [`byte.hpp`](byte.md) | Data | Endianness check, little-endian byte-swap helpers |
| [`cli.hpp`](cli.md) | CLI | Argument parsing (`Args`, `Cmd`, `Arg`), `receive`, `asArgs` |
| [`cpu.hpp`](cpu.md) | System | CPU core/thread count (cross-platform) |
| [`dbg.hpp`](dbg.md) | Diagnostics | Stack trace capture, function-scope RAII, debug macros |
| [`env.hpp`](env.md) | System | Environment variable access: `GET`, `SET`, `EXISTS`, `CWD`, `SEP`, `EOL` |
| [`except.hpp`](except.md) | Diagnostics | Base exception with file/line/cause; `Exit` with exit code |
| [`float.hpp`](float.md) | Math | Floating-point comparison with tolerance |
| [`for.hpp`](for.md) | Meta | Compile-time loops (`for_N`), boolean folds, `generate_from` |
| [`hash.hpp`](hash.md) | Crypto | SHA-256 hashing |
| [`io.hpp`](io.md) | I/O | Text and binary file readers/writers |
| [`ipc.hpp`](ipc.md) | IPC | Inter-process communication: `Server` and `Client` (Unix sockets / Win32 named pipes) |
| [`log.hpp`](log.md) | Logging | Levelled logging (`KLOG`, `KOUT`, `KERR`), pluggable logger manager |
| [`map.hpp`](map.md) | Containers | Hash maps and sets; optional Google sparsehash backend |
| [`math.hpp`](math.md) | Math | `abs`, `pow`, `root`, `product`, `sum` |
| [`os.hpp`](os.md) | Filesystem | `Dir`, `File`, `PushDir`, `fs::TimeStamps` |
| [`proc.hpp`](proc.md) | Processes | `Process`, `AProcess`, `ProcessCapture`, `proc::Call`, `this_proc::*` |
| [`scm.hpp`](scm.md) | SCM | Source control abstraction; `scm::Git` implementation |
| [`signal.hpp`](signal.md) | Signals | `Signal` handler registration; `this_thread::stacktrace` |
| [`span.hpp`](span.md) | Containers | Non-owning `Span<T>`, multi-span `SpanSet<T>` |
| [`string.hpp`](string.md) | Text | `String` utility: split, trim, replace, type conversion |
| [`sys.hpp`](sys.md) | System | Dynamic library loading: `SharedLibrary`, `SharedFunction`, `SharedClass` |
| [`threads.hpp`](threads.md) | Threading | `Thread`, `Mutex`, `ThreadQueue`, `ConcurrentThreadPool`, `this_thread::*` |
| [`time.hpp`](time.md) | Time | `Now::MILLIS/MICROS/NANOS`, `DateTime` formatting |
| [`tuple.hpp`](tuple.md) | Meta | `Pointer<N,T>`, `PointerContainer`, `tuple_from`, `make_pointer_container` |
| [`vector.hpp`](vector.md) | Containers | `Vector<T>` and friends with custom allocators; cross-allocator equality |
| [`wstring.hpp`](wstring.md) | Text | Wide string utilities: convert, trim, split, replace |
| [`yaml.hpp`](yaml.md) | Parsing | YAML parsing with schema validation (requires yaml-cpp) |
| [`zip.hpp`](zip.md) | Containers | `zip()` range adaptor over multiple containers |

## Platform notes

OS-specific implementations are selected at include time. The public API documented here is identical across all supported platforms (Linux, BSD, Windows).

| Feature | Unix/BSD | Windows |
|---------|----------|---------|
| Threads | pthreads | Win32 `HANDLE` |
| Signals | `sigaction` | SEH / `SetConsoleCtrlHandler` |
| IPC | Unix-domain sockets | Named pipes |
| Processes | `fork`/`exec` | `CreateProcess` |
| Shared libraries | `dlopen` / `dlsym` | `LoadLibrary` / `GetProcAddress` |
| CPU info | `/proc/cpuinfo` / sysctl | `GetSystemInfo` |

## Key environment variables

| Variable | Effect |
|----------|--------|
| `KLOG` | Log verbosity: `0`=off `1`=INF `2`=ERR `3`=DBG `4`=OTH `5`=TRC |
| `MKN_KUL_GIT_CO` | Extra args passed to `git clone` during dependency fetch |
