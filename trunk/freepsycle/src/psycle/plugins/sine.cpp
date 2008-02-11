// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::plugins::sine
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "sine.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(sine)

sine::sine(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
:
	node(plugin_library_reference, graph, name),
	phase_(),
	step_(), frequency_to_step_(),
	amplitude_()
{
	engine::ports::output::create_on_heap(*this, "out", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "phase", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "frequency", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "amplitude", boost::cref(1));
}

void sine::seconds_per_event_change_notification_from_port(engine::port const & port) {
	quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port);
		
	// no easy way to get the value for old_events_per_second
	//this->step_ *= old_events_per_second * port.seconds_per_event();
	
	real const frequency(this->frequency());
	frequency_to_step_ = 2 * engine::math::pi * port.seconds_per_event();
	this->frequency(frequency);
}

void sine::do_process() throw(engine::exception) {
	if(!out_port()) return;
	PSYCLE__PLUGINS__TEMPLATE_SWITCH__3(do_process_template, phase_port(), frequency_port(), amplitude_port());
}

template<bool use_phase, bool use_frequency, bool use_amplitude>
void sine::do_process_template() throw(engine::exception) {
	for(std::size_t
		phase_event(0),
		frequency_event(0),
		amplitude_event(0),
		out_event(0); out_event < out_channel().size(); ++out_event
	) {
		if(use_phase && phase_event < phase_channel().size() && phase_channel()[phase_event].index() == out_event)
			this->phase_ = phase_channel()[phase_event++].sample();
		if(use_frequency && frequency_event < frequency_channel().size() && frequency_channel()[frequency_event].index() == out_event)
			this->frequency(frequency_channel()[frequency_event++].sample());
		if(use_amplitude && amplitude_event < amplitude_channel().size() && amplitude_channel()[amplitude_event].index() == out_event)
			this->amplitude_ = amplitude_channel()[amplitude_event++].sample();
		out_channel()[out_event](out_event, amplitude_ * std::sin(phase_)); // \todo optimize with a cordic algorithm
		phase_ += step_;
	}
	out_channel().flag(channel::flags::continuous);
	phase_ = std::fmod(phase_, 2 * engine::math::pi);
}

}}
