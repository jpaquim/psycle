// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include <psycle/detail/project.hpp>
#include <psycle/engine/forward_declarations.hpp>
namespace psycle { namespace front_ends { namespace gui {

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

//namespace underlying = engine;
namespace underlying { using namespace engine; }

class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, engine::typenames> {};
typedef typenames::bases bases;

}}}
