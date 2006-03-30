// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::ports::input
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "input.hpp"
#include "output.hpp"
#include "../buffer.hpp"
#include "../node.hpp"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			input::input(input::parent_type & parent, name_type const & name, int const & channels)
			:
				input_base(parent, name, channels)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": new port input";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		
			input::~input() throw()
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
				input_base::connect(output_port);
			}
		
			void input::disconnect(typenames::ports::output & output_port)
			{
				if(loggers::information()())
				{
					std::ostringstream s;
					s << "disconnecting output port " << output_port.qualified_name() << " from input port " << this->qualified_name();
					loggers::information()(s.str());
				}
				input_base::disconnect(output_port);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#else // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			input::input(engine::node & node, const std::string & name, const int & channels)
			:
				port(node, name, channels)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": new port input";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		
			input::~input() throw()
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": delete port input";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				//disconnect_all();
			}
		
			void input::connect(output & output_port) throw(exception)
			{
				if(loggers::information()())
				{
					std::ostringstream s;
					s << "connecting output port " << output_port.qualified_name() << " to input port " << this->qualified_name();
					loggers::information()(s.str());
				}
				assert("ports must belong to different nodes:" && &output_port.node() != &this->node());
				assert("nodes of both ports must belong to the same graph:" && &output_port.node().graph_instance() == &this->node().graph_instance());
				output_port.connect_internal_side(*this);
				this->connect_internal_side(output_port);
				port::connect(output_port);
			}
		
			void input::disconnect(output & output_port)
			{
				if(loggers::information()())
				{
					std::ostringstream s;
					s << "disconnecting output port " << output_port.qualified_name() << " from input port " << this->qualified_name();
					loggers::information()(s.str());
				}
				this->disconnect_internal_side(output_port);
				output_port.disconnect_internal_side(*this);
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
