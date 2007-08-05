// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief \implementation psycle::plugins::bipolar_filter
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "bipolar_filter.hpp"
namespace psycle
{
	namespace plugins
	{
		bipolar_filter::bipolar_filter(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name, real const & logical_zero)
		:
			node(plugin_library_reference, graph, name),
			logical_zero_(logical_zero)
		{
			engine::ports::inputs::multiple::create(*this, "in", boost::cref(true));
			engine::ports::output::create(*this, "out");
		}
	
		void bipolar_filter::channel_change_notification_from_port(const engine::port & port) throw(engine::exception)
		{
			if(&port == output_ports()[0]) multiple_input_port()->propagate_channels(port.channels());
			else if(&port == multiple_input_port()) output_ports()[0]->propagate_channels(port.channels());
		}
	
		void bipolar_filter::seconds_per_event_change_notification_from_port(const engine::port & port)
		{
			if(&port == output_ports()[0]) multiple_input_port()->propagate_seconds_per_event(port.seconds_per_event());
			else if(&port == multiple_input_port()) output_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
		}
		
		void bipolar_filter::do_process_first() throw(engine::exception)
		{
			assert(&multiple_input_port()->buffer());
			assert(&output_ports()[0]->buffer());
			engine::buffer & in = multiple_input_port()->buffer();
			engine::buffer & out = output_ports()[0]->buffer();
			for(int channel(0) ; channel < in.size() ; ++channel)
				for(int event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event)
					out[channel][event].sample() = logical_zero_;
			do_process();
		}

		void bipolar_filter::do_process() throw(engine::exception)
		{
			assert(&multiple_input_port()->buffer());
			assert(&output_ports()[0]->buffer());
		}
	}
}
