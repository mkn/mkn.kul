# `mkn/kul/threads.hpp` — Threading

**Namespace:** `mkn::kul`, `mkn::kul::this_thread`

Threads and locks are the standard ones (`std::thread`, `std::jthread`, `std::mutex`,
`std::lock_guard`); this header adds a thread pool and a few `this_thread` helpers.

## `this_thread` — current thread

```cpp
namespace mkn::kul::this_thread {
  inline void sleep (unsigned long const& millis);
  inline void uSleep(unsigned long const& micros);
  inline void nSleep(unsigned long const& nanos);

  inline std::string const id();    // thread ID as a string
  inline bool              main();  // true if this is the main thread
  inline void              kill();  // terminate the current thread
}
```

## class `ThreadPool<E>`

A fixed set of worker threads pulling tasks from one queue. Idle workers block on a
condition variable rather than polling, so an idle pool costs nothing whatever the
workload, and a queued task starts as soon as a worker is free.

```cpp
template <class E = mkn::kul::Exception>
class ThreadPool {
public:
  using Task    = std::function<void()>;
  using OnError = std::function<void(E const&)>;

  ThreadPool(std::size_t const& threads = 1, bool const& start = true);
  virtual ~ThreadPool();  // stop() + join()

  // false if blocked or stopped; tasks queued before start() run once started
  bool async(Task&& task, OnError&& onError = OnError());

  ThreadPool& start ();
  ThreadPool& block ();  // stop accepting new tasks
  ThreadPool& finish();  // wait for every queued task, then stop
  ThreadPool& stop  ();  // workers exit after their current task, queue dropped
  ThreadPool& join  ();  // wait for the workers to exit

  bool               running  () const;
  std::exception_ptr exception() const;
  void               rethrow  () const;
};
```

- A task that throws `E` is passed to its `onError`, if given, on the worker thread.
  To abort on the first error, call `stop()` from `onError`.
- Any other exception, or one thrown out of `onError`, is kept (the first one) as the
  pool's `exception()` for `rethrow()`; the pool keeps running.
- `stop()` may be called from a task or `onError`; `finish()` and `join()` must not be.

```cpp
mkn::kul::ThreadPool<> pool(4);
for (auto const& file : files)
  pool.async([&file]() { compile(file); },
             [](mkn::kul::Exception const& e) { KERR << e.stack(); });
pool.finish().join();
pool.rethrow();
```
