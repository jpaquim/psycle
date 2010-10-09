// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::sine - oscillator with a sinusoidal wave form
#ifndef PSYCLE__PLUGINS__SINE
#define PSYCLE__PLUGINS__SINE
#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__SINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// oscillator with a sinusoidal wave form
class PSYCLE__DECL sine : public node {

	public:
		sine(class plugin_library_reference &, name_type const &);

		void freq(real const & freq) { step_ = std::abs(freq) * freq_to_step_; if(freq < 0) phase_ = math::pi - phase_; }
		real freq() const { return freq_to_step_ ? step_ / freq_to_step_ : 0; }

		void amp(real const & amp) { amp_ = amp; }
		real amp() const { return amp_; }

	protected:
		void seconds_per_event_change_notification_from_port(port const &) /*override*/;
		void do_process() throw(exception) /*override*/;

	private:
		template<
			channel::flags::type phase_flag,
			channel::flags::type  freq_flag,
			channel::flags::type   amp_flag
		>
		void do_process_template();
	
		real phase_, step_, freq_to_step_, amp_;
		ports::output out_port_;
		ports::inputs::single phase_port_, freq_port_, amp_port_;

		channel & phase_chn() { return phase_port_.buffer()[0]; }
		channel &  freq_chn() { return  freq_port_.buffer()[0]; }
		channel &   amp_chn() { return   amp_port_.buffer()[0]; }
		channel &   out_chn() { return   out_port_.buffer()[0]; }
};

}}
#include <psycle/detail/decl.hpp>
#endif
