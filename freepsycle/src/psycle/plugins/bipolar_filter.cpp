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
	in_port_(*this, "in", /*single_connection_is_identity_transform*/ true),
	out_port_(*this, "out")
{}

void bipolar_filter::channel_change_notification_from_port(port const & port) {
	if(&port == &out_port_) in_port_.propagate_channels(port.channels());
	else if(&port == &in_port_) out_port_.propagate_channels(port.channels());
	assert(in_port_.channels() == out_port_.channels());
}

void bipolar_filter::seconds_per_event_change_notification_from_port(port const & port) {
	if(&port == &out_port_) in_port_.propagate_seconds_per_event(port.seconds_per_event());
	else if(&port == &in_port_) out_port_.propagate_seconds_per_event(port.seconds_per_event());
	assert(math::roughly_equals(in_port_.seconds_per_event(), out_port_.seconds_per_event()));
}

void bipolar_filter::do_process_first() {
	if(!out_port_) return;
	if(!in_port_) return;
	assert(&in_port_.buffer());
	assert(&out_port_.buffer());
	buffer & in = in_port_.buffer();
	buffer & out = out_port_.buffer();
	assert(out.channels() == in.channels());
	#pragma omp parallel for
	for(std::size_t channel = 0; channel < in.channels(); ++channel)
		for(std::size_t event = 0; event < in.events() && in[channel][event].index() < in.events(); ++event) {
			// not needed because in and out have the same buffer: out[channel][event].index(event);
			out[channel][event].sample() = logical_zero_;
		}
	do_process();
}

void bipolar_filter::do_process() {
	if(!out_port_) return;
	if(!in_port_) return;
	buffer & in = in_port_.buffer();
	buffer & out = out_port_.buffer();
	for(std::size_t channel = 0; channel < in.channels(); ++channel) out[channel].flag(in[channel].flag());
}

}}
