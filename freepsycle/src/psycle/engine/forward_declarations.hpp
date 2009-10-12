// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
/// forward declarations for the namespace psycle::engine.
/// This file is used to avoid mutually recursive dependencies in declarations.
/// It does not have any real declarations, just forward names.
///\see file engine.hpp to #include the real declarations
#pragma once
#include <psycle/detail/project.hpp>
#include <psycle/generic/typenames.hpp>
#include <universalis/operating_system/loggers.hpp>
#include "sample.hpp"
namespace psycle {
	namespace loggers = universalis::operating_system::loggers;

	namespace engine {
		class exception;
		class named;
		class graph;
		class node;
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
		
		namespace typenames {
			using namespace engine;
			class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple> {};
			typedef typenames::bases bases;
		}
	}
}

