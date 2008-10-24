// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

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
	switch(a) { \
		case psycle::engine::channel::flags::continuous: \
			switch(b) { \
				case psycle::engine::channel::flags::continuous: \
					t<psycle::engine::channel::flags::continuous, psycle::engine::channel::flags::continuous>(); \
				break; \
				case psycle::engine::channel::flags::empty: \
					t<psycle::engine::channel::flags::continuous, psycle::engine::channel::flags::empty>(); \
				break; \
				case psycle::engine::channel::flags::discrete: \
					t<psycle::engine::channel::flags::continuous, psycle::engine::channel::flags::discrete>(); \
				break; \
				default: \
					throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
			} \
		break; \
		case psycle::engine::channel::flags::empty: \
			switch(b) { \
				case psycle::engine::channel::flags::continuous: \
					t<psycle::engine::channel::flags::empty, psycle::engine::channel::flags::continuous>(); \
				break; \
				case psycle::engine::channel::flags::empty: \
					t<psycle::engine::channel::flags::empty, psycle::engine::channel::flags::empty>(); \
				break; \
				case psycle::engine::channel::flags::discrete: \
					t<psycle::engine::channel::flags::empty, psycle::engine::channel::flags::discrete>(); \
				break; \
				default: \
					throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
			} \
		break; \
		case psycle::engine::channel::flags::discrete: \
			switch(b) { \
				case psycle::engine::channel::flags::continuous: \
					t<psycle::engine::channel::flags::discrete, psycle::engine::channel::flags::continuous>(); \
				break; \
				case psycle::engine::channel::flags::empty: \
					t<psycle::engine::channel::flags::discrete, psycle::engine::channel::flags::empty>(); \
				break; \
				case psycle::engine::channel::flags::discrete: \
					t<psycle::engine::channel::flags::discrete, psycle::engine::channel::flags::discrete>(); \
				break; \
				default: \
					throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
			} \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

#define PSYCLE__PLUGINS__TEMPLATE_SWITCH__3(t, a, b, c) \
	switch(a) { \
		case psycle::engine::channel::flags::continuous: \
			switch(b) { \
				case psycle::engine::channel::flags::continuous: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				case psycle::engine::channel::flags::empty: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				case psycle::engine::channel::flags::discrete: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				default: \
					throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
			} \
		break; \
		case psycle::engine::channel::flags::empty: \
			switch(b) { \
				case psycle::engine::channel::flags::continuous: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				case psycle::engine::channel::flags::empty: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				case psycle::engine::channel::flags::discrete: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				default: \
					throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
			} \
		break; \
		case psycle::engine::channel::flags::discrete: \
			switch(b) { \
				case psycle::engine::channel::flags::continuous: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::continuous, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				case psycle::engine::channel::flags::empty: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::empty, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				case psycle::engine::channel::flags::discrete: \
					switch(c) { \
						case psycle::engine::channel::flags::continuous: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::continuous>(); \
						break; \
						case psycle::engine::channel::flags::empty: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::empty>(); \
						break; \
						case psycle::engine::channel::flags::discrete: t< \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::discrete, \
							psycle::engine::channel::flags::discrete>(); \
						break; \
						default: \
							throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
					} \
				break; \
				default: \
					throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
			} \
		break; \
		default: \
			throw psycle::engine::exceptions::runtime_error("unhandled enumeration value", UNIVERSALIS__COMPILER__LOCATION); \
	}

}}
#include <universalis/compiler/dynamic_link/end.hpp>
