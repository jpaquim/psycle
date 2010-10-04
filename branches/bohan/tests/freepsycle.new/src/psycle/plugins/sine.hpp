// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::sine - oscillator with a sinusoidal wave form
#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__SINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// oscillator with a sinusoidal wave form
class PSYCLE__DECL sine : public node {

	public:
		sine(class plugin_library_reference &, name_type const &);

		void frequency(real const & frequency) { step_ = std::abs(frequency) * frequency_to_step_; if(frequency < 0) phase_ = engine::math::pi - phase_; }
		real frequency() const { return frequency_to_step_ ? step_ / frequency_to_step_ : 0; }

		void amplitude(real const & amplitude) { amplitude_ = amplitude; }
		real amplitude() const { return amplitude_; }

	protected:
		void seconds_per_event_change_notification_from_port(port const &) /*override*/;
		void do_process() throw(exception) /*override*/;

	private:
		template<channel::flags::type, channel::flags::type, channel::flags::type>
		void do_process_template() throw(exception);
		
		real phase_;
		real step_, frequency_to_step_;
		real amplitude_;

		ports::inputs::single & phase_port()     { return *single_input_ports()[0]; }
		ports::inputs::single & frequency_port() { return *single_input_ports()[1]; }
		ports::inputs::single & amplitude_port() { return *single_input_ports()[2]; }
		ports::output         & out_port()       { return *output_ports()[0]; }

		channel & phase_channel()     { return phase_port().buffer()[0]; }
		channel & frequency_channel() { return frequency_port().buffer()[0]; }
		channel & amplitude_channel() { return amplitude_port().buffer()[0]; }
		channel & out_channel()       { return out_port().buffer()[0]; }

		ports::output out_port_;
		ports::inputs::single phase_port_, freq_port_, amp_port_;
};

}}
#include <psycle/detail/decl.hpp>
