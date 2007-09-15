// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.sourceforge.net

///\interface virtual constructor and destructor support à la delphi
#pragma once
#include <universalis/compiler/template_constructors.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/ref.hpp>
namespace universalis { namespace compiler {
	class virtual_factory {
		public:
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
				Type static & create_on_heap(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
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
