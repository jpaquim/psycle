// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::plugins::template_plugin
#include <psycle/detail/project.private.hpp>
#include "template.hpp"
#include <limits> // std::numeric_limits<real>::min_denorm() infinity()
#include <iostream>
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(template_plugin)

template_plugin::template_plugin(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name)
:
	node(plugin_library_reference, graph, name)
{
	if(loggers::trace()()) {
		std::ostringstream s; s << qualified_name() << " new template plugin";
		loggers::trace()(s.str());
	}
	engine::ports::inputs::multiple::create_on_heap(*this, "in", /*single_connection_is_identity_transform*/ boost::cref(true));
	engine::ports::inputs::single::create_on_heap(*this, "side");
	engine::ports::output::create_on_heap(*this, "out")
}

void template_plugin::channel_change_notification_from_port(const engine::port & port) throw(engine::exception) {
	if(&port == output_ports()[0]) {
		multiple_input_port()->propagate_channels(port.channels());
		input_ports()[0]->propagate_channels(port.channels());
	}
	else if(&port == multiple_input_port()) {
		output_ports()[0]->propagate_channels(port.channels());
		input_ports()[0]->propagate_channels(port.channels());
	}
	else if(&port == input_ports()[0]) {
		multiple_input_port()->propagate_channels(port.channels());
		output_ports()[0]->propagate_channels(port.channels());
	}
	assert(multiple_input_port()->channels() == output_ports()[0]->channels());
	assert(multiple_input_port()->channels() == input_ports()[0]->channels());
}

void template_plugin::seconds_per_event_change_notification_from_port(const engine::port & port) {
	quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port);
	assert(multiple_input_port()->seconds_per_event() == output_ports()[0]->seconds_per_event())
}

void template_plugin::do_process() throw(engine::exception) {
	if(!*output_ports()[0]) return;
	if(!*multiple_input_port()) return;
	assert(&multiple_input_port()->buffer());
	assert(&input_ports()[0]->buffer());
	assert(&output_ports()[0]->buffer());
	engine::buffer & in(multiple_input_port()->buffer());
	engine::buffer & out(output_ports()[0]->buffer());
	assert(out.size() == in.size())
	if(single_input_ports()) {
		engine::buffer & side(input_ports()[0]->buffer());
		assert(out.channels() == side.channels())
		for(std::size_t channel(0) ; channel < in.channels() ; ++channel)
			for(std::size_t event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event)
				out[channel][event].index(event);
				if(side[channel][event].index() == event)
					out[channel][event].sample() *= in[channel][event].sample() * side[channel][event].sample();
				else
					out[channel][event].sample() *= in[channel][event].sample();
	} else {
		for(std::size_t channel(0) ; channel < in.channels() ; ++channel)
			for(std::size_t event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event) {
				out[channel][event].index(event);
				out[channel][event].sample() *= in[channel][event].sample();
			}
	}
	for(std::size_t channel(0) ; channel < in.channels() ; ++channel) out[channel].flag(channel::flags::continuous);

	// c++ exception:
	//throw exception("template!", UNIVERSALIS__COMPILER__LOCATION);

	// division by 0:
	//volatile int i(0); i = 0 / i; // trick so that the compiler does not remove the code when optimizing

	// infinite loop so that we can test interruption signal:
	//while(true);
}

}}

