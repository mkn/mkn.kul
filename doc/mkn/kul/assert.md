# `mkn/kul/assert.hpp` — Assertions

**Namespace:** `mkn::kul`

## struct `Assert`

Evaluates a condition at runtime; if false, captures a stack trace and aborts.

```cpp
struct Assert {
  template <typename T>
  Assert(T const& t);   // aborts with stack trace if !bool(t)
};
```

## Free functions

```cpp
void inline abort_if    (bool const b);   // abort if b == true
void inline abort_if_not(bool const b);   // abort if b == false
```

## Macros

| Macro | Description |
|-------|-------------|
| `KASSERT(b)` | `mkn::kul::Assert{ (b) }` |
| `assert(b)` | Alias for `KASSERT(b)` when `KASSERT_REPLACE_ASSERT` is defined |
