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
#ifndef _KUL_THREADS_HPP_
#define _KUL_THREADS_HPP_

#include "kul/map.hpp"
#include "kul/threads.os.hpp"

namespace kul{

class ScopeLock{
    private:
        Mutex& m;
    public:
        ScopeLock(Mutex& m) : m(m) {
            this->m.lock();
        }
        ~ScopeLock(){
            this->m.unlock();
        }
};

class ThreadQueue{
    protected:
        bool d = 0, f = 0, s = 0;
        uint16_t m = 1; 
        kul::Thread th;
        std::function<void()> func;
        std::vector<std::shared_ptr<kul::Thread> > ts;
        std::vector<std::exception_ptr> ePs;
        void setStarted()   { s = true; }
        virtual void start() throw (std::exception) {
            if(started()) KEXCEPT(Exception, "ThreadQueue is already started");
            setStarted();
            for(uint16_t i = 0 ; i < m; i++){
                std::shared_ptr<kul::Thread> at = std::make_shared<kul::Thread>(func);
                at->run();
                ts.push_back(at);
                this_thread::nSleep(__KUL_THREAD_SPAWN_WAIT__);
            }
        }
    public:
        ThreadQueue(const std::function<void()>& func) : th(std::ref(*this)), func(func){}
        template <class T> ThreadQueue(const T& t)     : th(std::ref(*this)), func(std::bind(&T::operator(), t)){}
        template <class T> ThreadQueue(const std::reference_wrapper<T>& ref) : th(std::ref(*this)), func(std::bind(&T::operator(), ref)){}
        void setMax(const int16_t& max) { m = max;}
        void run(){
            th.run();
        }
        void operator()(){
            start();
        }
        virtual void join() throw (std::exception){
            th.join();
            while(ts.size()){
                const std::shared_ptr<kul::Thread>* del = 0;
                for(const auto& t : ts){
                    if(t->finished()){
                        t->join();
                        if(t->exception() != std::exception_ptr()){
                            if(!d) std::rethrow_exception(t->exception());
                            ePs.push_back(t->exception());
                        }
                        del = &t; break;
                    }
                }
                this_thread::sleep(11);
                if(del) ts.erase(std::remove(ts.begin(), ts.end(), *del), ts.end());
            }
        }
        void detach(){
            d = true;
            th.detach();
        }
        void interrupt() throw(kul::threading::InterruptionException){ }
        const std::vector<std::exception_ptr> exceptionPointers() {
            return ePs;
        }
        bool started() { return s; }
        bool finished(){ return f; }
};


template<class P>
class PredicatedThreadQueue : public ThreadQueue{
    private:
        P& p;
        size_t ps;
    protected:
        void start() throw (std::exception) {
            if(started()) KEXCEPT(Exception, "ThreadQueue is already started");
            setStarted();
            size_t c = 0;
            while(c < ps){
                for(size_t i = ts.size(); i < m && c < ps; i++){
                    c++;
                    std::shared_ptr<kul::Thread> at = std::make_shared<kul::Thread>(func);
                    at->run();
                    ts.push_back(at);
                    this_thread::nSleep(__KUL_THREAD_SPAWN_WAIT__);
                }
                const std::shared_ptr<kul::Thread>* del = 0;
                for(const auto& t : ts){
                    if(t->finished()){
                        t->join();
                        if(t->exception() != std::exception_ptr()){
                            if(!d) std::rethrow_exception(t->exception());
                            ePs.push_back(t->exception());
                        }
                        del = &t; break;
                    }
                }
                this_thread::sleep(11);
                if(del) ts.erase(std::remove(ts.begin(), ts.end(), *del), ts.end());
            }
        }
    public:

        PredicatedThreadQueue(const std::function<void()>& func, P& pr) : ThreadQueue(func), p(pr), ps(p.size()){}
        template <class T> PredicatedThreadQueue(const T& t, P& pr) : ThreadQueue(t), p(pr), ps(p.size()){}
        template <class T> PredicatedThreadQueue(const std::reference_wrapper<T>& ref, P& pr) : ThreadQueue(ref), p(pr), ps(p.size()){}
};


template<class F, class E = kul::Exception>
class ConcurrentThreadQueue{
    friend class kul::Thread;
    protected:
        bool _detatched = 0, _up = 0;
        std::queue<std::pair<std::function<F>, std::function<void(const E&)>>> _q;
        kul::hash::map::S2T<std::shared_ptr<kul::Thread>> _k;
        kul::hash::map::S2T<std::function<void(const E&)>> _e;
        size_t _cur = 0, _max = 1;
        kul::Thread _thread;
        kul::Mutex _qmutex;

