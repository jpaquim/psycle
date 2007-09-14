#include <iostream>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/ref.hpp>
namespace universalis { namespace compiler {
	#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM  8
	#if UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY < UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
		#undef  UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY
		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY  UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
	#endif
	
	#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor) BOOST_PP_REPEAT(UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)

	class virtual_factory {
		public:
			template<typename Type>
			class stack_alloc {
				private:
					Type instance;
				public:
					#define constructor(_, count, __) \
						BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
						stack_alloc(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
						: instance(BOOST_PP_ENUM_PARAMS(count, xtra)) { instance.after_construction(); }
						
						UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor)
					#undef constructor
					operator Type & () { return instance; }
					operator Type const & () const { return instance; }
					~stack_alloc()  { instance.before_destruction(); }
				};
				
			#define constructor(_, count, __) \
				template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra) > \
				Type static & heap_alloc(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
				{ \
					Type & instance(*new Type(BOOST_PP_ENUM_PARAMS(count, xtra))); \
					instance.after_construction(); \
					return instance; \
				}
				
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor)
			#undef constructor

			void heap_free() {
				before_destruction();
				delete this;
			}
			
		protected:
			virtual_factory() {}
			void virtual after_construction() {}
			void virtual before_destruction() {}
			virtual ~virtual_factory() {}
	};
}}

namespace test {
	class node : public universalis::compiler::virtual_factory {
		public:
			float f_;

		protected: friend class universalis::compiler::virtual_factory;
			node(float f) : f_(f) { std::cout << "ctor (node)\n"; }
			void after_construction() { std::cout << "after ctor (node)\n"; }
			void before_destruction() { std::cout << "before dtor (node)\n"; }
			virtual ~node() { std::cout << "dtor (node)\n"; }
	};

	class sine : public node {
		protected: friend class universalis::compiler::virtual_factory;
			sine(float f) : node(f) { std::cout << "ctor (sine)\n"; }
			void after_construction() { node::after_construction(); std::cout << "after ctor (sine)\n"; }
			void before_destruction() { std::cout << "before dtor (sine)\n"; node::before_destruction(); }
			virtual ~sine() { std::cout << "dtor (sine)\n"; }
	};
}

int main(int, char**) {
	using namespace test;
	std::cout << "--------------\n";
	{
		node & n(node::heap_alloc<sine>(boost::cref(2)));
		std::cout << n.f_ << "\n";
		n.heap_free();
	}
	std::cout << "--------------\n";
	{
		node::stack_alloc<sine> s_(boost::cref(2)); node & n(s_);
		std::cout << n.f_ << "\n";
	}
	std::cout << "--------------\n";
}
