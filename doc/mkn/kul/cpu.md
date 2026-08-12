# `mkn/kul/cpu.hpp` — CPU Information

**Namespace:** `mkn::kul::cpu`

Platform-specific implementations are selected automatically (Linux, BSD, Windows). The public API is identical across platforms.

```cpp
namespace mkn::kul::cpu {
  inline uint32_t cores();    // number of physical cores
  inline uint16_t threads();  // number of logical threads (hardware concurrency)
}
```
