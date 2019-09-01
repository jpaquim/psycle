// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\implementation psycle::engine::ports::output
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "output.hpp"
#include "input.hpp"
#include "../buffer.hpp"
#include "../node.hpp"
#include <typeinfo>
namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			output::output(output::parent_type & parent, name_type const & name, int const & channels)
			:
				output_type(parent, name, channels)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": new port output";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		
			output::~output()
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": delete port output";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		
			void output::do_buffer(engine::buffer * const buffer)
			{
				port::do_buffer(buffer);
				// propagates the new buffer to the connected input ports:
				/*
				for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i)
					(**i).do_buffer(buffer);
				*/
			}
		
			void output::do_propagate_channels() throw(exception)
			{
				for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i)
					(**i).propagate_channels_to_node(this->channels());
			}
		
			void output::do_propagate_seconds_per_event()
			{
				for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i)
					(**i).propagate_seconds_per_event(this->seconds_per_event());
			}
		
			void output::dump(std::ostream & out, const int & tabulations) const
			{
				port::dump(out, /*"out",*/ tabulations);
				for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i)
					out << ' ' << (**i).semi_qualified_name();
				out << std::endl;
			}
		}
	}
}
