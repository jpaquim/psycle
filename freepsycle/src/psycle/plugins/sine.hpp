// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::sine - oscillator with a sinusoidal wave form
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__SINE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

/// oscillator with a sinusoidal wave form
class UNIVERSALIS__COMPILER__DYNAMIC_LINK sine : public engine::node {

	public:
		void frequency(real const & frequency) { step_ = std::abs(frequency) * frequency_to_step_; if(frequency < 0) phase_ = engine::math::pi - phase_; }
		real frequency() const { return frequency_to_step_ ? step_ / frequency_to_step_ : 0; }

	protected: friend class virtual_factory_access;
		sine(engine::plugin_library_reference &, engine::graph &, std::string const & name);
	protected:
		void seconds_per_event_change_notification_from_port(engine::port const &) /*override*/;
		void do_process() throw(engine::exception) /*override*/;

	private:
		template<channel::flags::type, channel::flags::type, channel::flags::type>
		void do_process_template() throw(engine::exception);
		
		real phase_;
		real step_, frequency_to_step_;
		real amplitude_;

		engine::ports::inputs::single & phase_port()     { return *single_input_ports()[0]; }
		engine::ports::inputs::single & frequency_port() { return *single_input_ports()[1]; }
		engine::ports::inputs::single & amplitude_port() { return *single_input_ports()[2]; }
		engine::ports::output         & out_port()       { return *output_ports()[0]; }

		channel & phase_channel()     { return phase_port().buffer()[0]; }
		channel & frequency_channel() { return frequency_port().buffer()[0]; }
		channel & amplitude_channel() { return amplitude_port().buffer()[0]; }
		channel & out_channel()       { return out_port().buffer()[0]; }
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
