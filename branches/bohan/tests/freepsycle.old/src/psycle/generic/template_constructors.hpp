// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.sourceforge.net

///\file
///\brief loop for defining template constructors

#ifndef PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__INCLUDED
#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__INCLUDED
#pragma once

#include <psycle/detail/project.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/ref.hpp>

#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM  8
#if PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY < PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
	#undef  PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
	#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY  PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
#endif

#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__LOOP(constructor) \
	BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)

#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS(type, base_type, arity) \
	BOOST_PP_REPEAT(arity, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__DETAIL, (type)(base_type)({}))

#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__WITH_BODY(type, base_type, body, arity) \
	BOOST_PP_REPEAT(arity, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__DETAIL, (type)(base_type)(body))

///\internal
#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__DETAIL(_, count, types_and_body) \
	protected: \
		BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
		BOOST_PP_SEQ_ELEM(0, types_and_body)(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
		: BOOST_PP_SEQ_ELEM(1, types_and_body)(BOOST_PP_ENUM_PARAMS(count, xtra)) BOOST_PP_SEQ_ELEM(2, types_and_body)

#endif
