# `mkn/kul/bon.hpp` — Better Object Notation

**Namespace:** `mkn::kul::bon`

JSON without quotes, parsed into a `YAML::Node`. Used by maiken for module arguments on the command
line, e.g. `-m 'cpp.iwyu{compile{fix: --nocomments, system_last: true}}'`.

```cpp
YAML::Node from(std::string const& s);  // throws bon::Exception on malformed input
```

| input | result |
|---|---|
| `{a: b, c: d}` | map `{a: b, c: d}` |
| `{a: {b: c}}` | nested map |
| `{a{b: c}}` | key directly followed by an object, same as `{a: {b: c}}` |
| `{a: {x, y, z}}` | `a` is the list `[x, y, z]` |
| `{a: {x}}` | a single bare entry is a scalar, `a: x` |
| `{url: http://x:80}` | only the first `:` separates key and value |
| `{a: b\, c}` | `\` escapes `,` `:` `{` `}` `\`, `a: "b, c"` |
| `{}`, `{a: {}}` | empty maps |
| `{a: b,}` | trailing commas are allowed |
| `{a: 1, a: 2}` | duplicate keys, the last wins |

Unescaped whitespace around keys and values is trimmed.

`bon::Exception` (derives `mkn::kul::Exception`) is thrown with the position for: missing or extra
braces, input after the closing `}`, an object without a key (`{{a: b}}`), empty keys or entries,
list and map entries mixed in one object, `{` inside a value, and an escape at the end of input.
