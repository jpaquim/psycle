// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::template_plugin
#include <psycle/detail/project.private.hpp>
#include "template.hpp"
#include <limits> // std::numeric_limits<real>::min_denorm() infinity()
#include <iostream>
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(template_plugin)

template_plugin::template_plugin(class plugin_library_reference & plugin_library_reference, name_type const & name)
:
	node(plugin_library_reference, name),
	in_port_(*this, "in", /*single_connection_is_identity_transform*/ true),
	side_port_(*this, "side"),
	out_port_(*this, "out")
{
	if(loggers::trace()) {
		std::ostringstream s; s << qualified_name() << " new template plugin";
		loggers::trace()(s.str());
	}
}

void template_plugin::channel_change_notification_from_port(port const & port) {
	if(&port == &out_port_) {
		in_port_.propagate_channels(port.channels());
		side_port_.propagate_channels(port.channels());
	}
	else if(&port == &in_port_) {
		out_port_.propagate_channels(port.channels());
		side_port_.propagate_channels(port.channels());
	}
	else if(&port == &side_port_) {
		in_port_.propagate_channels(port.channels());
		out_port_.propagate_channels(port.channels());
	}
	assert(in_port_.channels() == out_port_.channels());
	assert(in_port_.channels() == side_port_.channels());
}

void template_plugin::seconds_per_event_change_notification_from_port(port const & port) {
	quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port);
	assert(in_port_.seconds_per_event() == out_port_.seconds_per_event())
}

void template_plugin::do_process() {
	if(!out_port_) return;
	if(!in_port_) return;
	assert(&in_port_.buffer());
	assert(&side_port_.buffer());
	assert(&out_port_.buffer());
	buffer & in = in_port_.buffer();
	buffer & out = out_port_.buffer();
	assert(out.size() == in.size())
	if(single_input_ports()) {
		buffer & side(side_port_.buffer());
		assert(out.channels() == side.channels())
		for(std::size_t channel = 0; channel < in.channels() ; ++channel)
			for(std::size_t event = 0; event < in.events() && in[channel][event].index() < in.events() ; ++event)
				out[channel][event].index(event);
				if(side[channel][event].index() == event)
					out[channel][event].sample() *= in[channel][event].sample() * side[channel][event].sample();
				else
					out[channel][event].sample() *= in[channel][event].sample();
	} else {
		for(std::size_t channel = 0; channel < in.channels() ; ++channel)
			for(std::size_t event = 0; event < in.events() && in[channel][event].index() < in.events() ; ++event) {
				out[channel][event].index(event);
				out[channel][event].sample() *= in[channel][event].sample();
			}
	}
	for(std::size_t channel = 0; channel < in.channels() ; ++channel) out[channel].flag(channel::flags::continuous);

	// c++ exception:
	//throw exception("template!", UNIVERSALIS__COMPILER__LOCATION);

	// division by 0:
	//volatile int i = 0; i = 0 / i; // trick so that the compiler does not remove the code when optimizing

	// infinite loop so that we can test interruption signal:
	//while(true);
}

}}

