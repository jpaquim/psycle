// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::ports::output
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "output.hpp"
#include "input.hpp"
#include "../buffer.hpp"
#include "../node.hpp"
#include <typeinfo>

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
			output::output(output::parent_type & parent, name_type const & name, int const & channels)
			:
				output_base(parent, name, channels)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": new port output";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		
			output::~output() throw()
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
			output::output(engine::node & node, const std::string & name, const int & channels)
			:
				port(node, name, channels)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": new port output";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				this->node().output_ports_.push_back(this);
			}
		
			output::~output() throw()
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << qualified_name() << ": delete port output";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				disconnect_all();
			}
		
			void output::connect(input & input_port) throw(exception)
			{
				input_port.connect(*this);
			}
		
			void output::disconnect(input & input_port)
			{
				input_port.disconnect(*this);
			}
		
			void output::disconnect_all()
			{
				while(!input_ports_.empty())
					disconnect(*input_ports_.back());
			}
		
			void output::connect_internal_side(input & input_port)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << "connecting output port internal side to input port";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				input_ports_type::iterator i(std::find(input_ports_.begin(), input_ports_.end(), &input_port));
				if(i != input_ports_.end())
				{
					if(loggers::warning()())
					{
						std::ostringstream s;
						s << "already connected";
						loggers::warning()(s.str());
					}
					return;
				}
				input_ports_.push_back(&input_port);
			}
		
			void output::disconnect_internal_side(input & input_port)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << "disconnecting output port internal side from input port";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				input_ports_type::iterator i(std::find(input_ports_.begin(), input_ports_.end(), &input_port));
				if(i == input_ports_.end())
				{
					if(loggers::warning()())
					{
						std::ostringstream s;
						s << "was not connected";
						loggers::warning()(s.str());
					}
					return;
				}
				input_ports_.erase(i);
			}
		
			void output::do_buffer(engine::buffer * const buffer)
			{
				port::do_buffer(buffer);
				// propagates the new buffer to the connected input ports:
				/*
				for(input_ports_type::const_iterator i(input_ports_.begin()) ; i != input_ports_.end() ; ++i)
					(**i).do_buffer(buffer);
				*/
			}
		
			void output::do_propagate_channels() throw(exception)
			{
				for(input_ports_type::const_iterator i(input_ports_.begin()) ; i != input_ports_.end() ; ++i)
					(**i).propagate_channels_to_node(this->channels());
			}
		
			void output::do_propagate_seconds_per_event()
			{
				for(input_ports_type::const_iterator i(input_ports_.begin()) ; i != input_ports_.end() ; ++i)
					(**i).propagate_seconds_per_event(this->seconds_per_event());
			}
		
			void output::dump(std::ostream & out, const int & tabulations) const
			{
				port::dump(out, /*"out",*/ tabulations);
				for(input_ports_type::const_iterator i(input_ports_.begin()) ; i != input_ports_.end() ; ++i)
					out << ' ' << (**i).semi_qualified_name();
				out << std::endl;
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
