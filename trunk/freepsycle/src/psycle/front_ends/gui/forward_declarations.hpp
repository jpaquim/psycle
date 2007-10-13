///\file
/// forward declarations for the namespace psycle::front_ends::gui.
/// This file is used to avoid mutually recursive dependencies in declarations.
/// It does not have any real declarations, just forward names.
///\see file graph.hpp to #include the real declarations
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

	namespace underlying = engine;

	namespace typenames {
		using namespace gui;
		class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames::typenames> {};
	}
}}}

