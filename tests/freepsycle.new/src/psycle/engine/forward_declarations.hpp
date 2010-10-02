// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
/// forward declarations for the namespace psycle::engine.
/// This file is used to avoid mutually recursive dependencies in declarations.
/// It does not have any real declarations, just forward names.
///\see file engine.hpp to #include the real declarations
#ifndef PSYCLE__ENGINE__FORWARD_DECLARATIONS__INCLUDED
#define PSYCLE__ENGINE__FORWARD_DECLARATIONS__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>
#include "sample.hpp"
namespace psycle {
	//namespace loggers = universalis::os::loggers;
	namespace loggers { using namespace universalis::os::loggers; }

	namespace engine {
		class exception;
		class named;
		class graph;
		class node;
		class plugin_library_reference;
		#define PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(x) psycle__engine__node_instance__##x
		class port;
		namespace ports {
			class output;
			class input;
			namespace inputs {
				class single;
				class multiple;
			}
		}
		class buffer;
		class channel;
		class event;
		class sequence;
	}
}
#endif
