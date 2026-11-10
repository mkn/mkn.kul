# mkn.kul

**Cross platform C++20 wrapper for threading/process management/io/logging/signal handling and stack walking.**

Header-only, all symbols are under `mkn::kul`.

## Documentation

- [API.md](API.md): API reference, one page per header under [doc/mkn/kul](doc/mkn/kul/README.md)
- [Environment variables and compile switches](API.md#environment-variables)
- [tst/usage.cpp](tst/usage.cpp): usage of most headers in one file

## Example

```cpp
#include "mkn/kul/log.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"

int main() {
  mkn::kul::Dir dir{"."};
  for (auto const& file : dir.files()) KLOG(INF) << file.name();  // printed when KLOG=1 or higher

  mkn::kul::Process p{"echo"};
  p << "hello";
  p.start();
}
```

## Usage

In a project `mkn.yaml`:

```yaml
dep: mkn.kul
```

Build and run the tests and the usage example:

```bash
mkn clean build run -dtKOp test,usage -W 9
```

## Platforms

| Architecture | OS |
|---|---|
| x86_64 | Linux |
| x86_64 | BSD |
| x86_64 | Windows |
| ARMv7 | Linux |
| ARMv7 | Windows |
| ARM64 | Linux (TBC - should work, stack walking might not) |
