# `mkn/kul/bon.hpp` — Better Object Notation

**Namespace:** `mkn::kul::bon`

A compact custom notation that can be parsed and converted to a `YAML::Node`.

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, std::size_t const& l, std::string const& s);
};
```

## enum `TYPE`

```cpp
enum class TYPE { OBJ = 0, STR, INT };
```

## struct `BonParsableNode`

Internal parse-tree node.

```cpp
struct BonParsableNode {
  BonParsableNode(BonParsableNode* const parent = nullptr);
  BonParsableNode& emplace_back();   // add a child node
};
```

## class `ob`

Parsed BON object.

```cpp
class ob {
public:
  static ob BUILD(BonParsableNode const& root);
  YAML::Node to_yaml() const;
};
```

## Free function

```cpp
// Parse a BON string and return the equivalent YAML::Node
YAML::Node from(std::string const& s);
```
