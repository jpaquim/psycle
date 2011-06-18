// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <psycle/engine.hpp>
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
		) { \
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
}}
#include <psycle/detail/decl.hpp>
