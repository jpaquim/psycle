// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.sourceforge.net

///\file
///\brief loop for defining template constructors
#pragma once
#include <universalis/detail/project.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/ref.hpp>
//#region UNIVERSALIS
	//#region COMPILER
		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM  8
		#if UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY < UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
			#undef  UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY
			#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY  UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
		#endif

		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__LOOP(constructor) \
			BOOST_PP_REPEAT(UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)

		#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(type, base_type, arity) \
			BOOST_PP_REPEAT(arity, UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__DETAIL, (type)(base_type))

		///\internal			
		//#region DETAIL
			#define UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__DETAIL(_, count, types) \
				protected: \
					BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
					BOOST_PP_SEQ_ELEM(0, types)(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
					: BOOST_PP_SEQ_ELEM(1, types)(BOOST_PP_ENUM_PARAMS(count, xtra)) {}
		//#endregion
	//#endregion
//#endregion
