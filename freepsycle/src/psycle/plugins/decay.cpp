// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

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
	if(&port == single_input_ports()[0]) {
		single_input_ports()[1]->propagate_seconds_per_event(port.seconds_per_event());
		output_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
	} else if(&port == single_input_ports()[1]) {
		single_input_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
		output_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
	} else if(&port == output_ports()[0]) {
		single_input_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
		single_input_ports()[1]->propagate_seconds_per_event(port.seconds_per_event());
	}
	decay_ = std::pow(decay_, events_per_second_ * port.seconds_per_event());
	seconds_per_event_ = port.seconds_per_event();
	events_per_second_ = port.events_per_second();
}

void decay::do_process() throw(engine::exception) {
	if(!have_out()) return;
	PSYCLE__PLUGINS__TEMPLATE_SWITCH__2(do_process_template, have_pulse(), have_decay());
}

template<bool use_pulse, bool use_decay>
void decay::do_process_template() throw(engine::exception) {
	for(std::size_t
		pulse_event(0),
		decay_event(0),
		out_event(0); out_event < out_channel().size(); ++out_event
	) {
		if(use_pulse && pulse_event < pulse_channel().size() && pulse_channel()[pulse_event].index() == out_event)
			this->current_ = pulse_channel()[pulse_event++].sample();
		if(use_decay && decay_event < decay_channel().size() && decay_channel()[decay_event].index() == out_event)
			this->decay_per_second(decay_channel()[decay_event++].sample());
		out_channel()[out_event](out_event, current_);
		current_ *= decay_;
	}
}

}}
