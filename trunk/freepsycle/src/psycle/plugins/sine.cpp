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
	frequency_to_step_(0)
{
	engine::ports::output::create_on_heap(*this, "out", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "frequency", boost::cref(1));
	engine::ports::inputs::single::create_on_heap(*this, "phase", boost::cref(1));
}

namespace {
	namespace ports {
		struct outputs { enum output {
			out
		}; };
		struct inputs { enum input {
			frequency,
			phase
		}; };
	}
}

void sine::seconds_per_event_change_notification_from_port(engine::port const & port) {
	if(&port == single_input_ports()[0]) output_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
	else if(&port == output_ports()[0]) single_input_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
	//real frequency(frequency_to_step_ ? step_ / frequency_to_step_ : 0);
	frequency_to_step_ = 2 * engine::math::pi * port.seconds_per_event();
	this->frequency(100);//frequency); \todo remove
}

const real todo(1.000001); // \todo remove

void sine::do_process() throw(engine::exception) {
	if(!have_out()) return;
	if(!have_frequency()) goto const_frequency;
	if(!have_phase()) goto const_phase;
	
	//none_const:
		for(std::size_t frequency_event(0), phase_event(0), out_event(0); out_event < out_channel().size(); ++out_event) {
			if(frequency_event < frequency_channel().size() && frequency_channel()[frequency_event].index() == out_event)
				this->frequency(frequency_channel()[frequency_event++].sample());
			if(phase_event < phase_channel().size() && phase_channel()[phase_event].index() == out_event)
				this->phase_ = phase_channel()[phase_event++].sample();
			out_channel()[out_event](out_event, 0.3 * std::sin(phase_)); // \todo optimize with a cordic algorithm
			phase_ += step_;
			step_ *= todo; // \todo remove
		}
	goto modulo;
	
	const_frequency:
		if(!have_phase()) goto all_const;

		for(std::size_t phase_event(0), out_event(0); out_event < out_channel().size(); ++out_event) {
			if(phase_event < phase_channel().size() && phase_channel()[phase_event].index() == out_event)
				this->phase_ = phase_channel()[phase_event++].sample();
			out_channel()[out_event](out_event, 0.3 * std::sin(phase_)); // \todo optimize with a cordic algorithm
			phase_ += step_;
			step_ *= todo; // \todo remove
		}
	goto modulo;
	
	const_phase: {
		bool b = true;
		for(std::size_t frequency_event(0), out_event(0); out_event < out_channel().size(); ++out_event) {
			if(b && frequency_event < frequency_channel().size() && frequency_channel()[frequency_event].index() == out_event)
				this->frequency(frequency_channel()[frequency_event++].sample());
			else b = false; ///\todo goto const loop
			out_channel()[out_event](out_event, 0.3 * std::sin(phase_)); // \todo optimize with a cordic algorithm
			phase_ += step_;
			step_ *= todo; // \todo remove
		}
	}
	goto modulo;
		
	all_const:
		for(std::size_t out_event(0) ; out_event < out_channel().size() ; ++out_event) {
			out_channel()[out_event](out_event, 0.3 * std::sin(phase_)); // \todo optimize with a cordic algorithm
			phase_ += step_;
			step_ *= todo; // \todo remove
		}
	
	modulo: phase_ = std::fmod(phase_, 2 * engine::math::pi);
}

}}

