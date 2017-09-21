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
#ifndef _KUL_THREADS_OS_HPP_
#define _KUL_THREADS_OS_HPP_

#include "kul/threads.base.hpp"

#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#if defined(__NetBSD__)
#include <lwp.h>
#endif

namespace kul{ 
namespace this_thread{
inline const std::string id(){
    std::ostringstream os;
    os << std::hex << pthread_self();
    return os.str();
}

//http://stackoverflow.com/questions/4867839/how-can-i-tell-if-pthread-self-is-the-main-first-thread-in-the-process
inline bool main(){
#if defined(__FreeBSD__)
    return 0;
#elif defined(__NetBSD__)
    return _lwp_self();
#elif defined(__OpenBSD__)
    return 0;
#elif defined(__APPLE__)
    return 0;
#else
    return getpid() == syscall(SYS_gettid);
#endif
}
inline void kill(){
    pthread_exit(0);
}
} // END NAMESPACE this_thread

class Mutex{
    private:
        pthread_mutex_t mute;
    public:
        Mutex(){
            pthread_mutexattr_t att;
            pthread_mutexattr_init(&att);
            pthread_mutexattr_settype(&att, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&mute, &att);
            pthread_mutexattr_destroy(&att);
        }
        ~Mutex() {
            pthread_mutex_destroy(&mute);
        }
        bool tryLock(){
            return pthread_mutex_trylock(&mute);
        }
        void lock() {
            pthread_mutex_lock(&mute); 
        }
        void unlock() {
            pthread_mutex_unlock(&mute);
        }
};

class Thread : public threading::AThread{
    private:
        std::function<void()> func;
        pthread_t thr;
        static void* threadFunction(void* th){
            ((Thread*)th)->act();
            return 0;
        }
        void act(){
            try{
                func(); 
            }catch(const std::exception& e){ 
                ep = std::current_exception();
            }
            f = 1;
        }
    public:
        Thread(const std::function<void()>& func) : func(func){}
        template <class T> Thread(const T& t) : func(std::bind((void(T::*)())&T::operator(), t)){}
        template <class T> Thread(const std::reference_wrapper<T>& r) : func(std::bind((void(T::*)())&T::operator(), r)){}
        template <class T> Thread(const std::reference_wrapper<const T>& r) : func(std::bind((void(T::*)()const)&T::operator(), r)){}

        virtual ~Thread(){}
        bool detach(){
            return pthread_detach(thr);
        }
        void interrupt() KTHROW(kul::threading::InterruptionException){
            pthread_cancel(thr);
            f = 1;
        }
        void join(){
            if(!s) run();
            pthread_join(thr, 0);
            s = 0;
        }
        void run() KTHROW(kul::threading::Exception){
            if(s) KEXCEPTION("Thread running");
            f = 0;
            s = 1;
            pthread_create(&thr, NULL, Thread::threadFunction, this);
        }
};

}// END NAMESPACE kul
#endif /* _KUL_THREADS_OS_HPP_ */
