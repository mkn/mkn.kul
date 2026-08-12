# `mkn/kul/byte.hpp` — Byte Utilities

**Namespace:** `mkn::kul::byte`

## Free function

```cpp
inline bool isBigEndian();
```

## class `LittleEndian`

Byte-swap helpers ensuring little-endian representation.

```cpp
class LittleEndian {
public:
  static uint16_t UINT16(uint16_t i);
  static int16_t  INT16 (int16_t  i);
  static uint32_t UINT32(uint32_t i);
  static int32_t  INT32 (int32_t  i);
  static uint64_t UINT64(uint64_t i);
  static int64_t  INT64 (int64_t  i);
};
```

On a little-endian host these are no-ops; on a big-endian host they byte-swap the value.
