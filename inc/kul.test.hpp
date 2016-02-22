/**
Copyright (c) 2013, Philip Deegan.
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
#ifndef _KUL_TEST_HPP_
#define _KUL_TEST_HPP_

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

#include <iomanip>

namespace kul {
class TestThreadObject{
    private:
        int i = 0;
    public:
        void print(){ KLOG(INF) << "i = " << i;}
        friend class kul::ThreadCopy<TestThreadObject>;
        friend class kul::ThreadRef<TestThreadObject>;
    protected:
        void operator()(){
            KLOG(INF) << "THREAD RUNNING";
            i++;
            KLOG(INF) << "THREAD FINISHED";
        }
    public:
        void operator()() const {
            KLOG(INF) << "CONST THREAD RUNNING";
            KLOG(INF) << "CONST THREAD FINISHED";
        }
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
            kul::Ref<TestIPCServer> ref(ipc);
            kul::Thread t(ref);
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
            for(const kul::Dir& d : kul::Dir(kul::env::CWD()).dirs())
                for(const kul::File& f : d.files())
                    KOUT(NON) << d.join(f.name()); // or f.full()
            try{
                kul::Process("echo").arg("Hello").arg("World").start();
            }catch(const kul::proc::Exception& e){
                KERR << e.debug()<< " : " << typeid(e).name();
                KERR << "Error expected on windows without echo on path";
            }

            for(const std::string& arg : kul::cli::asArgs("/path/to \"words in quotes\" words\\ not\\ in\\ quotes end"))
                KOUT(NON) << "ARG: " << arg;

            for(const std::string& arg : kul::String::split("split - by - char - dash", '-'))
                KOUT(NON) << "BIT: " << arg;
            for(const std::string& arg : kul::String::split("split - by - string - dash", "-"))
                KOUT(NON) << "BIT: " << arg;

            for(const std::string& arg : kul::String::escSplit("split \\- by - char - dash with escape backslash", '-'))
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

            TestThreadObject tto1;
            kul::Ref<TestThreadObject> ref(tto1);
            kul::Thread th(ref);
            th.detach();
            th.join();
            tto1.print();
            th.join();
            tto1.print();

            TestThreadObject tto2;
            kul::Ref<const TestThreadObject> cref(tto2);
            kul::Thread th2(cref);
            th2.detach();
            th2.join();
            tto2.print();

            TestThreadObject tto3;
            kul::Thread th1(tto3);
            th1.detach();
            th1.join();
            tto3.print();

            kul::Mutex mutex;
            {
                {
                    kul::ScopeLock lock(mutex);
                }
                kul::ScopeLock lock(mutex);
            }

            KOUT(NON) << "LAUNCHING THREAD POOL";
            TestThreadQueueObject ttpo1(mutex);
            kul::Ref<TestThreadQueueObject> ref2(ttpo1);
            kul::ThreadQueue tp1(ref2);
            tp1.setMax(4);
            tp1.detach();
            tp1.join();
            ttpo1.print();

            std::queue<int> q;
            for(int i = 0; i < 10; i++) q.push(i);
            KOUT(NON) << "LAUNCHING PREDICATED THREAD POOL";
            TestThreadQueueQObject ttpo2(mutex, q);
            kul::Ref<TestThreadQueueQObject> ref3(ttpo2);
            kul::PredicatedThreadQueue<std::queue<int> > tp2(ref3, q);
            tp2.setMax(kul::cpu::threads());
            tp2.detach();
            tp2.join();
            ttpo2.print();

            TestIPC().run();

            KOUT(NON) << kul::math::abs(-1);

            KOUT(NON) << kul::math::root(16);
            KOUT(NON) << kul::math::root(64, 3);
            KOUT(NON) << std::setprecision(16) << kul::math::root<double>(64, 6);
            KOUT(NON) << kul::math::root(64, 6, 8);
            KOUT(NON) << kul::math::root(64, 6, 3, 3);
            // kul::math::root(root of, nth root(default 2), iterations(default 6), starting guess(default ignored));

            KOUT(NON) << std::setprecision(6) << kul::math::root(2, 2); // float implicit
            KOUT(NON) << std::setprecision(16) << kul::math::root<double>(2, 3);

            KOUT(NON) << kul::math::pow(-2, 0); // float implicit
            KOUT(NON) << kul::math::pow(2, 5);
            KOUT(NON) << kul::math::pow(2, -5);
            KOUT(NON) << kul::math::pow<double>(2.2974, 5);
            KOUT(NON) << kul::math::pow(2, 6);

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
#endif /* _KUL_TEST_HPP_ */



