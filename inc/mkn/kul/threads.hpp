/**
Copyright (c) 2024, Philip Deegan.
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
#ifndef _MKN_KUL_THREADS_HPP_
#define _MKN_KUL_THREADS_HPP_

#include "mkn/kul/map.hpp"
#include "mkn/kul/os/threads.hpp"

namespace mkn {
namespace kul {

class ScopeLock {
 private:
  Mutex& m;

 public:
  ScopeLock(Mutex& _m) : m(_m) { this->m.lock(); }
  ~ScopeLock() { this->m.unlock(); }
};

class ThreadQueue {
 protected:
  bool d = 0, f = 0, s = 0;
  uint16_t m = 1;
  mkn::kul::Thread th;
  std::function<void()> func;
  std::vector<std::shared_ptr<mkn::kul::Thread>> ts;
  std::vector<std::exception_ptr> ePs;
  void setStarted() { s = true; }
  virtual void start() KTHROW(std::exception) {
    if (started()) KEXCEPT(Exception, "ThreadQueue is already started");
    setStarted();
    for (uint16_t i = 0; i < m; i++) {
      std::shared_ptr<mkn::kul::Thread> at = std::make_shared<mkn::kul::Thread>(func);
      at->run();
      ts.push_back(at);
      this_thread::nSleep(__MKN_KUL_THREAD_SPAWN_WAIT__);
    }
  }

 public:
  ThreadQueue(std::function<void()> const& _func) : th(std::ref(*this)), func(_func) {}
  template <class T>
  ThreadQueue(const T& t) : th(std::ref(*this)), func(std::bind(&T::operator(), t)) {}
  template <class T>
  ThreadQueue(std::reference_wrapper<T> const& ref)
      : th(std::ref(*this)), func(std::bind(&T::operator(), ref)) {}
  void setMax(int16_t const& max) { m = max; }
  void run() { th.run(); }
  void operator()() { start(); }
  virtual void join() KTHROW(std::exception) {
    th.join();
    while (ts.size()) {
      std::shared_ptr<mkn::kul::Thread> const* del = 0;
      for (auto const& t : ts) {
        if (t->finished()) {
          t->join();
          if (t->exception() != std::exception_ptr()) {
            if (!d) std::rethrow_exception(t->exception());
            ePs.push_back(t->exception());
          }
          del = &t;
          break;
        }
      }
      this_thread::sleep(11);
      if (del) ts.erase(std::remove(ts.begin(), ts.end(), *del), ts.end());
    }
  }
  void detach() {
    d = true;
    th.detach();
  }
  void interrupt() KTHROW(mkn::kul::threading::InterruptionException) {}
  const std::vector<std::exception_ptr> exceptionPointers() { return ePs; }
  bool started() { return s; }
  bool finished() { return f; }
};

template <class P>
class PredicatedThreadQueue : public ThreadQueue {
 private:
  P& p;
  size_t ps;

 protected:
  void start() KTHROW(std::exception) {
    if (started()) KEXCEPT(Exception, "ThreadQueue is already started");
    setStarted();
    size_t c = 0;
    while (c < ps) {
      for (size_t i = ts.size(); i < m && c < ps; i++) {
        c++;
        std::shared_ptr<mkn::kul::Thread> at = std::make_shared<mkn::kul::Thread>(func);
        at->run();
        ts.push_back(at);
        this_thread::nSleep(__MKN_KUL_THREAD_SPAWN_WAIT__);
      }
      std::shared_ptr<mkn::kul::Thread> const* del = 0;
      for (auto const& t : ts) {
        if (t->finished()) {
          t->join();
          if (t->exception() != std::exception_ptr()) {
            if (!d) std::rethrow_exception(t->exception());
            ePs.push_back(t->exception());
          }
          del = &t;
          break;
        }
      }
      this_thread::sleep(11);
      if (del) ts.erase(std::remove(ts.begin(), ts.end(), *del), ts.end());
    }
  }

 public:
  PredicatedThreadQueue(std::function<void()> const& _func, P& pr)
      : ThreadQueue(_func), p(pr), ps(p.size()) {}
  template <class T>
  PredicatedThreadQueue(const T& t, P& pr) : ThreadQueue(t), p(pr), ps(p.size()) {}
  template <class T>
  PredicatedThreadQueue(std::reference_wrapper<T> const& ref, P& pr)
      : ThreadQueue(ref), p(pr), ps(p.size()) {}
};

template <class F, class E = mkn::kul::Exception>
class ConcurrentThreadQueue {
  friend class mkn::kul::Thread;

 protected:
  size_t _cur = 0, _max = 1;
  const uint64_t m_nWait;
  std::atomic<bool> _block, _detatched, _up;
  std::queue<std::unique_ptr<std::pair<std::function<F>, std::function<void(const E&)>>>> _q;
  mkn::kul::hash::map::S2T<std::shared_ptr<mkn::kul::Thread>> _k;
  mkn::kul::hash::map::S2T<std::function<void(const E&)>> _e;

  mkn::kul::Thread _thread;
  mkn::kul::Mutex _mmutex, _qmutex;

  void _KTHROW(std::exception_ptr const& ep, std::function<void(const E&)> const& func) {
    try {
      std::rethrow_exception(ep);
    } catch (const E& e) {
      func(e);
    }
  }
  virtual void operator()() {
    while (_up) {
      this_thread::nSleep(m_nWait);
      {
        mkn::kul::ScopeLock l(_qmutex);
        if (_q.empty()) continue;
      }

      for (; _cur < _max; _cur++) {
        mkn::kul::ScopeLock l(_qmutex);
        auto& f_ptr(_q.front());
        auto& f(*f_ptr.get());
        std::stringstream ss;
        ss << &f;
        auto k(ss.str());
        _k.insert(k, std::make_shared<mkn::kul::Thread>(f.first));
        _e.insert(k, f.second);
        _k[k]->run();
        _q.pop();
      }

      mkn::kul::hash::set::String del;
      for (auto const& t : _k) {
        if (t.second->finished()) {
          t.second->join();
          if (t.second->exception() != std::exception_ptr()) {
            if (_e.count(t.first))
              _KTHROW(t.second->exception(), _e[t.first]);
            else if (!_detatched)
              std::rethrow_exception(t.second->exception());
          }
          del.insert(t.first);
          _cur--;
        }
      }
      for (auto const& s : del) _k.erase(s) && _e.erase(s);
    }
  }

  ConcurrentThreadQueue(ConcurrentThreadQueue const&) = delete;
  ConcurrentThreadQueue(ConcurrentThreadQueue const&&) = delete;
  ConcurrentThreadQueue& operator=(ConcurrentThreadQueue const&) = delete;
  ConcurrentThreadQueue& operator=(ConcurrentThreadQueue const&&) = delete;

 public:
  ConcurrentThreadQueue(size_t const& max = 1, bool strt = 0, uint64_t const& nWait = 1000000)
      : _max(max), m_nWait(nWait), _block(0), _detatched(0), _up(0), _thread(std::ref(*this)) {
    _k.setDeletedKey("");
    _e.setDeletedKey("");
    if (strt) ConcurrentThreadQueue::start();
  }
  virtual ~ConcurrentThreadQueue() {}

  virtual ConcurrentThreadQueue& detach() {
    _detatched = 1;
    return *this;
  }
  virtual ConcurrentThreadQueue& join() {
    _thread.join();
    return *this;
  }
  virtual ConcurrentThreadQueue& stop() {
    _up = 0;
    return *this;
  }
  virtual ConcurrentThreadQueue& interrupt() {
    mkn::kul::ScopeLock l(_mmutex);
    _thread.interrupt();
    return *this;
  }
  virtual ConcurrentThreadQueue& finish(uint64_t const& nWait = 1000000)
      KTHROW(mkn::kul::Exception) {
    while (_up) {
      this_thread::nSleep(nWait);
      {
        mkn::kul::ScopeLock l(_qmutex);
        if (_q.empty()) stop();
      }
    }
    return *this;
  }
  virtual ConcurrentThreadQueue& block() {
    _block = 1;
    return *this;
  }
  virtual ConcurrentThreadQueue& unblock() {
    _block = 0;
    return *this;
  }
  virtual ConcurrentThreadQueue& start() {
    if (!_up) {
      _up = 1;
      _thread.run();
    }
    return *this;
  }

  bool async(std::function<F>&& function,
             std::function<void(const E&)>&& exception = std::function<void(const E&)>()) {
    if (_block) return false;
    {
      auto pair = std::unique_ptr<std::pair<std::function<F>, std::function<void(const E&)>>>(
          new std::pair<std::function<F>, std::function<void(const E&)>>(function, exception));
      mkn::kul::ScopeLock l(_qmutex);
      _q.push(std::move(pair));
    }
    return true;
  }

  std::exception_ptr const& exception() const { return _thread.exception(); }

  void rethrow() {
    if (_thread.exception()) std::rethrow_exception(_thread.exception());
  }
};

template <class E, class PT>
class ConcurrentThreadPool;

class PoolThread {
  template <class E, class PT>
  friend class ConcurrentThreadPool;

 protected:
  const uint64_t m_nWait;
  std::atomic<bool> m_ready, m_run;
  std::function<void()> m_function;
  mkn::kul::Mutex _mutex;

  bool if_ready_set(std::function<void()> const& f) {
    if (!m_ready) return false;
    m_function = f;
    m_ready = 0;
    return true;
  }

  bool ready() {  // be careful!
    return m_ready;
  }

  void stop() { m_run = 0; }

  virtual bool operate() {
    if (m_ready) return false;
    m_function();
    m_function = 0;
    m_ready = 1;
    return true;
  }

 public:
  PoolThread(uint64_t const& nWait = 1000000) : m_nWait(nWait), m_ready(1), m_run(1) {}
  virtual ~PoolThread() {}
  virtual void operator()() {
    while (m_run) {
      this_thread::nSleep(m_nWait);
      operate();
    }
  }
};

template <class E = mkn::kul::Exception, class PT = mkn::kul::PoolThread>
class ConcurrentThreadPool : public ConcurrentThreadQueue<void()> {
 protected:
  mkn::kul::hash::map::S2T<std::shared_ptr<PT>> _p;

  virtual bool operate() {
    bool qEmpty = 0;
    {
      mkn::kul::ScopeLock l(_qmutex);
      qEmpty = _q.empty();
    }
    if (!qEmpty) {
      mkn::kul::ScopeLock l(_qmutex);
      for (size_t i = 0; i < _max; i++) {
        auto const n = std::to_string(i);
        auto& f_ptr(_q.front());
        auto& f(*f_ptr.get());
        if (!_p[n]->if_ready_set(f.first)) continue;
        _e[n] = f.second;
        _q.pop();
        if (_q.empty()) break;
      }
    }

    std::vector<std::string> del;
    {
      if (!_up) return false;
      mkn::kul::ScopeLock l(_mmutex);
      for (auto const& t : _k) {
        if (t.second->started() && t.second->finished()) {
          if (t.second->exception() != std::exception_ptr()) {
            if (_e.count(t.first))
              _KTHROW(t.second->exception(), _e[t.first]);
            else if (!_detatched)
              std::rethrow_exception(t.second->exception());
            del.push_back(t.first);
            _p[t.first]->stop();
          }
        }
      }
    }
    {
      mkn::kul::ScopeLock l(_mmutex);
      if (!_up) return false;
      for (auto const& n : del) {
        _k[n]->join();
        _e.erase(n);
        _k.erase(n);
        _p.erase(n);
        _p.insert(n, std::make_shared<PT>());
        _k.insert(n, std::make_shared<mkn::kul::Thread>(std::ref(*_p[n].get())));
        _k[n]->run();
      }
    }
    return qEmpty;
  }
  virtual void operator()() override {
    while (_up) {
      this_thread::nSleep(m_nWait);
      operate();
    }
  }

  ConcurrentThreadPool(ConcurrentThreadPool const&) = delete;
  ConcurrentThreadPool(ConcurrentThreadPool const&&) = delete;
  ConcurrentThreadPool& operator=(ConcurrentThreadPool const&) = delete;
  ConcurrentThreadPool& operator=(ConcurrentThreadPool const&&) = delete;

 public:
  template<typename...Args>
  ConcurrentThreadPool(size_t const& max = 1, bool strt = 0, uint64_t const& nWait = 1000000, Args&&... args)
      : ConcurrentThreadQueue(max, 0, nWait) {
    for (size_t i = 0; i < max; ++i) {
      auto n = std::to_string(i);
      _p.insert(n, std::make_shared<PT>(nWait, args...));
      _k.insert(n, std::make_shared<mkn::kul::Thread>(std::ref(*_p[n].get())));
    }
    _p.setDeletedKey("");
    if (strt) start();
  }
  virtual ~ConcurrentThreadPool() {
    stop();
    join();
  }
  virtual ConcurrentThreadPool& start() override {
    if (!_up) {
      _up = 1;
      _thread.run();
      for (auto const& t : _k) t.second->run();
    }
    return *this;
  }
  virtual ConcurrentThreadPool& stop() override {
    _up = 0;
    mkn::kul::ScopeLock l(_mmutex);
    for (auto& t : _p) t.second->stop();
    return *this;
  }
  virtual ConcurrentThreadPool& finish(uint64_t const& nWait = 1000000)
      KTHROW(mkn::kul::Exception) override {
    while (_up && !_thread.exception()) {
      this_thread::nSleep(nWait);
      {
        mkn::kul::ScopeLock l1(_qmutex);
        mkn::kul::ScopeLock l2(_mmutex);
        if (_q.empty()) {
          size_t i;
          for (i = 0; i < _max; i++)
            if (!_p[std::to_string(i)]->ready()) break;
          if (i >= _max) {
            _up = 0;
            for (auto& t : _p) t.second->stop();
          }
        }
      }
    }
    return *this;
  }
  virtual ConcurrentThreadPool& interrupt() override {
    mkn::kul::ScopeLock l(_mmutex);
    _thread.interrupt();
    for (auto& t : _k) t.second->interrupt();
    return *this;
  }
  virtual ConcurrentThreadPool& join() override {
    _thread.join();
    mkn::kul::ScopeLock l(_mmutex);
    for (auto& t : _k)
      if (t.second->started()) t.second->join();
    return *this;
  }
  virtual ConcurrentThreadPool& detach() override {
    if (_up) {
      _thread.detach();
      mkn::kul::ScopeLock l(_mmutex);
      for (auto& t : _k) t.second->detach();
    }
    return *this;
  }
};

class AutoChronPoolThread : public PoolThread {
 protected:
  uint64_t m_scale = 0;

 public:
  AutoChronPoolThread(uint64_t const& nWait = 1000000, uint64_t const& scale = 1000)
      : PoolThread(nWait), m_scale(scale) {
    if (m_scale > nWait) KEXCEPTION("Time scale cannot be larger than wait period");
  }
  virtual ~AutoChronPoolThread() {}
  virtual void operator()() override {
    auto nWait(m_nWait / m_scale);
    uint8_t fails = 0;
    while (m_run) {
      this_thread::nSleep(nWait);
      auto run1 = operate();
      fails = (!run1 && fails < m_scale) ? fails + 1 : fails > 0 ? fails - 1 : fails;
      nWait = (m_nWait / (m_scale / (fails + 1)));
    }
  }
};

template <class E = mkn::kul::Exception>
class ChroncurrentThreadPool : public ConcurrentThreadPool<void(), AutoChronPoolThread> {
 protected:
  uint64_t m_scale = 0;

  ChroncurrentThreadPool(ChroncurrentThreadPool const&) = delete;
  ChroncurrentThreadPool(ChroncurrentThreadPool&&) = delete;
  ChroncurrentThreadPool& operator=(ChroncurrentThreadPool const&) = delete;
  ChroncurrentThreadPool& operator=(ChroncurrentThreadPool&&) = delete;

 public:
  ChroncurrentThreadPool(size_t const& max = 1, bool strt = 0, uint64_t const& nWait = 1000000,
                         uint64_t const& scale = 1000)
      : ConcurrentThreadPool(max, 0, nWait, scale), m_scale(scale) {
    if (m_scale > nWait) KEXCEPTION("Time scale cannot be larger than wait period");
    if (strt) start();
  }

  virtual void operator()() override {
    auto nWait(m_nWait / m_scale);
    uint8_t fails = 0;
    while (_up) {
      this_thread::nSleep(nWait);
      auto run1 = operate();
      fails = (!run1 && fails < m_scale) ? fails + 1 : fails > 0 ? fails - 1 : fails;
      nWait = (m_nWait / (m_scale / (fails + 1)));
    }
  }
};

}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_THREADS_HPP_ */
