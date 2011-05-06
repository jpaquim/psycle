// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::decay
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
	channel::flags::type const pulse_flag = have_pulse() ? pulse_channel().flag() : channel::flags::empty;
	channel::flags::type const decay_flag = have_decay() ? decay_channel().flag() : channel::flags::empty;
	#if defined DIVERSALIS__COMPILER__FEATURE__CXXOX
		do_process_template_switch<decay>(pulse_flag, decay_flag);
	#else
		PSYCLE__ENGINE__TEMPLATE_SWITCH(do_process_template, (pulse_flag)(decay_flag));
	#endif
}

template<
	channel::flags::type pulse_flag,
	channel::flags::type decay_flag
>
void decay::do_process_template() {
	for(std::size_t
		pulse_event(0),
		decay_event(0),
		out_event(0); out_event < out_channel().size(); ++out_event
	) {
		switch(pulse_flag) {
			case channel::flags::continuous:
				this->current_ = pulse_channel()[out_event].sample();
			break;
			case channel::flags::discrete:
				if(pulse_event < pulse_channel().size() && pulse_channel()[pulse_event].index() == out_event)
					this->current_ = pulse_channel()[pulse_event++].sample();
			break;
			case channel::flags::empty: default: /* nothing */ ;
		}
		switch(decay_flag) {
			case channel::flags::continuous:
				this->decay_per_second(decay_channel()[out_event].sample());
			break;
			case channel::flags::discrete:
				if(decay_event < decay_channel().size() && decay_channel()[decay_event].index() == out_event)
					this->decay_per_second(decay_channel()[decay_event++].sample());
			break;
			case channel::flags::empty: default: /* nothing */ ;
		}
		out_channel()[out_event](out_event, current_);
		current_ *= decay_;
	}
	///\todo flush to zero
	if(current_) out_channel().flag(channel::flags::continuous);
	else out_channel().flag(channel::flags::empty);
}

}}
