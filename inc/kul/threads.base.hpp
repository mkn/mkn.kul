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
#ifndef _KUL_THREADS_BASE_HPP_
#define _KUL_THREADS_BASE_HPP_

#include <queue>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "kul/defs.hpp" 
#include "kul/type.hpp" 
#include "kul/except.hpp"

namespace kul{ 
namespace this_thread{	
inline void sleep(const unsigned long& millis) { std::this_thread::sleep_for(std::chrono::milliseconds(millis));}
inline void uSleep(const unsigned long& micros){ std::this_thread::sleep_for(std::chrono::microseconds(micros));}
inline void nSleep(const unsigned long& nanos) { std::this_thread::sleep_for(std::chrono::nanoseconds(nanos));}
}// END NAMESPACE this_thread

// class ThreadQueue;
// template<class P> class PredicatedThreadQueue;

namespace threading{
class Exception : public kul::Exception{
	public:
		Exception(const char*f, const int l, const std::string& s) : kul::Exception(f, l, s){}
};
class InterruptionException : public Exception{
	public:
		InterruptionException(const char*f, const int l, const std::string& s) : Exception(f, l, s){}
};

class AThread;
class ThreadObject{
	private:		
		virtual void act() = 0;
		friend class AThread;
	public:
		virtual ~ThreadObject(){}
};
}
template <class T>
class ThreadCopy : public threading::ThreadObject{
	private:
		T t;
		void act(){ t(); } 
	public:
		ThreadCopy(T t) : t(t){}
};
template <class T>
class ThreadRef : public threading::ThreadObject{
	private:
		const Ref<T>& t;
		void act(){ t.get()(); } 
	public:
		ThreadRef(const Ref<T>& t) : t(t){}
};
namespace threading{
class AThread{
	protected:
		std::atomic<bool> f, s;
		std::exception_ptr ep;
		std::shared_ptr<threading::ThreadObject> to;

		AThread(const std::shared_ptr<ThreadObject>& t) : f(1), s(0), to(t){}
		template <class T> AThread(const T& t) : f(1), s(0), to(std::make_shared<ThreadCopy<T>>(t)){}
		template <class T> AThread(const Ref<T>& t) : f(1), s(0), to(std::make_shared<ThreadRef<T>>(t)){}
		void act(){
			try{
				to->act(); 
			}catch(const std::exception& e){ 
				ep = std::current_exception();
			}
			f = 1;
		}
		virtual void run() throw(kul::threading::Exception) = 0;
	public:
		virtual ~AThread(){}
		virtual void join() = 0;
		bool started() { return s; }
		bool finished(){ return f; }
		const std::exception_ptr& exception(){ return ep;}
		void rethrow(){ if(ep) std::rethrow_exception(ep);}
};

} // END NAMESPACE threading
} // END NAMESPACE kul
#endif /* _KUL_THREADS_BASE_HPP_ */
