/**
Copyright (c) 2017, Philip Deegan.
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
#define KUL_FORCE_TRACE

#include <iomanip>
#include "mkn/kul/asio/log.hpp"
#include "mkn/kul/assert.hpp"
#include "mkn/kul/cli.hpp"
#include "mkn/kul/dbg.hpp"
#include "mkn/kul/io.hpp"
#include "mkn/kul/ipc.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/math.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/signal.hpp"
#include "mkn/kul/span.hpp"
#include "mkn/kul/string.hpp"
#include "mkn/kul/threads.hpp"
#include "mkn/kul/time.hpp"
#include "mkn/kul/tuple.hpp"
#include "mkn/kul/wstring.hpp"

namespace mkn {
namespace kul {

class Test;

class TestThreadObject {
 private:
  int i = 0;

 public:
  void print() { KLOG(INF) << "i = " << i; }

 protected:
  void operator()() {
    KLOG(INF) << "THREAD RUNNING";
    i++;
    KLOG(INF) << "THREAD FINISHED";
  }
  void operator()() const {
    KLOG(INF) << "CONST THREAD RUNNING";
    KLOG(INF) << "CONST THREAD FINISHED";
  }
  friend class Test;
  friend class mkn::kul::Thread;
};

class TestThreadQueueObject {
 protected:
  int i = 0;
  Mutex &mutex;

 public:
  TestThreadQueueObject(Mutex &_mutex) : mutex(_mutex) {}
  void operator()() {
    mkn::kul::ScopeLock lock(mutex);
    KLOG(INF) << "THREAD RUNNING";
    i++;
    KLOG(INF) << "THREAD FINISHED";
  }
  void print() { KLOG(INF) << "i = " << i; }
};

class TestThreadQueueQObject : public TestThreadQueueObject {
 private:
  std::queue<int> &q;

 public:
  TestThreadQueueQObject(Mutex &_mutex, std::queue<int> &_q)
      : TestThreadQueueObject(_mutex), q(_q) {}
  void operator()() {
    mkn::kul::ScopeLock lock(mutex);
    KLOG(INF) << "THREAD RUNNING";
    i++;
    q.pop();
    KLOG(INF) << "THREAD FINISHED";
  }
  void print() { KLOG(INF) << "i = " << i; }
};

class TestConcQueueQObject {
 public:
  TestConcQueueQObject() {}
  void operator()() {
    KLOG(INF) << "TestConcQueueQObject RUNNING";
    mkn::kul::this_thread::sleep(10);
  }
};

class TestIPCServer : public mkn::kul::ipc::Server {
 public:
  TestIPCServer() : mkn::kul::ipc::Server("uuid", 1) {}  // UUID     CHECKS ONCE
  void handle(std::string const &s) { KLOG(INF) << "TestIPCServer " << s; }
  void operator()() { listen(); }
};

class TestIPC {
 public:
  void run() {
    TestIPCServer ipc;
    mkn::kul::Thread t(std::ref(ipc));
    t.run();
    mkn::kul::this_thread::sleep(1000);
    mkn::kul::ipc::Client("uuid").send("TestIPCClient");
    t.join();
  }
};

class Catch {
 public:
  void print(const int16_t &s) {
    (void)s;
    KOUT(NON) << "SEGMENTATION FAULT INTERCEPTED";
    KOUT(NON) << "PRINT STACKTRACE";
  }
};

class Test {
 private:
  const std::string s;

 public:
  Test(int argc, char *argv[]) : s("LAMBDAS ALLOWED IN SIGNAL") {
    KUL_DBG_FUNC_ENTER;
    Catch c;
    mkn::kul::Signal sig;  // Windows: each thread requires own handler, static
                           // singleton otherwise so only ever one.
    sig.segv(std::bind(&Catch::print, std::ref(c),
                       std::placeholders::_1));  // Vector of
                                                 // functions to call
                                                 // before exiting -
                                                 // CAUTION! KEEP
                                                 // SIMPLE!
    // Allows lamda notation
    sig.segv([this](int16_t) { KOUT(NON) << s; });

    KERR << "KERR";            // KOUT/KLOG are controlled via env/var KLOG
    KOUT(NON) << "KOUT(NON)";  // KLOG=0 / or unset
    KOUT(INF) << "KOUT(INF)";  // KLOG=1
    KOUT(ERR) << "KOUT(ERR)";  // KLOG=2
    KOUT(DBG) << "KOUT(DBG)";  // KLOG=3
    KLOG(INF) << "KLOG(INF)";  // KLOG=1
    KLOG(ERR) << "KLOG(ERR)";  // KLOG=2
    KLOG(DBG) << "KLOG(DBG)";  // KLOG=3
    KOUT(NON) << mkn::kul::Dir::SEP();
    KOUT(NON) << mkn::kul::env::SEP();
    KOUT(NON) << mkn::kul::env::CWD();
    KOUT(NON) << mkn::kul::user::home().path();
    KLOG(INF) << mkn::kul::user::home("maiken").path();

    mkn::kul::File fo("TEST_WRITE_OUT");
    mkn::kul::File fe("TEST_WRITE_ERR");
    {
      mkn::kul::io::Writer wo(fo);
      mkn::kul::io::Writer we(fe);
      auto lo = [&](std::string const &_s) { wo << _s; };
      auto le = [&](std::string const &_s) { we << _s; };
      mkn::kul::LogMan::INSTANCE().setOut(lo);
      mkn::kul::LogMan::INSTANCE().setErr(le);
      KOUT(INF) << "KOUT(INF)";
      KERR << "KOUT(ERR)";
      // scoped for autoflush - segfault later drops stream
    }
    fo.rm();
    fe.rm();
    mkn::kul::LogMan::INSTANCE().setOut(nullptr);
    mkn::kul::LogMan::INSTANCE().setErr(nullptr);

    {
      mkn::kul::File os_inc("os.ipp", mkn::kul::Dir("test"));
      mkn::kul::File os_hpp("os.hpp", mkn::kul::Dir("inc/kul"));
      if (!os_hpp || !os_inc) KEXCEPTION("UH OH!");
      KLOG(INF) << os_inc.relative(os_hpp);
      KLOG(INF) << os_inc.relative(os_hpp.dir());
      KLOG(INF) << os_inc.dir().relative(os_hpp.dir());
    }

    for (mkn::kul::Dir const &d : mkn::kul::Dir(mkn::kul::env::CWD()).dirs())
      for (mkn::kul::File const &f : d.files()) KOUT(NON) << d.join(f.name());  // or f.full()
    try {
      mkn::kul::Process("echo").arg("Hello").arg("World").start();

    } catch (const mkn::kul::proc::ExitException &e) {
      KERR << e.stack();
      KERR << e.code();
    } catch (const mkn::kul::proc::Exception &e) {
      KERR << e.debug() << " : " << typeid(e).name();
      KERR << "Error expected on windows without echo on path";
    }

    {
      std::vector<mkn::kul::cli::Arg> argV{
          {mkn::kul::cli::Arg('f', "flag"),
           mkn::kul::cli::Arg('m', "maybe_value", mkn::kul::cli::ArgType::MAYBE),
           mkn::kul::cli::Arg('o', "option", mkn::kul::cli::ArgType::STRING)}};
      std::vector<mkn::kul::cli::Cmd> cmdV{{"COMMAND"}};

      mkn::kul::cli::Args args(cmdV, argV);
      try {
        args.process(argc, argv);
      } catch (const mkn::kul::cli::Exception &e) {
        KEXIT(1, e.what());
      }
    }

    for (std::string const &arg :
         mkn::kul::cli::asArgs("/path/to \"words in quotes\" words\\ not\\ in\\ quotes end"))
      KOUT(NON) << "ARG: " << arg;

    for (std::string const &arg : mkn::kul::String::SPLIT("split - by - char - dash", '-'))
      KOUT(NON) << "BIT: " << arg;
    for (std::string const &arg : mkn::kul::String::SPLIT("split - by - string - dash", "-"))
      KOUT(NON) << "BIT: " << arg;

    for (std::string const &arg :
         mkn::kul::String::ESC_SPLIT("split \\- by - char - dash with escape backslash", '-'))
      KOUT(NON) << "BIT: " << arg;

    mkn::kul::hash::map::S2S sparse;
    sparse.insert("LEFT", "RIGHT");

#if defined(_MKN_WITH_GOOGLE_SPARSEHASH_)
    {
      mkn::kul::dense::hash::map::S2S dense;
      dense.setEmptyKey("");  // unique non occuring key
      dense.insert("LEFT", "RIGHT");
    }
#endif

    mkn::kul::File file("./write_access");
    if (file && !file.rm()) KERR << "CANNOT DELETE FILE " << file;
    if (!file && !file.mk()) KERR << "CANNOT CREATE FILE " << file;
    if (file && !file.rm()) KERR << "CANNOT DELETE FILE " << file;

    KOUT(NON) << "mkn::kul::Now::MILLIS(); " << mkn::kul::Now::MILLIS();
    KOUT(NON) << "mkn::kul::Now::MICROS(); " << mkn::kul::Now::MICROS();
    KOUT(NON) << "mkn::kul::Now::NANOS();  " << mkn::kul::Now::NANOS();

    KOUT(NON) << "mkn::kul::DateTime::NOW();  " << mkn::kul::DateTime::NOW();

    KOUT(NON) << "CPU CORES:   " << mkn::kul::cpu::cores();
    KOUT(NON) << "MAX THREADS: " << mkn::kul::cpu::threads();

    {
      mkn::kul::SpanSet<double> spanset{std::vector<size_t>{3, 2, 1}};
      mkn::kul::Span<double> raw = spanset.raw();
      std::vector<size_t> vals{1, 5, 5, 3, 4, 5};
      for (size_t i = 0; i < 6; i++) raw[i] = vals[i];
      for (auto const &span : spanset) {
        KLOG(INF) << span.size();
        for (auto const &d0 : span) KLOG(INF) << d0;
      }
    }

    {
      TestThreadObject tto1;
      mkn::kul::Thread th(std::ref(tto1));
      th.join();
      tto1.print();
      th.join();
      tto1.print();

      TestThreadObject tto2;
      mkn::kul::Thread th2(std::cref(tto2));
      th2.join();
      tto2.print();

      TestThreadObject tto3;
      mkn::kul::Thread th3(tto3);
      th3.join();
      tto3.print();

      mkn::kul::Thread([&tto1]() { tto1(); }).join();
      tto1.print();
      mkn::kul::Thread([tto1]() { tto1(); }).join();
      tto1.print();
    }

    mkn::kul::Mutex mutex;
    {
      { mkn::kul::ScopeLock lock(mutex); }
      mkn::kul::ScopeLock lock(mutex);
    }
    {
      KOUT(NON) << "LAUNCHING THREAD POOL";
      TestThreadQueueObject ttpo1(mutex);
      mkn::kul::ThreadQueue tp1(std::ref(ttpo1));
      tp1.setMax(4);
      tp1.detach();
      tp1.join();
      ttpo1.print();

      std::queue<int> q;
      for (int i = 0; i < 10; i++) q.push(i);
      KOUT(NON) << "LAUNCHING PREDICATED THREAD POOL";
      TestThreadQueueQObject ttpo2(mutex, q);
      mkn::kul::PredicatedThreadQueue<std::queue<int>> tp2(std::ref(ttpo2), q);
      tp2.setMax(mkn::kul::cpu::threads());
      tp2.detach();
      tp2.join();
      ttpo2.print();
    }

    TestConcQueueQObject tcqqo;
    ConcurrentThreadQueue<void()> ctq(5, 1);
    for (size_t i = 0; i < 10; i++)
      ctq.async(std::bind(&TestConcQueueQObject::operator(), std::ref(tcqqo)));
    mkn::kul::this_thread::sleep(500);
    ctq.stop().join();

    {
      mkn::kul::ConcurrentThreadPool<> ctp(5, 1);
      auto lambda = [](unsigned int a, unsigned int b) { KLOG(INF) << (a + b); };
      ctp.async(std::bind(lambda, 2, 4));
      auto lambdb = [](unsigned int a, unsigned int b) {
        KLOG(INF) << (a + b);
        KEXCEPTION("Exceptional!");
      };
      auto lambex = [](const mkn::kul::Exception &e) { KLOG(ERR) << e.stack(); };
      ctp.async(std::bind(lambdb, 2, 4), std::bind(lambex, std::placeholders::_1));
      mkn::kul::this_thread::sleep(500);
      ctp.block().finish().join();
    }

    {
      mkn::kul::ChroncurrentThreadPool<> ctp(5, 1);
      auto lambda = [](unsigned int a, unsigned int b) { KLOG(INF) << (a + b); };
      ctp.async(std::bind(lambda, 2, 4));
      auto lambdb = [](unsigned int a, unsigned int b) {
        KLOG(INF) << (a + b);
        KEXCEPT(mkn::kul::Exception, "Exceptional!");
      };
      auto lambex = [](const mkn::kul::Exception &e) { KLOG(ERR) << e.stack(); };
      ctp.async(std::bind(lambdb, 2, 4), std::bind(lambex, std::placeholders::_1));
      mkn::kul::this_thread::sleep(500);
      ctp.block().finish().join();
    }

    TestIPC().run();

    KOUT(NON) << "Phsical RAM used in KB: " << mkn::kul::this_proc::physicalMemory();
    KOUT(NON) << "Virtual RAM used in KB: " << mkn::kul::this_proc::virtualMemory();
    KOUT(NON) << "Total RAM used in KB:   " << mkn::kul::this_proc::totalMemory();

    KOUT(NON);
    KOUT(NON) << "SEG FAULTING MAIN THREAD!";
    KOUT(NON) << "NON ZERO EXIT CODE EXPECTED";
    KOUT(NON) << "FOR FULL DEBUG INFO BUILD WITH:";
    KOUT(NON) << "\tWINDOWS cl: -Z7 / link: -DEBUG";
    KOUT(NON) << "\tLINUX  gcc: -g";

    //*(int *)0 = 0;  // First seg fault always exits after handling
  }
};
}  // namespace kul
}  // namespace mkn

int main(int argc, char *argv[]) {
  try {
    mkn::kul::Test(argc, argv);
  } catch (const mkn::kul::Exception &e) {
    KERR << e.stack();
  } catch (const std::exception &e) {
    KERR << e.what();
  } catch (...) {
    KERR << "UNKNOWN EXCEPTION CAUGHT";
  }
  return 0;
}