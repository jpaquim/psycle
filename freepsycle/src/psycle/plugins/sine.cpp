// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::plugins::sine
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "sine.hpp"
#include <psycle/helpers/math/sine.hpp>
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
	PSYCLE__PLUGINS__TEMPLATE_SWITCH__3(do_process_template,
		phase_port()     ?     phase_channel().flag() : channel::flags::empty,
		frequency_port() ? frequency_channel().flag() : channel::flags::empty,
		amplitude_port() ? amplitude_channel().flag() : channel::flags::empty
	);
}

template<engine::channel::flags::type phase_flag, engine::channel::flags::type frequency_flag, engine::channel::flags::type amplitude_flag>
void sine::do_process_template() throw(engine::exception) {
	for(std::size_t
		phase_event(0),
		frequency_event(0),
		amplitude_event(0),
		out_event(0); out_event < out_channel().size(); ++out_event
	) {
		if(phase_flag == channel::flags::continuous || phase_flag != channel::flags::empty &&
			phase_event < phase_channel().size() && phase_channel()[phase_event].index() == out_event
		) this->phase_ = std::fmod(phase_channel()[phase_event++].sample() + engine::math::pi, 2 * engine::math::pi) - engine::math::pi;

		switch(frequency_flag) {
			case channel::flags::continuous:
				this->frequency(frequency_channel()[out_event].sample());
			break;
			case channel::flags::discrete:
				if(frequency_event < frequency_channel().size() && frequency_channel()[frequency_event].index() == out_event)
					this->frequency(frequency_channel()[frequency_event++].sample());
			break;
			case channel::flags::empty: default: /* nothing */ ;
		}

		switch(amplitude_flag) {
			case channel::flags::continuous:
				this->amplitude_ = amplitude_channel()[out_event].sample();
			break;
			case channel::flags::discrete:
				if(amplitude_event < amplitude_channel().size() && amplitude_channel()[amplitude_event].index() == out_event)
					this->amplitude_ = amplitude_channel()[amplitude_event++].sample();
			break;
			case channel::flags::empty: default: /* nothing */ ;
		}

		out_channel()[out_event](out_event, amplitude_ * helpers::math::fast_sin<2>(phase_));
		phase_ += step_;
		if(phase_ > engine::math::pi) phase_ -= 2 * engine::math::pi;
	}
	out_channel().flag(channel::flags::continuous);
}

}}
