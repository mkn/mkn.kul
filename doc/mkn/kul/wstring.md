# `mkn/kul/wstring.hpp` — Wide String Utilities

**Namespace:** `mkn::kul`

## class `WString`

All methods are `static`.

```cpp
class WString {
public:
  static std::string  toString  (std::wstring const& ws);
  static std::wstring toWString (std::string  const& s);

  static void replace   (std::wstring& s, std::wstring const& f, std::wstring const& r);
  static void replaceAll(std::wstring& s, std::wstring const& f, std::wstring const& r);

  static void leftTrim (std::wstring& s, wchar_t const& d = ' ');
  static void rightTrim(std::wstring& s, wchar_t const& d = ' ');
  static void trim     (std::wstring& s);

  static std::vector<std::wstring> split(std::wstring const& s, wchar_t const& d);
  static std::vector<std::wstring> split(std::wstring const& s, std::wstring const& d);
};
```
