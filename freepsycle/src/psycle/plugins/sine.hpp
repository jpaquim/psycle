// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::plugins::sine - oscillator with a sinusoidal wave form
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__SINE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

/// oscillator with a sinusoidal wave form
class UNIVERSALIS__COMPILER__DYNAMIC_LINK sine : public engine::node {

	public:
		void frequency(real const & frequency) { this->step_ = frequency * frequency_to_step_; }
		real frequency() const { return frequency_to_step_ ? step_ / frequency_to_step_ : 0; }

	protected: friend class virtual_factory_access;
		sine(engine::plugin_library_reference &, engine::graph &, std::string const & name);
	protected:
		void seconds_per_event_change_notification_from_port(engine::port const &) /*override*/;
		void do_process() throw(engine::exception) /*override*/;

	private:
		template<bool, bool, bool>
		void do_process_template() throw(engine::exception);
		
		real phase_;
		real step_, frequency_to_step_;
		real amplitude_;

		bool have_phase()     { return *single_input_ports()[0]; }
		bool have_frequency() { return *single_input_ports()[1]; }
		bool have_amplitude() { return *single_input_ports()[2]; }
		bool have_out()       { return *output_ports()[0]; }

		buffer::channel & phase_channel()     { return single_input_ports()[0]->buffer()[0]; }
		buffer::channel & frequency_channel() { return single_input_ports()[1]->buffer()[0]; }
		buffer::channel & amplitude_channel() { return single_input_ports()[2]->buffer()[0]; }
		buffer::channel & out_channel()       { return output_ports()[0]->buffer()[0]; }
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

