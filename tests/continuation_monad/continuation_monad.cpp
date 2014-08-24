
//https://www.fpcomplete.com/blog/2012/06/asynchronous-api-in-c-and-the-continuation-monad

#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include <string>
#include <iostream>

namespace test {
	using namespace std;
	
	template<typename R, typename A>
	class continuator {
		public:
			virtual ~continuator() {}
			virtual R and_then(function<R(A)> f) {}
	};
	
	template<typename R, typename A>
	class returnor : public continuator<R, A> {
		public:
			returnor(A a) : a_(a) {}
			R and_then(function<R(A)> f) override {
				return f(a_);
			}
		private:
			A a_;
	};
	
	template<typename R, typename A, typename C>
	class bindor : public continuator<R, A> {
		public:
			bindor(C c, function<unique_ptr<continuator<R, A>>(A)> fc) : c_(c), fc_(fc) {}
			
			R and_then(function<R(A)> f) override {
				auto fc = fc_;
				c_.and_then([fc, f] (A a) {
					return fc(a)->and_then(f);
				});
			}
		private:
			C c_;
			function<unique_ptr<continuator<R, A>>(A)> fc_;
	};
	
	void async_api_function(function<void(string)> f) {
		thread t([f] () {
			cout << "async started\n";
			this_thread::sleep_for(chrono::seconds(3));
			f("async done\n");
		});
		t.detach();
	}
	
	class async_api : public continuator<void, string> {
		public:
			void and_then(function<void(string)> f) override {
				async_api_function(f);
			}
	};
	
	class loop : public continuator<void, string> {
		public:
			loop(string s) : s_(s) {}
			
			void and_then(function<void(string)> f) override {
				cout << "loop::and_then: " << s_ << '\n';
				bindor<void, string, async_api>(async_api(), [] (string s) {
					return unique_ptr<continuator>(new loop(s));
				}).and_then(f);
			}
		private:
			string s_;
	};
}

int main() {
	using namespace std;
	using namespace test;
	
	async_api a;
	a.and_then([] (string s) {
		cout << s;
	});
	
	// This code will be executed in parallel to the async api.
	for(auto i = 0; i < 200; ++i) {
		cout << i << '\n';
		this_thread::sleep_for(chrono::seconds(1));
	}
}

