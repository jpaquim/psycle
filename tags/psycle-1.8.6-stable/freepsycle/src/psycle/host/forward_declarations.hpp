// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
/// forward declarations for the namespace psycle::host.
/// This file is used to avoid mutually recursive dependencies in declarations.
/// It does not have any real declarations, just forward names.
///\see file host.hpp to #include the real declarations
#pragma once
#include <psycle/engine/forward_declarations.hpp>
namespace psycle {

/// functionalities used by the host side only, not by the plugins.
namespace host {
	namespace underlying = engine;

	class graph;
	class node;
	class port;
	namespace ports {
		class output;
		class input;
		namespace inputs {
			class single;
			class multiple;
		}
	}

	namespace typenames {
		using namespace host;
		class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames::typenames> {};
	}
}

}

