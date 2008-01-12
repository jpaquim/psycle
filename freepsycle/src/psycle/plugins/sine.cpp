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
	phase_(0),
	step_(0),
	frequency_to_step_(0),
	amplitude_(0.2)
{
	engine::ports::output::create_on_heap(*this, "out", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "frequency", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "phase", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "amplitude", boost::cref(1));
}

void sine::seconds_per_event_change_notification_from_port(engine::port const & port) {
	if(&port == single_input_ports()[0]) output_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
	else if(&port == output_ports()[0]) single_input_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
	real frequency(this->frequency());
	frequency_to_step_ = 2 * engine::math::pi * port.seconds_per_event();
	this->frequency(frequency);
}

void sine::do_process() throw(engine::exception) {
	if(!have_out()) return;
	if(!have_phase()) {
		if(!have_frequency()) {
			if(!have_amplitude()) do_process_template<false, false, false>();
			else do_process_template<false, false, true>();
		} else if(!have_amplitude()) do_process_template<false, true, false>();
		else do_process_template<false, true, true>();
	} else if(!have_frequency()) {
		if(!have_amplitude()) do_process_template<true, false, false>();
		else do_process_template<true, false, true>();
	} else if(!have_amplitude()) do_process_template<true, true, false>();
	else do_process_template<true, true, true>();
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
	phase_ = std::fmod(phase_, 2 * engine::math::pi);
}

}}
