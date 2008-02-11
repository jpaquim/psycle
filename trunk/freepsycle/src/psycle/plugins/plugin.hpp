// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface declarations needed by psycle::plugins.
#pragma once
#include <psycle/engine.hpp>
#include <cmath>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__PLUGIN
#include <universalis/compiler/dynamic_link/begin.hpp>
///\internal
/// extensible modular audio frawework.
namespace psycle {
///\internal
/// functionalities used by the plugin side only, not by the host.
/// Place your plugins in this namespace.
namespace plugins {

typedef engine::real real;

///\internal
/// plugins driving an underlying output device.
namespace outputs {}

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

#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__2(t, a, b) \
	if(a) { \
		if(b) t<true, true>(); \
		else t<true, false>(); \
	} else if(b) t<false, true>(); \
	else t<false, false>();

#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__3(t, a, b, c) \
	if(a) { \
		if(b) { \
			if(c) t<true, true, true>(); \
			else t<true, true, false>(); \
		} else if(c) t<true, false, true>(); \
		else t<true, false, false>(); \
	} else if(b) { \
		if(c) t<false, true, true>(); \
		else t<false, true, false>(); \
	} else if(c) t<false, false, true>(); \
	else t<false, false, false>();

#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__4(t, a, b, c, d) \
	if(a) { \
		if(b) { \
			if(c) { \
				if(d) t<true, true, true, true>(); \
				else t<true, true, true, false>(); \
			} else if(d) t<true, true, false, true>(); \
			else t<true, true, false, false>(); \
		} else if(c) { \
			if(d) t<true, false, true, true>(); \
			else t<true, false, true, false>(); \
		} else if(d) t<true, false, false, true>(); \
		else t<true, false, false, false>(); \
	} else if(b) { \
		if(c) { \
			if(d) t<false, true, true, true>(); \
			else t<false, true, true, false>(); \
		} else if(d) t<false, true, false, true>(); \
		else t<false, true, false, false>(); \
	} else if(c) { \
		if(d) t<false, false, true, true>(); \
		else t<false, false, true, false>(); \
	} else if(d) t<false, false, false, true>(); \
	else t<false, false, false, false>();

}}
#include <universalis/compiler/dynamic_link/end.hpp>
