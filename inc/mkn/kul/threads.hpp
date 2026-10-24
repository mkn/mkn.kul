/**
Copyright (c) 2026, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MKN_KUL_THREADS_HPP_
#define MKN_KUL_THREADS_HPP_

#include "mkn/kul/defs.hpp"
#include "mkn/kul/threads/def.hpp"

#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

namespace mkn::kul {

// A fixed set of worker threads pulling tasks from one queue. Idle workers block on a
// condition variable rather than polling, so an idle pool costs nothing whatever the
// workload, and a queued task starts as soon as a worker is free.
//
// A task that throws `E` is passed to its `onError`, if given, on the worker thread;
// onError may stop() the pool. Any other exception, or one thrown out of `onError`, is
// kept (the first one) as the pool's exception() for rethrow(); the pool keeps running.
template <class E = mkn::kul::Exception>
class ConcurrentThreadPool {
 public:
  using Task = std::function<void()>;
  using OnError = std::function<void(E const&)>;

  ConcurrentThreadPool(std::size_t const& threads = 1, bool const& strt = true)
      : _threads(threads) {
    if (strt) start();
  }
  virtual ~ConcurrentThreadPool() { stop().join(); }

  ConcurrentThreadPool(ConcurrentThreadPool const&) = delete;
  ConcurrentThreadPool(ConcurrentThreadPool&&) = delete;
  ConcurrentThreadPool& operator=(ConcurrentThreadPool const&) = delete;
  ConcurrentThreadPool& operator=(ConcurrentThreadPool&&) = delete;

  // false if the pool is blocked or stopped; tasks queued before start() run once started
  bool async(Task&& task, OnError&& onError = OnError()) {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_blocked) return false;
      _queue.emplace(std::move(task), std::move(onError));
    }
    _work.notify_one();
    return true;
  }

  ConcurrentThreadPool& start() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_run) return *this;
    _run = true;
    _blocked = false;
    for (std::size_t i = 0; i < _threads; ++i) _workers.emplace_back([this]() { work(); });
    return *this;
  }

  // stop accepting new tasks; queued and running ones are unaffected
  ConcurrentThreadPool& block() {
    std::lock_guard<std::mutex> lock(_mutex);
    _blocked = true;
    return *this;
  }

  // waits until every queued task has run (or the pool was stopped), then stops - must not
  // be called from a task
  ConcurrentThreadPool& finish() {
    {
      std::unique_lock<std::mutex> lock(_mutex);
      if (_run) _idle.wait(lock, [this]() { return !_run || (_queue.empty() && !_busy); });
    }
    return stop();
  }

  // stops the workers once their current task is done; queued tasks are dropped.
  // Safe to call from a task or onError, see join() for waiting on the workers.
  ConcurrentThreadPool& stop() {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _run = false;
      _blocked = true;
      _queue = {};
    }
    _work.notify_all();
    _idle.notify_all();
    return *this;
  }

  // waits for the workers to exit, which they do once stopped - must not be called from a task
  ConcurrentThreadPool& join() {
    std::vector<std::thread> workers;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      workers.swap(_workers);
    }
    for (auto& w : workers)
      if (w.joinable()) w.join();
    return *this;
  }

  bool running() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _run;
  }
  std::exception_ptr exception() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _ep;
  }
  void rethrow() const {
    if (auto const ep = exception()) std::rethrow_exception(ep);
  }

 private:
  std::size_t const _threads;
  mutable std::mutex _mutex;
  std::condition_variable _work, _idle;
  std::queue<std::pair<Task, OnError>> _queue;
  std::vector<std::thread> _workers;
  std::exception_ptr _ep;
  std::size_t _busy = 0;
  bool _run = false, _blocked = false;

  void work() {
    while (true) {
      std::pair<Task, OnError> task;
      {
        std::unique_lock<std::mutex> lock(_mutex);
        _work.wait(lock, [this]() { return !_run || !_queue.empty(); });
        if (!_run) return;
        task = std::move(_queue.front());
        _queue.pop();
        ++_busy;
      }
      execute(task);
      {
        std::lock_guard<std::mutex> lock(_mutex);
        --_busy;
      }
      _idle.notify_all();
    }
  }

  void execute(std::pair<Task, OnError>& task) {
    try {
      task.first();
    } catch (E const& e) {
      if (!task.second) return fail(std::current_exception());
      try {
        task.second(e);
      } catch (...) {
        fail(std::current_exception());
      }
    } catch (...) {
      fail(std::current_exception());
    }
  }

  void fail(std::exception_ptr const& ep) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_ep) _ep = ep;
  }
};

}  // namespace mkn::kul

#endif /* MKN_KUL_THREADS_HPP_ */
