// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\implementation psycle::engine::ports::inputs::multiple
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "multiple.hpp"
#include "../output.hpp"
#include "../../buffer.hpp"
#include "../../node.hpp"
namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			namespace inputs
			{
				multiple::multiple(multiple::parent_type & parent, name_type const & name, bool const & single_connection_is_identity_transform, int const & channels)
				:
					multiple_type(parent, name, channels),
					single_connection_is_identity_transform_(single_connection_is_identity_transform)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << qualified_name() << ": new port input multiple";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				multiple::~multiple()
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << qualified_name() << ": delete port input multiple";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				void multiple::do_propagate_channels() throw(exception)
				{
					for(output_ports_type::const_iterator i(output_ports().begin()); i != output_ports().end() ; ++i)
						(**i).propagate_channels_to_node(this->channels());
				}
			
				void multiple::do_propagate_seconds_per_event()
				{
					for(output_ports_type::const_iterator i(output_ports().begin()); i != output_ports().end() ; ++i)
						(**i).propagate_seconds_per_event_to_node(this->seconds_per_event());
				}

				void multiple::dump(std::ostream & out, int const & tabulations) const
				{
					port::dump(out, /*"in",*/ tabulations);
					for(std::vector<output*>::const_iterator i = output_ports().begin() ; i != output_ports().end() ; ++i)
						out << ' ' << (**i).semi_qualified_name();
					out << std::endl;
				}
			}
		}
	}
}
