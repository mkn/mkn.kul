# `mkn/kul/sys.hpp` — System Utilities / Shared Libraries

**Namespace:** `mkn::kul::sys`

Platform-specific implementations are selected automatically (`os/nixish/sys.hpp` using `dlopen`/`dlsym`, `os/win/sys.hpp` using `LoadLibrary`/`GetProcAddress`). The public API is identical across platforms.

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, uint16_t const& l, std::string const& s);
};
```

## class `SharedLibrary`

Loads a shared library (`.so` / `.dll`) at runtime.

```cpp
class SharedLibrary {
public:
  SharedLibrary(mkn::kul::File const& f) KTHROW(Exception);
  ~SharedLibrary();

  mkn::kul::File const file() const;
};
```

## class `SharedFunction<F>`

Resolves a named symbol from a `SharedLibrary` as a function pointer of type `F`.

```cpp
template <class F>
class SharedFunction {
public:
  SharedFunction(SharedLibrary& lib, std::string const& symbol) KTHROW(Exception);
  ~SharedFunction();

  F* pointer();   // raw function pointer; call via (*sf.pointer())(args...)
};
```

## class `SharedClass<T>`

Loads a class from a shared library via named factory and destructor functions.

```cpp
template <class T>
class SharedClass {
public:
  // f: shared library file
  // c: name of the exported factory function  (returns T*)
  // d: name of the exported destructor function (takes T*)
  SharedClass(mkn::kul::File const& f,
              std::string const& c,
              std::string const& d) KTHROW(Exception);
  virtual ~SharedClass();

protected:
  void construct(T*& t) KTHROW(Exception);   // call the factory
  void destruct (T*& t);                     // call the destructor
};
```

**Usage:**
```cpp
mkn::kul::File lib("libfoo.so");
mkn::kul::sys::SharedLibrary sl(lib);

using InitFn = int(const char*);
mkn::kul::sys::SharedFunction<InitFn> fn(sl, "init");
int result = (*fn.pointer())("arg");
```
