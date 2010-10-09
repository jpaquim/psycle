// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::sine
#include <psycle/detail/project.private.hpp>
#include "sine.hpp"
#include <psycle/helpers/math.hpp>
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(sine)

sine::sine(class plugin_library_reference & plugin_library_reference, name_type const & name)
:
	node(plugin_library_reference, name),
	phase_(), step_(), freq_to_step_(), amp_(),
	out_port_(*this, "out", 1),
	phase_port_(*this, "phase", 1),
	freq_port_(*this, "frequency", 1),
	amp_port_(*this, "amplitude", 1)
{}

void sine::seconds_per_event_change_notification_from_port(engine::port const & port) {
	quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port);
		
	// no easy way to get the value for old_events_per_second
	//this->step_ *= old_events_per_second * port.seconds_per_event();
	
	real const freq = this->freq();
	freq_to_step_ = 2 * engine::math::pi * port.seconds_per_event();
	this->freq(freq);
}

void sine::do_process() throw(engine::exception) {
	if(!out_port_) return;
	PSYCLE__PLUGINS__TEMPLATE_SWITCH(do_process_template,
		(phase_port_ ? phase_chn().flag() : channel::flags::empty)
		( freq_port_ ?  freq_chn().flag() : channel::flags::empty)
		(  amp_port_ ?   amp_chn().flag() : channel::flags::empty)
	);
}

template<engine::channel::flags::type phase_flag, engine::channel::flags::type freq_flag, engine::channel::flags::type amp_flag>
void sine::do_process_template() {
	for(std::size_t phase_evt = 0, freq_evt = 0, amp_evt = 0, out_evt = 0; out_evt < out_chn().size(); ++out_evt) {
		if(phase_flag == channel::flags::continuous || (
				phase_flag != channel::flags::empty &&
				phase_evt < phase_chn().size() && phase_chn()[phase_evt].index() == out_evt
			)
		) phase_ = std::fmod(phase_chn()[phase_evt++].sample() + engine::math::pi, 2 * engine::math::pi) - engine::math::pi;

		switch(freq_flag) {
			case channel::flags::continuous:
				freq(freq_chn()[out_evt].sample());
			break;
			case channel::flags::discrete:
				if(freq_evt < freq_chn().size() && freq_chn()[freq_evt].index() == out_evt)
					freq(freq_chn()[freq_evt++].sample());
			break;
			case channel::flags::empty: default: /* nothing */ ;
		}

		switch(amp_flag) {
			case channel::flags::continuous:
				amp_ = amp_chn()[out_evt].sample();
			break;
			case channel::flags::discrete:
				if(amp_evt < amp_chn().size() && amp_chn()[amp_evt].index() == out_evt)
					amp_ = amp_chn()[amp_evt++].sample();
			break;
			case channel::flags::empty: default: /* nothing */ ;
		}

		out_chn()[out_evt](out_evt, amp_ * helpers::math::fast_sin<2>(phase_));
		phase_ += step_;
		if(phase_ > engine::math::pi) phase_ -= 2 * engine::math::pi;
	}
	out_chn().flag(channel::flags::continuous);
}

}}
