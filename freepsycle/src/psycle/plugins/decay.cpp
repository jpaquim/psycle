// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::decay
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "decay.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(decay)

decay::decay(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
:
	node(plugin_library_reference, graph, name),
	current_(),
	decay_(), events_per_second_(), seconds_per_event_()
{
	engine::ports::output::create_on_heap(*this, "out", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "pulse", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "decay", boost::cref(1));
}

void decay::seconds_per_event_change_notification_from_port(engine::port const & port) {
	quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port);
	decay_ = std::pow(decay_, events_per_second_ * port.seconds_per_event());
	seconds_per_event_ = port.seconds_per_event();
	events_per_second_ = port.events_per_second();
}

void decay::do_process() throw(engine::exception) {
	if(!have_out()) return;
	PSYCLE__PLUGINS__TEMPLATE_SWITCH__2(do_process_template,
		have_pulse() ? pulse_channel().flag() : channel::flags::empty,
		have_decay() ? decay_channel().flag() : channel::flags::empty
	);
}

template<engine::channel::flags::type pulse_flag, engine::channel::flags::type decay_flag>
void decay::do_process_template() throw(engine::exception) {
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
	if(current_) out_channel().flag(channel::flags::continuous);
	else out_channel().flag(channel::flags::empty);
}

}}
