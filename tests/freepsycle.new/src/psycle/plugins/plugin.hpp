// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface declarations needed by psycle::plugins.
#ifndef PSYCLE__PLUGINS__PLUGIN__INCLUDED
#define PSYCLE__PLUGINS__PLUGIN__INCLUDED
#pragma once
#include <psycle/engine.hpp>
#include <boost/preprocessor/seq.hpp>
#include <cmath>
#define PSYCLE__DECL  PSYCLE__PLUGINS__PLUGIN
#include <psycle/detail/decl.hpp>

///\internal
/// extensible modular audio frawework.
namespace psycle {

///\internal
/// functionalities used by the plugin side only, not by the host.
/// Place your plugins in this namespace.
namespace plugins {

using engine::exception;
using engine::node;
using engine::plugin_library_reference;
using engine::real;
using engine::channel;
using engine::buffer;
using engine::port;
//namespace ports = engine::ports;
namespace ports {
	using engine::ports::output;
	using engine::ports::input;
	namespace inputs {
		using namespace engine::ports::inputs;
	}
}
//namespace math = engine::math;
namespace math { using namespace engine::math; }

#define PSYCLE__PLUGINS__CALLING_CONVENTION  UNIVERSALIS__COMPILER__CALLING_CONVENTION__C

///\todo function signature currently typedefed in host::plugin_resolver
//typedef engine::node & (*node_instanciator) (engine::reference_counter &, engine::node::name_type const &);

#define PSYCLE__PLUGINS__NODE_INSTANTIATOR(typename) \
	extern "C" { \
		UNIVERSALIS__COMPILER__DYN_LINK__EXPORT \
		psycle::engine::node &  \
		PSYCLE__PLUGINS__CALLING_CONVENTION \
		PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(new) ( \
			psycle::engine::plugin_library_reference & plugin_library_reference, \
			psycle::engine::node::name_type const & name \
		) throw(int) { \
			return *new typename(plugin_library_reference, name); \
		} \
		\
		UNIVERSALIS__COMPILER__DYN_LINK__EXPORT \
		void \
		PSYCLE__PLUGINS__CALLING_CONVENTION \
		PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(delete)(psycle::engine::node & node) { \
			delete &node; \
		} \
	}

/// generates switch statements that evaluate 'flags_to_evaluate' and call 'template' with the result of the evaluation
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH(template, flags_to_evaluate) \
	BOOST_PP_CAT(PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__, BOOST_PP_SEQ_SIZE(flags_to_evaluate))(template,, flags_to_evaluate)

///\internal implementation for PSYCLE__PLUGINS__TEMPLATE_SWITCH with 1 flag
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

///\internal implementation for PSYCLE__PLUGINS__TEMPLATE_SWITCH with 2 flags
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

///\internal implementation for PSYCLE__PLUGINS__TEMPLATE_SWITCH with 3 flags
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

///\internal implementation for PSYCLE__PLUGINS__TEMPLATE_SWITCH with 4 flags
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

///\internal implementation for PSYCLE__PLUGINS__TEMPLATE_SWITCH with 5 flags
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

///\internal implementation for PSYCLE__PLUGINS__TEMPLATE_SWITCH with 6 flags
#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__6(template, evaluated_flags, flags_to_evaluate) \
	switch(BOOST_PP_SEQ_HEAD(flags_to_evaluate)) { \
		case psycle::engine::channel::flags::continuous: \
			PSYCLE__PLUGINS__TEMPLATE_SWITCH__DETAIL__5(template, evaluated_flags \
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
#include <psycle/detail/decl.hpp>
#endif
