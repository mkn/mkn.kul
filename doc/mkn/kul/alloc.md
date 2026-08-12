# `mkn/kul/alloc.hpp` — Custom Allocators

**Namespace:** `mkn::kul`

## Free functions

```cpp
void* aligned_alloc(size_t size, size_t alignment);
void  aligned_free (void* p);
```

Platform-neutral aligned allocation/deallocation (uses `_aligned_malloc` on Windows, `posix_memalign` on Unix).

## class `Allocator<T>`

Standard-conforming allocator; base for the other variants.

```cpp
template <typename T>
class Allocator {
public:
  T*   allocate  (size_t n);
  void deallocate(T* p, size_t n);
};
```

## class `NonConstructingAllocator<T>`

Skips default value-initialisation on allocation. Explicit construction still works via `construct`.

```cpp
template <typename T>
class NonConstructingAllocator : public Allocator<T> {
public:
  template <typename U, typename... Args>
  void construct(U* p, Args&&... args);
};
```

## class `AlignedAllocator<T, alignment>`

Allocates memory aligned to `alignment` bytes (default: 32).

```cpp
template <typename T, std::size_t alignment = 32>
class AlignedAllocator {
public:
  T*   allocate  (std::size_t n);
  void deallocate(T* p, std::size_t n);
};
```

## class `HugePageAllocator<T, S>`

Allocates from huge pages of size `S` bytes (default: 4096).

```cpp
template <typename T, std::size_t S = 4096>
class HugePageAllocator { /* standard allocator interface */ };
```

## class `NonConstructingHugePageAllocator<T, S>`

Combines `NonConstructingAllocator` semantics with huge-page backing.
