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
		bool d = 0, s = 0;
		unsigned int m = 1;	
		kul::Ref<ThreadQueue> re;
		kul::Thread th;
		std::shared_ptr<kul::threading::ThreadObject> to;
		std::vector<std::shared_ptr<kul::Thread> > ts;
		std::vector<std::exception_ptr> ePs;
		void setStarted()	{ s = true; }
		bool started()		{ return s; }
		virtual void start() throw (std::exception) {
			if(started()) KEXCEPT(Exception, "ThreadQueue is already started");
			setStarted();
			for(unsigned int i = 0 ; i < m; i++){
				std::shared_ptr<kul::Thread> at = std::make_shared<kul::Thread>(to);
				at->run();
				ts.push_back(at);
				this_thread::nSleep(__KUL_THREAD_SPAWN_WAIT__);
			}
		}
	public:
		template <class T> ThreadQueue(const T& t) 			: re(*this), th(re), to(std::make_shared<kul::ThreadCopy<T> >(t)){}
		template <class T> ThreadQueue(const Ref<T>& ref) 	: re(*this), th(re), to(std::make_shared<kul::ThreadRef<T> >(ref)){}
		void setMax(const int& max) { m = max;}
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
};

template<class P>
class PredicatedThreadQueue : public ThreadQueue{
	private:
		P& p;
		unsigned int ps;
	protected:
		void start() throw (std::exception) {
			if(started()) KEXCEPT(Exception, "ThreadQueue is already started");
			setStarted();
			unsigned int c = 0;
			while(c < ps){
				for(unsigned int i = ts.size(); i < m && c < ps; i++){
					c++;
					std::shared_ptr<kul::Thread> at = std::make_shared<kul::Thread>(to);
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
		template <class T> PredicatedThreadQueue(const T& t, P& pr) 			: ThreadQueue(t) 	, p(pr), ps(p.size()){}
		template <class T> PredicatedThreadQueue(const Ref<T>& ref, P& pr) 	: ThreadQueue(ref)	, p(pr), ps(p.size()){}
};
}// END NAMESPACE kul

#endif /* _KUL_THREADS_HPP_ */
