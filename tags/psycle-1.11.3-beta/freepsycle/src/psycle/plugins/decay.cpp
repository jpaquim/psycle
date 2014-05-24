// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#include <psycle/detail/project.private.hpp>
#include "decay.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(decay)

decay::decay(class plugin_library_reference & plugin_library_reference, name_type const & name)
:
	node(plugin_library_reference, name),
	current_(),
	decay_(), events_per_second_(), seconds_per_event_(),
	out_port_(*this, "out", 1),
	pulse_port_(*this, "pulse", 1),
	decay_port_(*this, "decay", 1)
{}

void decay::seconds_per_event_change_notification_from_port(engine::port const & port) {
	quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port);
	decay_ = std::pow(decay_, events_per_second_ * port.seconds_per_event());
	seconds_per_event_ = port.seconds_per_event();
	events_per_second_ = port.events_per_second();
}

void decay::do_process() {
	if(!have_out()) return;
	buffer::flags const pulse_flag = have_pulse() ? pulse_port_.buffer().flag() : buffer::flags::empty;
	buffer::flags const decay_flag = have_decay() ? decay_port_.buffer().flag() : buffer::flags::empty;
	do_process_template_switch(*this, pulse_flag, decay_flag);
}

template<
	buffer::flags pulse_flag,
	buffer::flags decay_flag
>
void decay::do_process_template() {
	for(
		std::size_t pulse_event = 0, decay_event = 0, out_event = 0, end = out_port_.buffer().events();
		out_event < end; ++out_event
	) {
		switch(pulse_flag) {
			case buffer::flags::continuous:
				this->current_ = pulse_port_.buffer().sample(out_event);
			break;
			case buffer::flags::discrete:
				if(pulse_event < end && pulse_port_.buffer().index(pulse_event) == out_event)
					current_ = pulse_port_.buffer().sample(pulse_event++);
			break;
			case buffer::flags::empty: default: /* nothing */ ;
		}
		switch(decay_flag) {
			case buffer::flags::continuous:
				decay_per_second(decay_port_.buffer().sample(out_event));
			break;
			case buffer::flags::discrete:
				if(decay_event < end && decay_port_.buffer().index(decay_event) == out_event)
					decay_per_second(decay_port_.buffer().sample(decay_event++));
			break;
			case buffer::flags::empty: default: /* nothing */ ;
		}
		out_port_.buffer().index(out_event) = out_event;
		out_port_.buffer().sample(out_event) = current_;
		current_ *= decay_;
	}
	// TODO flush to zero
	out_port_.buffer().flag(current_ ? buffer::flags::continuous : buffer::flags::empty);
}

}}
