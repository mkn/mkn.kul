/**
Copyright (c) 2016, Philip Deegan.
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
#include "kul/io.hpp"
#include "kul/os.hpp"
#include "kul/cli.hpp"
#include "kul/ipc.hpp"
#include "kul/log.hpp"
#include "kul/math.hpp"
#include "kul/proc.hpp"
#include "kul/time.hpp"
#include "kul/signal.hpp"
#include "kul/string.hpp"
#include "kul/wstring.hpp"
#include "kul/threads.hpp"

#include "kul/asio/log.hpp"

#include <iomanip>

namespace kul {

class Test;

class TestThreadObject{
    private:
        int i = 0;
    public:
        void print(){ KLOG(INF) << "i = " << i;}
    protected:
        void operator()(){
            KLOG(INF) << "THREAD RUNNING";
            i++;
            KLOG(INF) << "THREAD FINISHED";
        }
        void operator()() const {
            KLOG(INF) << "CONST THREAD RUNNING";
            KLOG(INF) << "CONST THREAD FINISHED";
        }
        friend class Test;
        friend class kul::Thread;
};

class TestThreadQueueObject{
    protected:
        int i = 0;
        Mutex& mutex;
    public:
        TestThreadQueueObject(Mutex& mutex) : mutex(mutex){}
        void operator()(){
            kul::ScopeLock lock(mutex);
            KLOG(INF) << "THREAD RUNNING";
            i++;
            KLOG(INF) << "THREAD FINISHED";
        }
        void print(){ KLOG(INF) << "i = " << i;}
};

class TestThreadQueueQObject : public TestThreadQueueObject{
    private:
        std::queue<int>& q;
    public:
        TestThreadQueueQObject(Mutex& mutex, std::queue<int>& q) : TestThreadQueueObject(mutex), q(q){}
        void operator()(){
            kul::ScopeLock lock(mutex);
            KLOG(INF) << "THREAD RUNNING";
            i++;
            q.pop();
            KLOG(INF) << "THREAD FINISHED";
        }
        void print(){ KLOG(INF) << "i = " << i;}
};

class TestConcQueueQObject{
    public:
        TestConcQueueQObject(){}
        void operator()(){
            KLOG(INF) << "TestConcQueueQObject RUNNING";
            kul::this_thread::sleep(10);
        }
};

class TestIPCServer : public kul::ipc::Server{
    public:
        TestIPCServer() : kul::ipc::Server("uuid", 1){} // UUID     CHECKS ONCE
        void handle(const std::string& s){
            KLOG(INF) << "TestIPCServer " << s;
        }
        void operator()(){
            listen();
        }
};


class TestIPC{
    public:
        void run(){
            TestIPCServer ipc;
            kul::Thread t(std::ref(ipc));
            t.run();
            kul::this_thread::sleep(1000);
            kul::ipc::Client("uuid").send("TestIPCClient");
            t.join();
        }
};

class Catch{
    public:
        void print(const int16_t& s){
            KOUT(NON) << "SEGMENTATION FAULT INTERCEPTED";
            KOUT(NON) << "PRINT STACKTRACE";
        }
};

class Test{
    private:
        const std::string s;
    public:
        Test() : s("LAMBDAS ALLOWED IN SIGNAL"){
            Catch c;
            kul::Signal sig;                                                         // Windows: each thread requires own handler, static singleton otherwise so only ever one.
            sig.segv(std::bind(&Catch::print, std::ref(c), std::placeholders::_1));  // Vector of functions to call before exiting - CAUTION! KEEP SIMPLE!
            sig.segv([this](int16_t){ KOUT(NON) << s; });                            // Allows lamda notation

            KERR << "KERR";
            KOUT(NON) << "KOUT(NON)";
            KOUT(INF) << "KOUT(INF)";
            KOUT(ERR) << "KOUT(ERR)";
            KOUT(DBG) << "KOUT(DBG)";
            KLOG(INF) << "KLOG(INF)";
            KLOG(ERR) << "KLOG(ERR)";
            KLOG(DBG) << "KLOG(DBG)";
            KOUT(NON) << kul::Dir::SEP();
            KOUT(NON) << kul::env::SEP();
            KOUT(NON) << kul::env::CWD();
            KOUT(NON) << kul::user::home().path();
            KLOG(INF) << kul::user::home("maiken").path();

            kul::File fo("TEST_WRITE_OUT");
            kul::File fe("TEST_WRITE_ERR");

            {
                kul::io::Writer wo(fo);
                kul::io::Writer we(fe);
                auto lo = [&](const std::string& s){ wo << s; };
                auto le = [&](const std::string& s){ we << s; };
                kul::LogMan::INSTANCE().setOut(lo);
                kul::LogMan::INSTANCE().setErr(le);

                KOUT(INF) << "KOUT(INF)";
                KERR      << "KOUT(ERR)";
                // scoped for autoflush - segfault later drops stream
            }
            fo.rm();
            fe.rm();

            kul::LogMan::INSTANCE().setOut(nullptr);
            kul::LogMan::INSTANCE().setErr(nullptr);

            // {
            //     kul::asio::Logger logger;
            //     logger.out("ASYNCHRONOUS QUEUED LOGGING");
            // }

            KASIO_OUT(NON) << "ASYNCHRONOUS QUEUED LOGGING";

            for(const kul::Dir& d : kul::Dir(kul::env::CWD()).dirs())
                for(const kul::File& f : d.files())
                    KOUT(NON) << d.join(f.name()); // or f.full()
            try{
                kul::Process("echo").arg("Hello").arg("World").start();

            }catch(const kul::proc::ExitException& e){ 
                KERR << e.stack(); 
                KERR << e.code(); 
            }catch(const kul::proc::Exception& e){
                KERR << e.debug()<< " : " << typeid(e).name();
                KERR << "Error expected on windows without echo on path";
            }

            for(const std::string& arg : kul::cli::asArgs("/path/to \"words in quotes\" words\\ not\\ in\\ quotes end"))
                KOUT(NON) << "ARG: " << arg;

            for(const std::string& arg : kul::String::SPLIT("split - by - char - dash", '-'))
                KOUT(NON) << "BIT: " << arg;
            for(const std::string& arg : kul::String::SPLIT("split - by - string - dash", "-"))
                KOUT(NON) << "BIT: " << arg;

            for(const std::string& arg : kul::String::ESC_SPLIT("split \\- by - char - dash with escape backslash", '-'))
                KOUT(NON) << "BIT: " << arg;

            kul::hash::map::S2S sparse;
            sparse.insert("LEFT", "RIGHT");
            kul::dense::hash::map::S2S dense;
            dense.setEmptyKey(""); // unique non occuring key
            dense.insert("LEFT", "RIGHT");

            kul::File file("./write_access");
            if(file && !file.rm())  KERR << "CANNOT DELETE FILE " << file;
            if(!file && !file.mk()) KERR << "CANNOT CREATE FILE " << file;
            if(file && !file.rm())  KERR << "CANNOT DELETE FILE " << file;

            KOUT(NON) << "kul::Now::MILLIS(); " << kul::Now::MILLIS();
            KOUT(NON) << "kul::Now::MICROS(); " << kul::Now::MICROS();
            KOUT(NON) << "kul::Now::NANOS();  " << kul::Now::NANOS();

            KOUT(NON) << "kul::DateTime::NOW();  " << kul::DateTime::NOW();

            KOUT(NON) << "CPU CORES:   " << kul::cpu::cores();
            KOUT(NON) << "MAX THREADS: " << kul::cpu::threads();

            {
                TestThreadObject tto1;
                kul::Thread th(std::ref(tto1));
                th.join();
                tto1.print();
                th.join();
                tto1.print();

                TestThreadObject tto2;
                kul::Thread th2(std::cref(tto2));
                th2.join();
                tto2.print();

                TestThreadObject tto3;
                kul::Thread th3(tto3);
                th3.join();
                tto3.print();

                kul::Thread([&tto1](){ tto1(); }).join();
                tto1.print();
                kul::Thread([tto1](){ tto1(); }).join();
                tto1.print();
            }

            kul::Mutex mutex;
            {
                {
                    kul::ScopeLock lock(mutex);
                }
                kul::ScopeLock lock(mutex);
            }
            {
                KOUT(NON) << "LAUNCHING THREAD POOL";
                TestThreadQueueObject ttpo1(mutex);
                kul::ThreadQueue tp1(std::ref(ttpo1));
                tp1.setMax(4);
                tp1.detach();
                tp1.join();
                ttpo1.print();

                std::queue<int> q;
                for(int i = 0; i < 10; i++) q.push(i);
                KOUT(NON) << "LAUNCHING PREDICATED THREAD POOL";
                TestThreadQueueQObject ttpo2(mutex, q);
                kul::PredicatedThreadQueue<std::queue<int> > tp2(std::ref(ttpo2), q);
                tp2.setMax(kul::cpu::threads());
                tp2.detach();
                tp2.join();
                ttpo2.print();
            }


            TestConcQueueQObject tcqqo;
            ConcurrentThreadQueue<void()> ctq(5, 1);
            for(size_t i = 0; i < 10; i++) 
                ctq.async(std::bind(&TestConcQueueQObject::operator(), std::ref(tcqqo)));
            kul::this_thread::sleep(500);
            ctq.stop().join();

            {
                kul::ConcurrentThreadPool<> ctp(5, 1);
                auto lambda = [](uint a, uint b){ KLOG(INF) << (a + b); };
                ctp.async(std::bind(lambda, 2, 4));
                auto lambdb = [](uint a, uint b){ KLOG(INF) << (a + b); KEXCEPT(kul::Exception, "Exceptional!"); };
                auto lambex = [](const kul::Exception& e){ KLOG(ERR) << e.stack(); };
                ctp.async(std::bind(lambdb, 2, 4), std::bind(lambex, std::placeholders::_1));
                kul::this_thread::sleep(500);
                ctp.block().finish().join();
            }

            {
                kul::ChroncurrentThreadPool<> ctp(5, 1);
                auto lambda = [](uint a, uint b){ KLOG(INF) << (a + b); };
                ctp.async(std::bind(lambda, 2, 4));
                auto lambdb = [](uint a, uint b){ KLOG(INF) << (a + b); KEXCEPT(kul::Exception, "Exceptional!"); };
                auto lambex = [](const kul::Exception& e){ KLOG(ERR) << e.stack(); };
                ctp.async(std::bind(lambdb, 2, 4), std::bind(lambex, std::placeholders::_1));
                kul::this_thread::sleep(500);
                ctp.block().finish().join();
            }

            TestIPC().run();

            KOUT(NON) << "Phsical RAM used in KB: " << kul::this_proc::physicalMemory();
            KOUT(NON) << "Virtual RAM used in KB: " << kul::this_proc::virtualMemory();
            KOUT(NON) << "Total RAM used in KB:   " << kul::this_proc::totalMemory();

            KOUT(NON);
            KOUT(NON) << "SEG FAULTING MAIN THREAD!";
            KOUT(NON) << "NON ZERO EXIT CODE EXPECTED";
            KOUT(NON) << "FOR FULL DEBUG INFO BUILD WITH:";
            KOUT(NON) << "\tWINDOWS cl: -Z7 / link: -DEBUG";
            KOUT(NON) << "\tLINUX  gcc: -g";

            *(int *) 0 = 0;                                                          // First seg fault always exits after handling
        }
};
}

int main(int argc, char* argv[]){
    try{
        kul::Test();
    }
    catch(const kul::Exception& e){ KERR << e.stack(); } 
    catch(const std::exception& e){ KERR << e.what(); }
    catch(...)                    { KERR << "UNKNOWN EXCEPTION CAUGHT"; }
    return 0;
}