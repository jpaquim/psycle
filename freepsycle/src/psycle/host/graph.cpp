// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 Johan Boule <bohan@jabber.org>
// copyright 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::host::graph
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "graph.hpp"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace host
	{
		graph::graph(graph::underlying_type & underlying)
		:
			graph_base(underlying)
		{
		}

		#if 0
			node::node
			(
				node::parent_type & parent,
				plugin_resolver & resolver,
				std::string const & type,
				std::string const & name,
				coordinates const & coordinates,
				std::vector<coordinates> & port_coordinates
			)
			:
			try
			{
				node_base(parent, resolver(type, name)),
				plugin_library_reference_(plugin_library_reference),
				coordinates(x, y)
			}
			{
			}
			catch(...)
			{
				throw;
			}
		#endif

		node::node(node::parent_type & parent, node::underlying_type & underlying, node::real /*const*/ x, port::real /*const*/ y)
		:
			node_base(parent, underlying),
			coordinates(x, y)
		{
			int const ports(underlying.single_input_ports().size() + underlying.output_ports().size() + (underlying.multiple_input_port() ? 1 : 0));
			coordinates::real const angle_step(engine::math::pi * 2 / ports);
			coordinates::real const radius(60);
			coordinates::real angle(0);
			for(underlying_type::output_ports_type::const_iterator i(underlying.output_ports().begin()) ; i != underlying.output_ports().end() ; ++i)
			{
				engine::ports::output & output_port(**i);
				coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
//				output_ports_.push_back(new ports::output(*this, output_port, x, y));
				angle += angle_step;
			}
			if(underlying.multiple_input_port())
			{
				typenames::underlying::ports::inputs::multiple & multiple_input_port(*underlying.multiple_input_port());
				coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
//				multiple_input_port_ = new ports::inputs::multiple(*this, multiple_input_port, x, y);
				angle += angle_step;
			}
			for(underlying_type::single_input_ports_type::const_iterator i(underlying.single_input_ports().begin()) ; i != underlying.single_input_ports().end() ; ++i)
			{
				typenames::underlying::ports::inputs::single & single_input_port(**i);
				coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
//				input_ports_.push_back(new ports::inputs::single(*this, single_input_port, x, y));
				angle += angle_step;
			}
		}

		port::port(node & node, port::underlying_type & underlying, port::real /*const*/ x, port::real /*const*/ y)
		:
			port_base(node, underlying),
			coordinates(x, y)
		{}

		namespace ports
		{
			output::output(node & node, output::underlying_type & underlying, output::real /*const*/ x, port::real /*const*/ y)
			: output_base(node, underlying, x, y)
			{}

			input::input(node & node, input::underlying_type & underlying, input::real /*const*/ x, port::real /*const*/ y)
			: input_base(node, underlying, x, y)
			{}
			
			namespace inputs
			{
				single::single(node & node, single::underlying_type & underlying, single::real /*const*/ x, port::real /*const*/ y)
				: single_base(node, underlying, x, y)
				{}

				multiple::multiple(node & node, multiple::underlying_type & underlying, multiple::real /*const*/ x, port::real /*const*/ y)
				: multiple_base(node, underlying, x, y)
				{}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#elif !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace host
	{
		graph::graph(graph::underlying_type & underlying)
		:
			underlying_wrapper_type(underlying)
		{
			for(underlying_type::const_iterator i(underlying.begin()) ; i != underlying.end() ; ++i)
			{
				//underlying_type::const_iterator::type & node(**i);
				//typeof(**i) & node(**i);
				//engine::node & node(**i);
				coordinates::real const x(0), y(0);
				insert(new node(*this, **i, x, y));
			}
		}

		node::node(graph & graph, node::underlying_type & underlying, node::real const & x, node::real const & y)
		:
			underlying_wrapper_type(underlying),
			coordinates(x, y),
			graph_(graph),
			multiple_input_port_(0)
		{
			int const ports(underlying.single_input_ports().size() + underlying.output_ports().size() + (underlying.multiple_input_port() ? 1 : 0));
			coordinates::real const angle_step(engine::math::pi * 2 / ports);
			coordinates::real const radius(60);
			coordinates::real angle(0);
			for(underlying_type::output_ports_type::const_iterator i(underlying.output_ports().begin()) ; i != underlying.output_ports().end() ; ++i)
			{
				engine::ports::output & output_port(**i);
				coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
				output_ports_.push_back(new ports::output(*this, output_port, x, y));
				angle += angle_step;
			}
			if(underlying.multiple_input_port())
			{
				engine::ports::inputs::multiple & multiple_input_port(*underlying.multiple_input_port());
				coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
				multiple_input_port_ = new ports::inputs::multiple(*this, multiple_input_port, x, y);
				angle += angle_step;
			}
			for(underlying_type::single_input_ports_type::const_iterator i(underlying.single_input_ports().begin()) ; i != underlying.single_input_ports().end() ; ++i)
			{
				engine::ports::inputs::single & single_input_port(**i);
				coordinates::real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
				input_ports_.push_back(new ports::inputs::single(*this, single_input_port, x, y));
				angle += angle_step;
			}
			graph.insert(this);
		}

		ports::input * const node::input_port(std::string const & name) const
		{
			if(multiple_input_port() && multiple_input_port()->underlying_instance().name() == name) return multiple_input_port();
			for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i) if((**i).underlying_instance().name() == name) return *i;
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << this->underlying().qualified_name() << " input port not found: " << name;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			return 0;
		}
	
		ports::output * const node::output_port(std::string const & name) const
		{
			for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) if((**i).underlying_instance().name() == name) return *i;
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << this->underlying().qualified_name() << " output port not found: " << name;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			return 0;
		}

		port::port(node & node, port::underlying_type & underlying, port::real const & x, port::real const & y)
		:
			underlying_wrapper_type(underlying),
			coordinates(x, y),
			node_(node)
		{}

		namespace ports
		{
			output::output(node & node, output::underlying_type & underlying, output::real const & x, output::real const & y)
			: underlying_wrapper_type(node, underlying, x, y)
			{}

			input::input(node & node, input::underlying_type & underlying, input::real const & x, input::real const & y)
			: underlying_wrapper_type(node, underlying, x, y)
			{}
			
			namespace inputs
			{
				single::single(node & node, single::underlying_type & underlying, single::real const & x, single::real const & y)
				: underlying_wrapper_type(node, underlying, x, y)
				{}

				multiple::multiple(node & node, multiple::underlying_type & underlying, multiple::real const & x, multiple::real const & y)
				: underlying_wrapper_type(node, underlying, x, y)
				{}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#endif // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
