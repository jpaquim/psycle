// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::ports::inputs::multiple
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "multiple.hpp"
#include "../output.hpp"
#include "../../buffer.hpp"
#include "../../node.hpp"

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
			namespace inputs
			{
				multiple::multiple(multiple::parent_type & parent, name_type const & name, bool const & single_connection_is_identity_transform, int const & channels)
				:
					multiple_base(parent, name, channels),
					single_connection_is_identity_transform_(single_connection_is_identity_transform)	
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << qualified_name() << ": new port input multiple";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				multiple::~multiple() throw()
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
			namespace inputs
			{
				multiple::multiple(engine::node & node, std::string const & name, bool const & single_connection_is_identity_transform, int const & channels)
				:
					input(node, name, channels),
					single_connection_is_identity_transform_(single_connection_is_identity_transform)	
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << qualified_name() << " new port input multiple";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					assert(!this->node().multiple_input_port());
					this->node().multiple_input_port(*this);
				}
			
				multiple::~multiple() throw()
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << qualified_name() << " delete port input multiple";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					disconnect_all();
				}
			
				void multiple::disconnect_all()
				{
					while(!output_ports_.empty())
						disconnect(*output_ports_.back());
				}
			
				void multiple::connect_internal_side(output & output_port)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "connecting multiple input port internal side to output port";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					output_ports_type::iterator i(std::find(output_ports_.begin(), output_ports_.end(), &output_port));
					if(i != output_ports_.end())
					{
						if(loggers::warning()())
						{
							std::ostringstream s;
							s << "already connected";
							loggers::warning()(s.str());
						}
						return;
					}
					output_ports_.push_back(&output_port);
				}
			
				void multiple::disconnect_internal_side(output & output_port)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "disconnecting multiple input port internal side from output port";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					output_ports_type::iterator i(std::find(output_ports_.begin(), output_ports_.end(), &output_port));
					if(i == output_ports_.end())
					{
						if(loggers::warning()())
						{
							std::ostringstream s;
							s << "was not connected";
							loggers::warning()(s.str());
						}
						return;
					}
					output_ports_.erase(i);
				}
			
				void multiple::do_propagate_channels() throw(exception)
				{
					for(output_ports_type::iterator i(output_ports_.begin()); i != output_ports_.end() ; ++i)
						(**i).propagate_channels_to_node(this->channels());
				}
			
				void multiple::do_propagate_seconds_per_event()
				{
					for(output_ports_type::iterator i(output_ports_.begin()); i != output_ports_.end() ; ++i)
						(**i).propagate_seconds_per_event_to_node(this->seconds_per_event());
				}

				void multiple::dump(std::ostream & out, const int & tabulations) const
				{
					port::dump(out, /*"in",*/ tabulations);
					for(std::vector<output*>::const_iterator o = output_ports_.begin() ; o != output_ports_.end() ; ++o)
						out << ' ' << (**o).semi_qualified_name();
					out << std::endl;
				}
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
