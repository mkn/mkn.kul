# `mkn/kul/threads.hpp` — Threading

**Namespace:** `mkn::kul`, `mkn::kul::this_thread`

Low-level `Thread` and `Mutex` types are provided by platform-specific headers (`os/nixish/threads.os.hpp` using pthreads, `os/win/threads.os.hpp` using Win32 handles). The public API is identical across platforms.

## `this_thread` — current thread

```cpp
namespace mkn::kul::this_thread {
  inline std::string const id();    // thread ID as a string
  inline bool              main();  // true if this is the main thread
  inline void              kill();  // terminate the current thread
}
```

## class `Mutex`

Non-recursive mutex.

```cpp
class Mutex {
public:
  Mutex();
  ~Mutex();

  bool tryLock();   // returns false without blocking if already locked (Unix only)
  void lock();
  void unlock();
};
```

## class `Thread`

Wraps a callable in a joinable/detachable OS thread.

```cpp
class Thread {
public:
  Thread(std::function<void()> const& func);

  template <class T>
  Thread(T const& t);

  template <class T>
  Thread(std::reference_wrapper<T> const& r);

  template <class T>
  Thread(std::reference_wrapper<T const> const& r);

  virtual ~Thread();

  void run    () KTHROW(mkn::kul::threading::Exception);
  void join   ();
  bool detach ();
  void interrupt() KTHROW(mkn::kul::threading::InterruptionException);
};
```

## class `ScopeLock`

RAII mutex guard.

```cpp
class ScopeLock {
public:
  ScopeLock(Mutex& m);
  ~ScopeLock();
};
```

## class `ThreadQueue`

Runs a callable repeatedly on a pool of threads up to a configured maximum.

```cpp
class ThreadQueue {
public:
  ThreadQueue(std::function<void()> const& func);

  template <class T>
  ThreadQueue(T const& t);

  template <class T>
  ThreadQueue(std::reference_wrapper<T> const& ref);

  void setMax(int16_t const& max);   // max concurrent threads (default: 1)

  void run();
  void operator()();

  virtual void join     () KTHROW(std::exception);
  void         detach   ();
  void         interrupt() KTHROW(mkn::kul::threading::InterruptionException);

  std::vector<std::exception_ptr> const& exceptionPointers() const;

  bool started ();
  bool finished();
};
```

## class `PredicatedThreadQueue<P>`

`ThreadQueue` that only spawns additional threads while predicate `P` evaluates to true.

```cpp
template <class P>
class PredicatedThreadQueue : public ThreadQueue {
public:
  PredicatedThreadQueue(std::function<void()> const& func, P& pr);

  template <class T>
  PredicatedThreadQueue(T const& t, P& pr);

  template <class T>
  PredicatedThreadQueue(std::reference_wrapper<T> const& ref, P& pr);
};
```

## class `ConcurrentThreadQueue<F, E>`

A long-running thread that processes work items posted via `async()`.

```cpp
template <class F, class E = mkn::kul::Exception>
class ConcurrentThreadQueue {
public:
  ConcurrentThreadQueue(size_t const& max = 1, bool start = false,
                        uint64_t const& nWait = 1000000);
  virtual ~ConcurrentThreadQueue();

  virtual ConcurrentThreadQueue& start    ();
  virtual ConcurrentThreadQueue& stop     ();
  virtual ConcurrentThreadQueue& join     ();
  virtual ConcurrentThreadQueue& detach   ();
  virtual ConcurrentThreadQueue& interrupt();
  virtual ConcurrentThreadQueue& finish   (uint64_t const& nWait = 1000000)
                                              KTHROW(mkn::kul::Exception);
  virtual ConcurrentThreadQueue& block    ();
  virtual ConcurrentThreadQueue& unblock  ();

  // Post a work item; returns false if the queue is full
  bool async(std::function<F>&& function,
             std::function<void(E const&)>&& exception = {});

  std::exception_ptr const& exception() const;
  void rethrow();
};
```

## class `PoolThread`

Base worker thread for use with `ConcurrentThreadPool`. Override `operator()` for custom behaviour.

```cpp
class PoolThread {
public:
  PoolThread(uint64_t const& nWait = 1000000);
  virtual ~PoolThread();
  virtual void operator()();
};
```

## class `ConcurrentThreadPool<E, PT>`

Thread pool backed by `PoolThread` instances (or a custom subclass).

```cpp
template <class E = mkn::kul::Exception, class PT = mkn::kul::PoolThread>
class ConcurrentThreadPool : public ConcurrentThreadQueue<void()> {
public:
  template <typename... Args>
  ConcurrentThreadPool(size_t const& max = 1, bool start = false,
                       uint64_t const& nWait = 1000000, Args&&... args);
  virtual ~ConcurrentThreadPool();

  virtual ConcurrentThreadPool& start    () override;
  virtual ConcurrentThreadPool& stop     () override;
  virtual ConcurrentThreadPool& finish   (uint64_t const& nWait = 1000000)
                                              KTHROW(mkn::kul::Exception) override;
  virtual ConcurrentThreadPool& interrupt() override;
  virtual ConcurrentThreadPool& join     () override;
  virtual ConcurrentThreadPool& detach   () override;
};
```

## class `AutoChronPoolThread`

`PoolThread` with automatic back-off timing between iterations.

```cpp
class AutoChronPoolThread : public PoolThread {
public:
  AutoChronPoolThread(uint64_t const& nWait = 1000000, uint64_t const& scale = 1000);
  virtual ~AutoChronPoolThread();
  virtual void operator()() override;
};
```

## class `ChroncurrentThreadPool<E>`

Thread pool using `AutoChronPoolThread` workers.

```cpp
template <class E = mkn::kul::Exception>
class ChroncurrentThreadPool : public ConcurrentThreadPool<void(), AutoChronPoolThread> {
public:
  ChroncurrentThreadPool(size_t const& max = 1, bool start = false,
                         uint64_t const& nWait = 1000000,
                         uint64_t const& scale = 1000);
  virtual void operator()() override;
};
```
