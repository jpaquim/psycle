// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#include <psycle/detail/project.private.hpp>
#include "graph.hpp"
namespace psycle { namespace host {

node::node(class graph & graph, node::underlying_type & underlying, node::real /*const*/ x, port::real /*const*/ y)
:
	bases::node(graph, underlying),
	coordinates(x, y)
{}

void node::after_construction() {
	bases::node::after_construction();
	int const ports(single_input_ports().size() + output_ports().size() + (multiple_input_port() ? 1 : 0));
	coordinates::real const angle_step(engine::math::pi * 2 / ports);
	coordinates::real const radius(60);
	coordinates::real angle(0);
	for(output_ports_type::const_iterator i(output_ports().begin()), e(output_ports().end()); i != e; ++i) {
		ports::output & output_port(**i);
		coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		output_port(x, y);
		angle += angle_step;
	}
	if(multiple_input_port()) {
		ports::inputs::multiple & multiple_input_port(*this->multiple_input_port());
		coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		multiple_input_port(x, y);
		angle += angle_step;
	}
	for(single_input_ports_type::const_iterator i(single_input_ports().begin()), e(single_input_ports().end()) ; i != e; ++i) {
		ports::inputs::single & single_input_port(**i);
		coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		single_input_port(x, y);
		angle += angle_step;
	}
}

port::port(class node & node, port::underlying_type & underlying, port::real /*const*/ x, port::real /*const*/ y)
:
	bases::port(node, underlying),
	coordinates(x, y)
{}

namespace ports {
	output::output(class node & node, output::underlying_type & underlying, output::real /*const*/ x, port::real /*const*/ y)
	: bases::ports::output(node, underlying, x, y)
	{}

	input::input(class node & node, input::underlying_type & underlying, input::real /*const*/ x, port::real /*const*/ y)
	: bases::ports::input(node, underlying, x, y)
	{}
	
	namespace inputs {
		single::single(class node & node, single::underlying_type & underlying, single::real /*const*/ x, port::real /*const*/ y)
		: bases::ports::inputs::single(node, underlying, x, y)
		{}

		multiple::multiple(class node & node, multiple::underlying_type & underlying, multiple::real /*const*/ x, port::real /*const*/ y)
		: bases::ports::inputs::multiple(node, underlying, x, y)
		{}
}}}}
