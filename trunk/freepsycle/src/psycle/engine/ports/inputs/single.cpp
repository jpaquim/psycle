// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::ports::inputs::single
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "single.hpp"
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
				single::single(single::parent_type & parent, name_type const & name, int const & channels)
				:
					single_base(parent, name, channels)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << this->qualified_name() << ": new port input single";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				single::~single() throw()
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
				single::single(engine::node & node, const std::string & name, const int & channels)
				:
					input(node, name, channels),
					output_port_(0)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << qualified_name() << ": new port input single";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					this->node().single_input_ports_.push_back(this);
				}
			
				single::~single() throw()
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << qualified_name() << ": delete port input single";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					disconnect_all();
				}
			
				void single::disconnect_all()
				{
					if(output_port_) disconnect(*output_port_);
				}
			
				void single::connect_internal_side(output & output_port)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "connecting single input port internal side to output port";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(&output_port == this->output_port_)
					{
						if(loggers::warning()())
						{
							std::ostringstream s;
							s << "already connected";
							loggers::warning()(s.str());
						}
						return;
					}
					output_port_ = &output_port;
				}
			
				void single::disconnect_internal_side(output & output_port)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "disconnecting single input port internal side from output port";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(&output_port != this->output_port_)
					{
						if(loggers::warning()())
						{
							std::ostringstream s;
							s << "was not connected";
							loggers::warning()(s.str());
						}
						return;
					}
					output_port_ = 0;
				}
			
				void single::do_propagate_channels() throw(exception)
				{
					if(output_port_) output_port_->propagate_channels_to_node(this->channels());
				}
			
				void single::do_propagate_seconds_per_event()
				{
					if(output_port_) output_port_->propagate_seconds_per_event_to_node(this->seconds_per_event());
				}

				void single::dump(std::ostream & out, int const & tabulations) const
				{
					port::dump(out, /*"in",*/ tabulations);
					if(output_port_) out << ' ' << output_port_->semi_qualified_name();
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
