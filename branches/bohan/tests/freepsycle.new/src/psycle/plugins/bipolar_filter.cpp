// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\\implementation psycle::plugins::bipolar_filter
#include <psycle/detail/project.private.hpp>
#include "bipolar_filter.hpp"
namespace psycle { namespace plugins {

bipolar_filter::bipolar_filter(class plugin_library_reference & plugin_library_reference, name_type const & name, real const & logical_zero)
:
	node(plugin_library_reference, name),
	logical_zero_(logical_zero),
	multiple_input_port_(*this, "in", /*single_connection_is_identity_transform*/ true),
	output_port_(*this, "out")
{}

void bipolar_filter::channel_change_notification_from_port(port const & port) throw(exception) {
	if(&port == &output_port_) multiple_input_port_.propagate_channels(port.channels());
	else if(&port == &multiple_input_port_) output_port_.propagate_channels(port.channels());
	assert(multiple_input_port_.channels() == output_port_.channels());
}

void bipolar_filter::seconds_per_event_change_notification_from_port(port const & port) {
	if(&port == &output_port_) multiple_input_port_.propagate_seconds_per_event(port.seconds_per_event());
	else if(&port == &multiple_input_port_) output_port_.propagate_seconds_per_event(port.seconds_per_event());
	assert(math::roughly_equals(multiple_input_port_.seconds_per_event(), output_port_.seconds_per_event()));
}

void bipolar_filter::do_process_first() throw(exception) {
	if(!output_port_) return;
	if(!multiple_input_port_) return;
	assert(&multiple_input_port_.buffer());
	assert(&output_port_.buffer());
	buffer & in = multiple_input_port_.buffer();
	buffer & out = output_port_.buffer();
	assert(out.channels() == in.channels());
	for(std::size_t channel = 0; channel < in.channels(); ++channel)
		for(std::size_t event = 0; event < in.events() && in[channel][event].index() < in.events(); ++event) {
			// not needed because in and out have the same buffer: out[channel][event].index(event);
			out[channel][event].sample() = logical_zero_;
		}
	do_process();
}

void bipolar_filter::do_process() throw(exception) {
	if(!output_port_) return;
	if(!multiple_input_port_) return;
	buffer & in = multiple_input_port_.buffer();
	buffer & out = output_port_.buffer();
	for(std::size_t channel = 0; channel < in.channels(); ++channel) out[channel].flag(in[channel].flag());
}

}}