        void _throw(const std::exception_ptr& ep, const std::function<void(const E&)>& func){
            try{
                std::rethrow_exception(ep);
            }catch(const E& e){
                func(e);
            }
        }
        virtual void operator()(){
            while(_up){
                this_thread::nSleep(1000000);
                {
                    kul::ScopeLock l(_qmutex);
                    if(_q.empty()) continue;      
                }

                for(; _cur < _max; _cur++){  
                    kul::ScopeLock l(_qmutex);
                    auto& f(_q.front());
                    std::stringstream ss;
                    ss << &f;
                    auto k(ss.str());
                    _k.insert(k, std::make_shared<kul::Thread>(f.first));
                    _e.insert(k, f.second);
                    _k[k]->run();
                    _q.pop();
                }

                kul::hash::set::String del;
                for(const auto& t : _k){
                    if(t.second->finished()){
                        t.second->join();
                        if(t.second->exception() != std::exception_ptr()){
                            if(_e.count(t.first)) _throw(t.second->exception(), _e[t.first]);
                            else
                            if(!_detatched)       std::rethrow_exception(t.second->exception());
                        }
                        del.insert(t.first);
                        _cur--;
                    }
                }
                for(const auto& s : del) _k.erase(s) && _e.erase(s);
            }
        }
    public:
        ConcurrentThreadQueue(const size_t& max = 1, bool strt = 0) : _max(max), _thread(std::ref(*this)){
            _k.setDeletedKey("");
            _e.setDeletedKey("");
            if(strt) start();
        }
        void detatch(bool d = 1){
            _detatched = d;
        }
        virtual void shutdown(bool wait = 1, uint8_t timeout = 5){
            _up = 0;
            if(wait) join();
        }
        virtual void join(){
            _thread.join();
        }
        void stop(bool force = 1){
            _up = 0;
            if(!force) while(_q.size()){}
        }
        virtual void start(){
            if(!_up){
                _up  = 1;
                _thread.run();
            }
        }
        void async(const std::function<F>& function, const std::function<void(const E&)>& exception = std::function<void(const E&)>()){
            kul::ScopeLock l(_qmutex);
            _q.push(std::make_pair(function, exception));
        }

        const std::exception_ptr& exception(){ 
            return _thread.exception();
        }
};

template<class E>
class ConcurrentThreadPool;

class PoolThread{
    template<class E>
    friend class ConcurrentThreadPool;
    private:
        bool _finished = 0, _ready = 1, _run = 1;
        std::function<void()> _function;
        kul::Mutex _mutex;

        void set(const std::function<void()>& f){
            kul::ScopeLock l(_mutex);
            _function = f;
            _ready = 0;
        }
        bool ready(){
             kul::ScopeLock l(_mutex);
            return _ready;
        }
        void shutdown(bool wait = 0, uint8_t timeout = 5){
            {
                kul::ScopeLock l(_mutex);
                _run = 0;
            }
            uint8_t t = 0;
            if(wait && timeout) 
                while(!_finished) {
                    kul::this_thread::sleep(1);
                    t++;
                    if(t == timeout) break;
                }
        }
    public:
        PoolThread(){}
        void operator()(){
            while(1){
                this_thread::nSleep(1000000);
                {
                    kul::ScopeLock l(_mutex);
                    if(!_run) break;
                }
                if(!_function) continue;
                _function();
                kul::ScopeLock l(_mutex);
                _function = 0;
                _ready = 1;
            }
            _finished = 1;
        } 
};

template<class E = kul::Exception>
class ConcurrentThreadPool : public ConcurrentThreadQueue<void()>{

    protected:
        kul::hash::map::S2T<std::shared_ptr<kul::PoolThread>> _p;

        virtual void operator()(){
            while(_up){
                this_thread::nSleep(1000000);
                std::vector<std::string> del;
                for(const auto& t : _k){
                    if(t.second->started() && t.second->finished()){
                        t.second->join();
                        if(t.second->exception() != std::exception_ptr()){
                            if(_e.count(t.first)) _throw(t.second->exception(), _e[t.first]);
                            else
                            if(!_detatched)       std::rethrow_exception(t.second->exception());
                            del.push_back(t.first);
                        }
                    }
                }
                for(const auto& n : del){
                    _e.erase(n);
                    _k.erase(n);
                    _p.erase(n);
                    _p[n] = std::make_shared<kul::PoolThread>();
                    _k[n] = std::make_shared<kul::Thread>(std::ref(*_p[n].get()));
                    _k[n]->run();
                }

                {
                    kul::ScopeLock l(_qmutex);
                    if(_q.empty()) continue;
                }

                std::vector<std::string> ready;
                for(size_t i = 0; i < _max; i++){
                    const auto n = std::to_string(i);
                    if(_p[n]->ready()) ready.push_back(n);
                }

                for(const auto& n : ready){  
                    kul::ScopeLock l(_qmutex);
                    if(_q.empty()) break;
                    auto& f(_q.front());
                    _p[n]->set(f.first);
                    _e[n] = f.second;
                    _q.pop();
                }
            }
        }
    public:
        ConcurrentThreadPool(const size_t& max = 1, bool strt = 0) : ConcurrentThreadQueue(max, 0){
            for(size_t i = 0; i < max; i++){
                auto n = std::to_string(i);
                _p.insert(n, std::make_shared<kul::PoolThread>());
                _k.insert(n, std::make_shared<kul::Thread>(std::ref(*_p[n].get())));
            }
            _p.setDeletedKey("");
            if(strt) start();
        }
        ~ConcurrentThreadPool(){
            kul::ScopeLock l(_qmutex);
            _k.clear();
            _e.clear();
            _p.clear();
        }
        virtual void start() override {
            if(!_up){
                _up  = 1;
                _thread.run();
                for(const auto& t : _k) t.second->run();
            }
        }
        virtual void shutdown(bool wait = 0, uint8_t timeout = 5) override{
            _up = 0;
            for(const auto& t : _p) t.second->shutdown(wait, timeout);
            if(wait) join();
        }
        virtual void join(){
            _thread.join();
            for(const auto& t : _k) t.second->join();
        }
        void detach(){
            if(_up){
                _thread.detach();
                for(const auto& t : _k) t.second->detach();
            }
        }
};


}// END NAMESPACE kul

#endif /* _KUL_THREADS_HPP_ */
