# `mkn/kul/scm.hpp` — Source Control Management

**Namespace:** `mkn::kul` / `mkn::kul::scm`

## Exceptions

```cpp
namespace mkn::kul::scm {
  class Exception : public mkn::kul::Exception {
  public:
    Exception(char const* f, uint16_t const& l, std::string const& s);
  };

  class NotFoundException : public mkn::kul::Exception {
  public:
    NotFoundException(char const* f, uint16_t const& l, std::string const& s);
  };
}
```

## class `SCM` (abstract)

```cpp
class SCM {
public:
  virtual ~SCM();
  std::string type();

  // Checkout repo at version v into directory dir
  virtual std::string co(std::string const& dir, std::string const& repo,
                          std::string const& version) const KTHROW(Exception) = 0;

  // Update checkout to version v
  virtual void up(std::string const& dir, std::string const& repo,
                   std::string const& version) const KTHROW(Exception) = 0;

  virtual std::string origin       (std::string const& dir)                            const = 0;
  virtual std::string localVersion (std::string const& dir, std::string const& branch) const = 0;
  virtual std::string remoteVersion(std::string const& url, std::string const& branch)
                                                                   const KTHROW(Exception) = 0;

  virtual bool hasChanges(std::string const& dir)             const = 0;
  virtual void status    (std::string const& dir, bool full = true) const = 0;
  virtual void diff      (std::string const& dir)             const = 0;

  virtual std::string defaultRemoteBranch(std::string const& repo) const = 0;
};
```

## class `scm::Git`

Concrete Git implementation of `SCM`.

```cpp
namespace mkn::kul::scm {
  class Git : public mkn::kul::SCM {
  public:
    std::string defaultRemoteBranch(std::string const& repo) const override;

    std::string branch(mkn::kul::Dir const& dir) const;
    std::string branch(std::string const& dir)   const;

    std::string co(std::string const& dir, std::string const& repo,
                    std::string const& version) const KTHROW(Exception) override;
    void up(std::string const& dir, std::string const& repo,
             std::string const& version) const KTHROW(Exception) override;

    std::string origin       (std::string const& dir)                            const override;
    std::string localVersion (std::string const& dir, std::string const& branch) const override;
    std::string remoteVersion(std::string const& url, std::string const& branch)
                                             const KTHROW(Exception) override;

    bool hasChanges(std::string const& dir)             const override;
    void status    (std::string const& dir, bool full = true) const override;
    void diff      (std::string const& dir)             const override;
  };
}
```
