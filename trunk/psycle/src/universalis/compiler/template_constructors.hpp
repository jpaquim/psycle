// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2006 johan boule <bohan@jabber.org>
// copyright 2004-2006 psycledelics http://psycle.sourceforge.net

///\file
///\brief loop for defining template constructors
#pragma once
#include <universalis/detail/project.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/seq/elem.hpp>
//#region UNIVERSALIS
	//#region COMPILER
		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM  8
		#if UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY < UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
			#undef  UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY
			#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY  UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
		#endif

		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(enclosing, nested_access, after_construction, before_destruction, arity) \
			BOOST_PP_REPEAT(arity, UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__FACTORY, enclosing) \
			BOOST_PP_REPEAT(arity, UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__FACTORY__VIRTUAL, nested_access) \
			class nested_access \
			{ \
				private: friend class enclosing; \
				BOOST_PP_REPEAT(arity, UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__NEW, after_construction) \
				void destroy(enclosing & instance) \
				{ \
					instance.before_destruction(); \
					delete & instance; \
				} \
			};

		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__FACTORY(_, count, type) \
			BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
			typename type static & create(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
			{ \
				return type::template create< typename type >(BOOST_PP_ENUM_PARAMS(count, xtra)); \
			}

		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__FACTORY__VIRTUAL(_, count, nested_access) \
			template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra) > \
			Type static & create(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
			{ \
				return nested_access::template create< Type BOOST_PP_ENUM_TRAILING_PARAMS(count, Xtra) >(BOOST_PP_ENUM_PARAMS(count, xtra)); \
			}

		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__NEW(_, count, after_construction) \
			template<typename Type BOOST_PP_ENUM_TRAILING_PARAMS(count, typename Xtra)> \
			Type static & create(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
			{ \
				Type & instance(*new Type(BOOST_PP_ENUM_PARAMS(count, xtra))); \
				instance.after_construction(); \
				return instance; \
			}

		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__CONSTRUCTORS(type, base_type, arity) \
			BOOST_PP_REPEAT(arity, UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__CONSTRUCTOR, (type)(base_type))
		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__CONSTRUCTOR(_, count, types) \
			protected: \
				BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
				BOOST_PP_SEQ_ELEM(0, types)(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
				: BOOST_PP_SEQ_ELEM(1, types)(BOOST_PP_ENUM_PARAMS(count, xtra)) {}
	//#endregion
//#endregion
