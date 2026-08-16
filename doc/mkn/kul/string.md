# `mkn/kul/string.hpp` — String Utilities

**Namespace:** `mkn::kul`

## enum `STR_INT_RET`

```cpp
enum STR_INT_RET { IS_SUCCESS = 0, IS_OVERFLOW, IS_UNDERFLOW, IS_INCONVERTIBLE };
```

## class `StringException`

```cpp
class StringException : public mkn::kul::Exception {
public:
  StringException(char const* f, uint16_t const& l, std::string const& s);
};
```

## struct `Between`

```cpp
struct Between {
  std::string remaining;
  std::optional<std::string> found;
  bool error = 0;
};
```

## class `String`

All methods are `static`.

```cpp
class String {
public:
  // In-place modification
  static void REPLACE    (std::string& s, std::string const& f, std::string const& r);
  static void REPLACE_ALL(std::string& s, std::string const& f, std::string const& r);
  static void TRIM_LEFT  (std::string& s, char const& delim = ' ');
  static void TRIM_RIGHT (std::string& s, char const& delim = ' ');
  static void TRIM       (std::string& s);

  template <typename V>
  static void TRIM(V& strs);          // trim all strings in a container

  static void PAD(std::string& s, uint16_t const& p);   // left-pad to width p

  // Splitting
  static std::vector<std::string> SPLIT(std::string const& s, char const& d);
  static void                     SPLIT(std::string const& s, char const& d,
                                        std::vector<std::string>& v);
  static std::vector<std::string> SPLIT(std::string const& s, std::string const& d);
  static void                     SPLIT(std::string const& s, std::string const& d,
                                        std::vector<std::string>& v);

  // Split with escape character
  static std::vector<std::string> ESC_SPLIT(std::string const& s, char const& d,
                                             char const& e = '\\');
  static void                     ESC_SPLIT(std::string const& s, char const& d,
                                             std::vector<std::string>& v,
                                             char const& e = '\\');

  static bool NO_CASE_CMP(std::string a, std::string b);   // case-insensitive compare

  // Extract the substring between the last occurrence of rstr and the first
  // occurrence of lstr before it, removing both delimiters and the extracted
  // text from the remainder. error is set if lstr is found but rstr is not.
  static Between BETWEEN(std::string const& in, std::string lstr, std::string rstr);

  // Line splitting (splits on \n / \r\n)
  static std::vector<std::string> LINES(std::string const& s);
  static void                     LINES(std::string const& s, std::vector<std::string>& v);

  // Type conversion (throw StringException on failure)
  static bool     BOOL  (std::string s);
  static uint16_t UINT16(std::string const& str) KTHROW(StringException);
  static int16_t  INT16 (std::string const& str) KTHROW(StringException);
  static uint32_t UINT32(std::string const& str) KTHROW(StringException);
  static int32_t  INT32 (std::string const& str) KTHROW(StringException);
  static uint64_t UINT64(std::string const& str) KTHROW(StringException);
  static int64_t  INT64 (std::string const& str) KTHROW(StringException);
};
```
