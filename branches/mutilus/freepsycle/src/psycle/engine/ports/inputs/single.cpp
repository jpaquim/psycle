// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\implementation psycle::engine::ports::inputs::single
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "single.hpp"
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
				single::single(single::parent_type & parent, name_type const & name, int const & channels)
				:
					single_type(parent, name, channels)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << this->qualified_name() << ": new port input single";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				single::~single()
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << this->qualified_name() << ": delete port input single";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				void single::do_propagate_channels() throw(exception)
				{
					if(output_port()) output_port()->propagate_channels_to_node(this->channels());
				}
			
				void single::do_propagate_seconds_per_event()
				{
					if(output_port()) output_port()->propagate_seconds_per_event_to_node(this->seconds_per_event());
				}

				void single::dump(std::ostream & out, int const & tabulations) const
				{
					port::dump(out, /*"in",*/ tabulations);
					if(output_port()) out << ' ' << output_port()->semi_qualified_name();
					out << std::endl;
				}
			}
		}
	}
}
