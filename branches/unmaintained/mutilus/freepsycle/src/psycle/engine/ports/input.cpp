// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\implementation psycle::engine::ports::input
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "input.hpp"
#include "output.hpp"
#include "../buffer.hpp"
#include "../node.hpp"
namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			input::input(input::parent_type & parent, name_type const & name, int const & channels)
			:
				input_type(parent, name, channels)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": new port input";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		
			input::~input()
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": delete port input";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				//disconnect_all();
			}
		
			void input::connect(typenames::ports::output & output_port) throw(exception)
			{
				if(loggers::information()())
				{
					std::ostringstream s;
					s << "connecting output port " << output_port.qualified_name() << " to input port " << this->qualified_name();
					loggers::information()(s.str());
				}
				input_type::connect(output_port);
			}
		
			void input::disconnect(typenames::ports::output & output_port)
			{
				if(loggers::information()())
				{
					std::ostringstream s;
					s << "disconnecting output port " << output_port.qualified_name() << " from input port " << this->qualified_name();
					loggers::information()(s.str());
				}
				input_type::disconnect(output_port);
			}
		}
	}
}
