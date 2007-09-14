#include <iostream>

class node {
	public:
		float f_;

	protected:
		friend class factory;
		
		node(float f) : f_(f) {
			std::cout << "ctor\n";
		}

		void virtual after_construction() {
			std::cout << "after ctor\n";
		}

		void virtual before_destruction() {
			std::cout << "before dtor\n";
		}
		
		virtual ~node() {
			std::cout << "dtor\n";
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
				class create_on_stack {
					private:
						T t_;
					public:
						create_on_stack(float f) : t_(f) { t_.after_construction(); }
						operator T & () { return t_; }
						~create_on_stack() { t_.before_destruction(); }
				};
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
		node & n(node::factory::create_on_heap<sine>(2));
		std::cout << n.f_ << "\n";
		n.destroy();
	}
	std::cout << "--------------\n";
	{
		node::factory::create_on_stack<sine> ss(2);
		node & n(ss);
		std::cout << n.f_ << "\n";
	}
	std::cout << "--------------\n";
}
