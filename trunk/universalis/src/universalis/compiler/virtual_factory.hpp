// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.sourceforge.net

///\interface virtual constructor and destructor support à la delphi but also with support for creating on the stack
#pragma once
#include <universalis/compiler/template_constructors.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
namespace universalis { namespace compiler {

	//template<>
	//class virtual_factory<> {
	class basic_virtual_factory {
		public:
			class virtual_factory_access {
				public://private: friend class basic_virtual_factory;
				
					#if defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION__MAJOR < 4
						// A gross hack is needed for gcc < 4 because making virtual_factory_access a friend class
						// does not make the nested create_on_stack class a friend too on this version of the compiler.
						// Perhaps this is also needed for msvc 14 ("8.0"), but that compiler is already unsupported because
						// it's having a lot more troubles anyway.
						#define protected public
					#endif
					
					template<typename Type>
					class create_on_stack {
						private:
							Type instance;
						public:
							#define constructor(_, count, __) \
								BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
								create_on_stack(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
								: instance(BOOST_PP_ENUM_PARAMS(count, xtra)) { instance.after_construction(); }
								
								UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor)
							#undef constructor
							operator Type & () { return instance; }
							operator Type const & () const { return instance; }
							~create_on_stack()  { instance.before_destruction(); }
						};
						
					#define constructor(_, count, __) \
						template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra) > \
						Type static & create_on_heap(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) { \
							Type & instance(*new Type(BOOST_PP_ENUM_PARAMS(count, xtra))); \
							instance.after_construction(); \
							return instance; \
						}
						
						UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor)
					#undef constructor
			};
			
			#define constructor(_, count, __) \
				template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra) > \
				Type static & create_on_heap(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) { \
					return virtual_factory_access::template create_on_heap<Type BOOST_PP_ENUM_TRAILING_PARAMS(count, Xtra) >(BOOST_PP_ENUM_PARAMS(count, xtra)); \
				}
				
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor)
			#undef constructor

			void free_heap() {
				before_destruction();
				delete this;
			}
			
		protected:
			basic_virtual_factory() {}
			void virtual after_construction() {}
			void virtual before_destruction() {}
			virtual ~basic_virtual_factory() {}
	};
	
	template<typename Derived = void, typename Base = void /*basic_virtual_factory*/>
	class virtual_factory : public Base {
		protected:
			typedef virtual_factory virtual_factory_type;
			UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(virtual_factory, Base, UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY)
		public:
			typedef basic_virtual_factory::virtual_factory_access::create_on_stack<Derived> create_on_stack;

			#define constructor(_, count, __) \
				BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
				Derived static & create_on_heap(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) { \
					return basic_virtual_factory::template create_on_heap<Derived BOOST_PP_ENUM_TRAILING_PARAMS(count, Xtra) >(BOOST_PP_ENUM_PARAMS(count, xtra)); \
				}
				
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor)
			#undef constructor
	};

	#if 1
	template<typename Derived>
	class virtual_factory<Derived> : public basic_virtual_factory {
		protected:
			typedef virtual_factory virtual_factory_type;
		public:
			typedef basic_virtual_factory::virtual_factory_access::create_on_stack<Derived> create_on_stack;

			#define constructor(_, count, __) \
				BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
				Derived static & create_on_heap(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) { \
					return basic_virtual_factory::template create_on_heap<Derived BOOST_PP_ENUM_TRAILING_PARAMS(count, Xtra) >(BOOST_PP_ENUM_PARAMS(count, xtra)); \
				}
				
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor)
			#undef constructor
	};
	#endif
}}
