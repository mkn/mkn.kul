# `mkn/kul/yaml.hpp` — YAML Parsing

Requires the `parse.yaml` dependency (yaml-cpp).

**Namespace:** `mkn::kul::yaml`

## enum `NodeType`

```cpp
enum NodeType { NON = 0, STRING, LIST, MAP };
```

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, std::uint16_t const& l, std::string const& s);
};
```

## class `NodeValidator`

Describes an expected node in a YAML schema.

```cpp
class NodeValidator {
public:
  NodeValidator(std::string const& name, bool mandatory = false);
  NodeValidator(std::string const& name, std::vector<NodeValidator> const& children,
                bool mandatory, NodeType const& type);

  std::vector<NodeValidator> const& children () const;
  bool                              mandatory() const;
  std::string const&                name     () const;
  NodeType const&                   type     () const;
};
```

## class `Item`

Base for YAML-backed objects.

```cpp
class Item {
public:
  virtual ~Item();
  YAML::Node const& root() const;

  static void VALIDATE(YAML::Node const& n,
                        std::vector<NodeValidator> const& nvs) KTHROW(Exception);
};
```

## class `Validator`

Validates a `YAML::Node` against a schema of `NodeValidator` entries.

```cpp
class Validator {
public:
  Validator(std::vector<NodeValidator> const& children);
  std::vector<NodeValidator> const& children() const;
  void validate(YAML::Node const& n);
};
```

## class `String`

Parse YAML from an in-memory string.

```cpp
class String : public Item {
public:
  String(std::string const& s) KTHROW(Exception);
  YAML::Node const& validate(Validator const&& v) KTHROW(Exception);
};
```

## class `File`

Parse YAML from a file. Subclass and implement `validator()`.

```cpp
class File : public Item {
public:
  template <class T>
  static T CREATE(std::string const& path) KTHROW(Exception);

  virtual ~File();
  std::string const& file() const;
  virtual Validator validator() const = 0;
};
```
