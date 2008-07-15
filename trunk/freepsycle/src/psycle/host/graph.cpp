// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::host::graph
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "graph.hpp"
namespace psycle { namespace host {

graph::graph(graph::underlying_type & underlying)
:
	graph_type(underlying)
{}

#if 0
	node::node(
		node::parent_type & parent,
		plugin_resolver & resolver,
		std::string const & type,
		std::string const & name,
		coordinates const & coordinates,
		std::vector<coordinates> & port_coordinates
	) : try {
		node_type(parent, resolver(type, name)),
		plugin_library_reference_(plugin_library_reference),
		coordinates(x, y)
	} {} catch(...) {
		throw;
	}
#endif

node::node(node::parent_type & parent, node::underlying_type & underlying, node::real /*const*/ x, port::real /*const*/ y)
:
	node_type(parent, underlying),
	coordinates(x, y)
{}

void node::after_construction() {
	node_type::after_construction();
	int const ports(single_input_ports().size() + output_ports().size() + (multiple_input_port() ? 1 : 0));
	coordinates::real const angle_step(engine::math::pi * 2 / ports);
	coordinates::real const radius(60);
	coordinates::real angle(0);
	for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) {
		typenames::ports::output & output_port(**i);
		coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		output_port(x, y);
		angle += angle_step;
	}
	if(multiple_input_port()) {
		typenames::ports::inputs::multiple & multiple_input_port(*this->multiple_input_port());
		coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		multiple_input_port(x, y);
		angle += angle_step;
	}
	for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) {
		typenames::ports::inputs::single & single_input_port(**i);
		coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		single_input_port(x, y);
		angle += angle_step;
	}
}

port::port(node & node, port::underlying_type & underlying, port::real /*const*/ x, port::real /*const*/ y)
:
	port_type(node, underlying),
	coordinates(x, y)
{}

namespace ports {
	output::output(node & node, output::underlying_type & underlying, output::real /*const*/ x, port::real /*const*/ y)
	: output_type(node, underlying, x, y)
	{}

	input::input(node & node, input::underlying_type & underlying, input::real /*const*/ x, port::real /*const*/ y)
	: input_type(node, underlying, x, y)
	{}
	
	namespace inputs {
		single::single(node & node, single::underlying_type & underlying, single::real /*const*/ x, port::real /*const*/ y)
		: single_type(node, underlying, x, y)
		{}

		multiple::multiple(node & node, multiple::underlying_type & underlying, multiple::real /*const*/ x, port::real /*const*/ y)
		: multiple_type(node, underlying, x, y)
		{}
}}}}
