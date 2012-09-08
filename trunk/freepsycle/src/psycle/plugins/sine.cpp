// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

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
		
	// TODO no easy way to get the value for old_events_per_second
	//this->step_ *= old_events_per_second * port.seconds_per_event();
	
	real const freq = this->freq();
	freq_to_step_ = 2 * math::pi * port.seconds_per_event();
	this->freq(freq);
}

void sine::do_process() {
	if(!out_port_) return;
	#if PSYCLE__TODO__IDEAL
		// Note: not ideal
		buffer::flags const phase_flag = phase_port_.buffer_flag();
		buffer::flags const freq_flag = freq_port_.buffer_flag();
		buffer::flags const amp_flag = amp_port_.buffer_flag();
		do_process_template_switch(*this, phase_flag, freq_flag, amp_flag);
	#else
		// Note: ideal
		ports::inputs::single * ports[] = {&phase_port_, &freq_port_, &amp_port_};
		do_process_split<sizeof ports / sizeof *ports>(ports);
		out_port_.buffer().flag(buffer::flags::continuous);
	#endif
}

#if PSYCLE__TODO__IDEAL
	// Note: this is not the ideal way to write the algorithm as there still is code duplication between the continuous and discrete cases.
	template<
		buffer::flags phase_flag,
		buffer::flags  freq_flag,
		buffer::flags   amp_flag
	>
	void sine::do_process_template() {
		for(
			std::size_t phase_evt = 0, freq_evt = 0, amp_evt = 0, out_evt = 0, end = out_port_.buffer().events();
			out_evt < end; ++out_evt
		) {
			switch(phase_flag) {
				case buffer::flags::continuous:
					phase_ = std::fmod(phase_port_.buffer().sample(out_evt) + math::pi, 2 * math::pi) - math::pi;
				break;
				case buffer::flags::discrete:
					if(phase_evt < end && phase_port_.buffer().index(phase_evt) == out_evt)
						phase_ = std::fmod(phase_port_.buffer().sample(phase_evt++) + math::pi, 2 * math::pi) - math::pi;
				break;
				case buffer::flags::empty: default: /* nothing */ ;
			}

			switch(freq_flag) {
				case buffer::flags::continuous:
					freq(freq_port_.buffer().sample(out_evt));
				break;
				case buffer::flags::discrete:
					if(freq_evt < end && freq_port_.buffer().index(freq_evt) == out_evt)
						freq(freq_port_.buffer().sample(freq_evt++));
				break;
				case buffer::flags::empty: default: /* nothing */ ;
			}

			switch(amp_flag) {
				case buffer::flags::continuous:
					amp_ = amp_port_.buffer().sample(out_evt);
				break;
				case buffer::flags::discrete:
					if(amp_evt < end && amp_port_.buffer().index(amp_evt) == out_evt)
						amp_ = amp_port_.buffer().sample(amp_evt++);
				break;
				case buffer::flags::empty: default: /* nothing */ ;
			}

			out_port_.buffer().index(out_evt) = out_evt;
			out_port_.buffer().sample(out_evt) = amp_ * math::fast_sin<2>(phase_);
			phase_ += step_;
			if(phase_ > math::pi) phase_ -= 2 * math::pi;
		}
		out_port_.buffer().flag(buffer::flags::continuous);
	}
#else
	template<int Ports>
	void sine::do_process_split(ports::inputs::single * ports[Ports]) {
		std::size_t const size = out_port_.buffer().events();
		std::size_t is[Ports]; for(std::size_t i = 0; i < Ports; ++i) is[i] = 0;
		std::size_t i = 0;
		while(i < size) {
			std::size_t end = size;
			bool in[Ports];
			for(int p = 0; p < Ports; ++p) {
				in[p] = false;
				ports::inputs::single & port = *ports[p];
				if(port) {
					auto buffer = port.buffer();
					switch(buffer.flag()) {
						case buffer::flags::continuous:
							in[p] = true;
						break;
						case buffer::flags::discrete:
							if(is[p] < buffer.events()) {
								if(buffer.index(is[p]) == i) {
									{ std::ostringstream s; s << "pulse i" << i << " p" << p << " is" << is[p]; loggers::trace()(s.str()); }
									end = i;
									++is[p];
									in[p] = true;
								} else if(buffer.index(is[p]) < end) {
									end = buffer.index(is[p]);
									{ std::ostringstream s; s << "end i" << i << " p" << p << " e" << end; loggers::trace()(s.str()); }
								}
							}
					}
				}
			}
			{ std::ostringstream s; s << "loop i" << i << " e" << end << " p" << in[0] << " " << in[1] << " " << in[2]; loggers::trace()(s.str()); }
			do_process_template_switch_xx(i, end, *this, in[0], is[0], in[1], is[1], in[2], is[2]); // TODO variadic
			i = end;
		}
	}

	// TODO This would be the ideal way to write the algorithm as there is no code duplication. It relies on do_process_split calling it on slices.
	template<bool have_phase, bool have_freq, bool have_amp>
	void sine::do_process_template(
		std::size_t out_begin, std::size_t out_end,
		std::size_t phase_begin, std::size_t freq_begin, std::size_t amp_begin
	) {
		// TODO need iterator for each input
		for(std::size_t i = out_begin; i < out_end; ++i) {
			if(have_phase) phase_ = std::fmod(phase_port_.buffer().sample(phase_begin + i) + math::pi, 2 * math::pi) - math::pi;
			if(have_freq) freq(freq_port_.buffer().sample(freq_begin + i));
			if(have_amp) amp_ = amp_port_.buffer().sample(amp_begin + i);
			out_port_.buffer().index(out_begin + i) = out_begin + i;
			out_port_.buffer().sample(out_begin + i) = amp_ * math::fast_sin<2>(phase_);
			phase_ += step_;
			if(phase_ > math::pi) phase_ -= 2 * math::pi;
		}
	}
#endif

}}
