// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

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

void sine::seconds_per_event_change_notification_from_port(port const & port) {
	quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port);
		
	// no easy way to get the value for old_events_per_second
	//this->step_ *= old_events_per_second * port.seconds_per_event();
	
	real const freq = this->freq();
	freq_to_step_ = 2 * math::pi * port.seconds_per_event();
	this->freq(freq);
}

void sine::do_process() {
	if(!out_port_) return;
	channel::flags::type const phase_flag = phase_port_ ? phase_chn().flag() : channel::flags::empty;
	channel::flags::type const  freq_flag =  freq_port_ ?  freq_chn().flag() : channel::flags::empty;
	channel::flags::type const   amp_flag =   amp_port_ ?   amp_chn().flag() : channel::flags::empty;
	#if defined DIVERSALIS__COMPILER__FEATURE__CXX0X
		#if 1
			do_process_template_switch(*this, phase_flag, freq_flag, amp_flag);
		#else
			ports::inputs::single * ports[] = {&phase_port_, &freq_port_, &amp_port_};
			do_process_split<sizeof ports / sizeof *ports>(ports);
			out_chn().flag(channel::flags::continuous);
		#endif
	#else
		PSYCLE__ENGINE__TEMPLATE_SWITCH(do_process_template, (phase_flag)(freq_flag)(amp_flag));
	#endif
	
}

#if defined DIVERSALIS__COMPILER__FEATURE__CXX0X
	template<int Ports>
	void sine::do_process_split(ports::inputs::single * ports[Ports]) {
		std::size_t const size = out_chn().size();
		std::size_t is[Ports];
		std::size_t i = 0;
		for(std::size_t i = 0; i < Ports; ++i) is[i] = 0;
		while(i < size) {
			std::size_t end = size;
			bool in[Ports];
			for(int p = 0; p < Ports; ++p) {
				in[p] = false;
				ports::inputs::single & port = *ports[p];
				if(port) {
					auto chn = port.buffer()[0]; ///\todo flag the buffer, not the channel
					switch(chn.flag()) {
						case channel::flags::continuous:
							in[p] = true;
						break;
						case channel::flags::discrete:
							if(is[p] < chn.size()) {
								if(chn[is[p]].index() == i) {
									{ std::ostringstream s; s << "pulse i" << i << " p" << p << " is" << is[p]; loggers::trace()(s.str()); }
									end = i;
									++is[p];
									in[p] = true;
								} else if(chn[is[p]].index() < end) {
									end = chn[is[p]].index();
									{ std::ostringstream s; s << "end i" << i << " p" << p << " e" << end; loggers::trace()(s.str()); }
								}
							}
					}
				}
			}
			{ std::ostringstream s; s << "loop i" << i << " e" << end << " p" << in[0] << " " << in[1] << " " << in[2]; loggers::trace()(s.str()); }
			do_process_template_switch_x(i, end, *this, in[0], in[1], in[2]); ///\todo variadic
			i = end;
		}
	}

	template<bool have_phase, bool have_freq, bool have_amp>
	void sine::do_process_template(std::size_t i, std::size_t end) {
		///\todo need iterator for each input
		for(; i < end; ++i) {
			if(have_phase) phase_ = std::fmod(phase_chn()[i] + math::pi, 2 * math::pi) - math::pi;
			if(have_freq) freq(freq_chn()[i]);
			if(have_amp) amp_ = amp_chn()[i];
			out_chn()[i](i, amp_ * math::fast_sin<2>(phase_));
			phase_ += step_;
			if(phase_ > math::pi) phase_ -= 2 * math::pi;
		}
	}
#endif

template<
	channel::flags::type phase_flag,
	channel::flags::type  freq_flag,
	channel::flags::type   amp_flag
>
void sine::do_process_template() {
	for(std::size_t phase_evt = 0, freq_evt = 0, amp_evt = 0, out_evt = 0; out_evt < out_chn().size(); ++out_evt) {
		switch(phase_flag) {
			case channel::flags::continuous:
				phase_ = std::fmod(phase_chn()[out_evt].sample() + math::pi, 2 * math::pi) - math::pi;
			break;
			case channel::flags::discrete:
				if(phase_evt < phase_chn().size() && phase_chn()[phase_evt].index() == out_evt)
					phase_ = std::fmod(phase_chn()[phase_evt++].sample() + math::pi, 2 * math::pi) - math::pi;
			break;
			case channel::flags::empty: default: /* nothing */ ;
		}
		
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

		out_chn()[out_evt](out_evt, amp_ * math::fast_sin<2>(phase_));
		phase_ += step_;
		if(phase_ > math::pi) phase_ -= 2 * math::pi;
	}
	out_chn().flag(channel::flags::continuous);
}

}}
