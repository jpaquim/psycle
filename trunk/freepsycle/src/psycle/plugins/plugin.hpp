// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface declarations needed by psycle::plugins.
#pragma once
#include <psycle/engine.hpp>
#include <cmath>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__PLUGIN
#include <universalis/compiler/dynamic_link/begin.hpp>
#include <boost/preprocessor/seq.hpp>
///\internal
/// extensible modular audio frawework.
namespace psycle {
///\internal
/// functionalities used by the plugin side only, not by the host.
/// Place your plugins in this namespace.
namespace plugins {

typedef engine::real real;

#define PSYCLE__PLUGINS__CALLING_CONVENTION  UNIVERSALIS__COMPILER__CALLING_CONVENTION__C

#define PSYCLE__PLUGINS__NODE_INSTANTIATOR(typename) \
	extern "C" { \
		UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT \
		psycle::engine::node &  \
		PSYCLE__PLUGINS__CALLING_CONVENTION \
		PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(new) ( \
			psycle::engine::plugin_library_reference & plugin_library_reference, \
			psycle::engine::graph & graph, \
			std::string const & name \
		) \
		throw(psycle::engine::exception) { \
			return psycle::engine::node::virtual_factory_access::create_on_heap<typename>(plugin_library_reference, graph, name); \
		} \
		\
		UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT \
		void \
		PSYCLE__PLUGINS__CALLING_CONVENTION \
		PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(delete)(psycle::engine::node & node) { \
			node.free_heap(); \
		} \
	}

#define PSYCLE__PLUGINS__TEMPLATE_SWITCH(template, flags_to_evaluate) \
	BOOST_PP_CAT(PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__, BOOST_PP_SEQ_SIZE(flags_to_evaluate))(template,, flags_to_evaluate)

///\internal
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__1(template, evaluated_flags, flags_to_evaluate) \
	switch(BOOST_PP_SEQ_HEAD(flags_to_evaluate)) { \
		case psycle::engine::channel::flags::continuous: \
			template<BOOST_PP_SEQ_ENUM(evaluated_flags), psycle::engine::channel::flags::continuous>(); \
		break; \
		case psycle::engine::channel::flags::empty: \
			template<BOOST_PP_SEQ_ENUM(evaluated_flags), psycle::engine::channel::flags::empty>(); \
		break; \
		case psycle::engine::channel::flags::discrete: \
			template<BOOST_PP_SEQ_ENUM(evaluated_flags), psycle::engine::channel::flags::discrete>(); \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

///\internal
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__2(template, evaluated_flags, flags_to_evaluate) \
	switch(BOOST_PP_SEQ_HEAD(flags_to_evaluate)) { \
		case psycle::engine::channel::flags::continuous: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__1(template, evaluated_flags \
				(psycle::engine::channel::flags::continuous), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::empty: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__1(template, evaluated_flags \
				(psycle::engine::channel::flags::empty), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::discrete: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__1(template, evaluated_flags \
				(psycle::engine::channel::flags::discrete), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

///\internal
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__3(template, evaluated_flags, flags_to_evaluate) \
	switch(BOOST_PP_SEQ_HEAD(flags_to_evaluate)) { \
		case psycle::engine::channel::flags::continuous: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__2(template, evaluated_flags \
				(psycle::engine::channel::flags::continuous), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::empty: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__2(template, evaluated_flags \
				(psycle::engine::channel::flags::empty), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::discrete: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__2(template, evaluated_flags \
				(psycle::engine::channel::flags::discrete), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

///\internal
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__4(template, evaluated_flags, flags_to_evaluate) \
	switch(BOOST_PP_SEQ_HEAD(flags_to_evaluate)) { \
		case psycle::engine::channel::flags::continuous: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__3(template, evaluated_flags \
				(psycle::engine::channel::flags::continuous), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::empty: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__3(template, evaluated_flags \
				(psycle::engine::channel::flags::empty), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::discrete: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__3(template, evaluated_flags \
				(psycle::engine::channel::flags::discrete), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

///\internal
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__5(template, evaluated_flags, flags_to_evaluate) \
	switch(BOOST_PP_SEQ_HEAD(flags_to_evaluate)) { \
		case psycle::engine::channel::flags::continuous: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__4(template, evaluated_flags \
				(psycle::engine::channel::flags::continuous), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::empty: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__4(template, evaluated_flags \
				(psycle::engine::channel::flags::empty), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::discrete: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__4(template, evaluated_flags \
				(psycle::engine::channel::flags::discrete), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

///\internal
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__6(template, evaluated_flags, flags_to_evaluate) \
	switch(BOOST_PP_SEQ_HEAD(flags_to_evaluate)) { \
		case psycle::engine::channel::flags::continuous: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__5(template, evaluated_flags \
				(psycle::engine::channel::flags::continuous), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::empty: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__5(template, evaluated_flags \
				(psycle::engine::channel::flags::empty), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		case psycle::engine::channel::flags::discrete: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__5(template, evaluated_flags \
				(psycle::engine::channel::flags::discrete), BOOST_PP_SEQ_TAIL(flags_to_evaluate) \
			); \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

}}
#include <universalis/compiler/dynamic_link/end.hpp>
