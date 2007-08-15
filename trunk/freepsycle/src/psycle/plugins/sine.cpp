// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief \implementation psycle::plugins::sine
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "sine.hpp"
namespace psycle
{
	namespace plugins
	{
		PSYCLE__PLUGINS__NODE_INSTANCIATOR(sine)

		sine::sine(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
		:
			node(plugin_library_reference, graph, name),
			phase_(0),
			step_(0),
			frequency_to_step_(0)
		{
			engine::ports::output::create(*this, "out", boost::cref(1));
			engine::ports::inputs::single::create(*this, "frequency", boost::cref(1));
			engine::ports::inputs::single::create(*this, "phase", boost::cref(1));
		}

		namespace
		{
			namespace ports
			{
				struct outputs
				{
					enum output
					{
						out
					};
				};
				
				struct inputs
				{
					enum input
					{
						frequency,
						phase
					};
				};
			}
		}

		void sine::seconds_per_event_change_notification_from_port(engine::port const & port)
		{
			if(&port == single_input_ports()[0]) output_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
			else if(&port == output_ports()[0]) single_input_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
			//sample frequency(frequency_to_step_ ? step_ / frequency_to_step_ : 0);
			frequency_to_step_ = 2 * engine::math::pi * port.seconds_per_event();
			this->frequency(100);//frequency); \todo remove
		}
	
		void sine::do_process() throw(engine::exception)
		{
			engine::buffer::channel & out(output_ports()[0]->buffer()[0]);
			for(unsigned int frequency_event(0), out_event(0) ; out_event < out.size() ; ++out_event)
			{
				frequency_event;//if(frequency[frequency_event].index() == out_event)
					//this->frequency(frequency[frequency_event++].sample());
				out[out_event].index(out_event);
				out[out_event].sample(0.3 * std::sin(phase_)); // \todo optimize with a cordic algorithm
				phase_ += step_;
				step_ *= 1.00001; // \todo remove
			}
			phase_ = std::fmod(phase_, 2 * engine::math::pi);
		}
	}
}
