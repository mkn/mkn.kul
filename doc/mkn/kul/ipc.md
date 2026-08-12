# `mkn/kul/ipc.hpp` — Inter-Process Communication

**Namespace:** `mkn::kul::ipc`

Platform-specific implementations are selected automatically (`os/nixish/ipc.hpp` on Unix/BSD using Unix-domain sockets, `os/win/ipc.hpp` on Windows using named pipes). The public API is identical across platforms.

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, uint16_t const& l, std::string const& s);
};
```

## class `Server`

Listens for a single client connection, receives a message, optionally responds.

```cpp
class Server {
public:
  // Listen on an auto-assigned port; UUID is derived from PID
  Server(int16_t const& listenPort = -1) KTHROW(Exception);

  // Listen with an explicit UUID identifier
  Server(std::string const& uuid, int16_t const& listenPort = -1) KTHROW(Exception);

  virtual ~Server();

  // Block until a client connects and sends a message
  void listen() KTHROW(Exception);

protected:
  // Called with each received message; override to process it
  virtual void handle(std::string const& s);

  // Send a response back to the connected client
  void respond(std::string const& s);
};
```

## class `Client`

Connects to a `Server` and sends a message.

```cpp
class Client {
public:
  // Connect to server identified by UUID string
  Client(std::string const& uuid) KTHROW(Exception);

  // Connect to server identified by its PID
  Client(int16_t const& pid) KTHROW(Exception);

  virtual ~Client();

  virtual void send(std::string const& m) const KTHROW(Exception);
};
```

**Usage:**
```cpp
// Server side
struct MyServer : mkn::kul::ipc::Server {
  void handle(std::string const& msg) override {
    respond("ack");
  }
};
MyServer srv;
srv.listen();

// Client side
mkn::kul::ipc::Client client(mkn::kul::this_proc::id());
client.send("hello");
```
