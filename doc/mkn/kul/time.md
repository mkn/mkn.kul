# `mkn/kul/time.hpp` — Time and Date

**Namespace:** `mkn::kul::time`

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, int const l, std::string const& s);
};
```

## class `Now`

High-resolution current timestamp. All methods are `static`.

```cpp
class Now {
public:
  static uint64_t MILLIS();   // milliseconds since epoch
  static uint64_t MICROS();   // microseconds since epoch
  static uint64_t NANOS();    // nanoseconds since epoch
};
```

## class `DateTime`

Date/time formatting using `strftime` format strings. All methods are `static`.

```cpp
class DateTime {
public:
  // Format a time_t value
  static std::string const AS(std::time_t const t,
                               std::string f = "%Y-%m-%d-%H:%M:%S");

  // Parse epoch string, then format
  static std::string const AS(std::string const& epoch,
                               std::string const& f = "%Y-%m-%d-%H:%M:%S");

  // Format the current time
  static std::string const NOW(std::string const& f = "%Y-%m-%d-%H:%M:%S");
};
```
