#include <iostream>
#include <cmath>

class node {
	public:
		float f_;

	protected:
		friend class factory;
		
		node(float f) : f_(f) {
			for(int i(0); i < 1000000; ++i) f_ += std::log(f_);
			std::cout << "ctor " << f_ << "\n";
		}

		void virtual after_construction() {
			std::cout << "actor\n";
		}

		void virtual before_destruction() {
			std::cout << "bdtor\n";
		}
		
		virtual ~node() {
			for(int i(0); i < 1000000; ++i) f_ -= std::log(f_);
			std::cout << "dtor " << f_ << "\n";
		}

	public:
		class factory {
			public:
				template<typename T>
				T static & create_on_heap(float f) {
					T & nvr(*new T(f));
					nvr.after_construction();
					return nvr;
				}
				
				template<typename T>
				T static create_on_stack(float f) {
					T nvr(f);
					nvr.after_construction();
					return nvr;
				}
		};
		
		void virtual destroy() {
			before_destruction();
			delete this;
		}
};

class sine : public node {
	protected: friend class factory;
		sine(float f) : node(f) {}
};

int main(int, char**) {
	std::cout << "--------------\n";
	{
		node & n(sine::factory::create_on_heap<sine>(2));
		std::cout << n.f_ << "\n";
		n.destroy();
	}
	std::cout << "--------------\n";
	{
		sine s(sine::factory::create_on_stack<sine>(2));
		std::cout << s.f_ << "\n";
	}
	std::cout << "--------------\n";
}
