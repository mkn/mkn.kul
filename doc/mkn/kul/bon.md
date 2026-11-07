# `mkn/kul/bon.hpp` — Better Object Notation

**Namespace:** `mkn::kul::bon`

JSON without quotes, parsed into a `bon::Node`, no dependencies. Used by maiken for module
arguments on the command line, e.g. `-m 'cpp.iwyu{compile{fix: --nocomments, system_last: true}}'`.

```cpp
Node parse(std::string const& s);  // throws bon::Exception on malformed input

struct Node {
  struct Entry { std::string key; Node node; };
  using List = std::vector<Node>;
  using Map  = std::vector<Entry>;  // in input order
  std::variant<Map, List, std::string> value;

  bool is_map() const;  bool is_list() const;  bool is_string() const;
  Map const& map() const;  List const& list() const;  std::string const& str() const;
  std::size_t size() const;  // entries of a map or list, 1 for a string
  bool contains(std::string const& key) const;
  Node const& operator[](std::string const& key) const;
  Node const& operator[](std::size_t i) const;  // list entry
};
```

The accessors throw `bon::Exception` for the wrong type, a missing key or an index out of range.

`mkn/kul/bon/yaml.hpp` converts to yaml-cpp:

```cpp
YAML::Node to_yaml(Node const& node);
YAML::Node from(std::string const& s);  // to_yaml(parse(s))
```

| input | result |
|---|---|
| `{a: b, c: d}` | map `{a: b, c: d}` |
| `{a: {b: c}}` | nested map |
| `{a{b: c}}` | key directly followed by an object, same as `{a: {b: c}}` |
| `{a: {x, y, z}}` | `a` is the list `[x, y, z]` |
| `{a: {x}}` | a single bare entry is a string, `a: x` |
| `{url: http://x:80}` | only the first `:` separates key and value |
| `{a: b\, c}` | `\` escapes `,` `:` `{` `}` `\`, `a: "b, c"` |
| `{}`, `{a: {}}` | empty maps |
| `{a: b,}` | trailing commas are allowed |
| `{a: 1, a: 2}` | duplicate keys, the last wins |

Unescaped whitespace around keys and values is trimmed.

`bon::Exception` (derives `mkn::kul::Exception`) is thrown with the position for: missing or extra
braces, input after the closing `}`, an object without a key (`{{a: b}}`), empty keys or entries,
list and map entries mixed in one object, `{` inside a value, and an escape at the end of input.
