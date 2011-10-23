// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <psycle/engine/forward_declarations.hpp>
namespace psycle {

namespace host {
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
}

}
