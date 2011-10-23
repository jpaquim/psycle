// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__DECAY
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// a decaying pulse
class PSYCLE__DECL decay : public node {
	public:
		decay(class plugin_library_reference &, name_type const &);

	public:
		void decay_per_second(real const & decay_per_second) {
			this->decay_ = std::pow(decay_per_second, seconds_per_event_);
		}
		real decay_per_second() const {
			return std::pow(decay_, events_per_second_);
		}

	protected:
		void seconds_per_event_change_notification_from_port(port const &) /*override*/;
		void do_process() /*override*/;

	private:
		template<
			channel::flags::type pulse_flag,
			channel::flags::type decay_flag
		>
		void do_process_template(); friend class node;

		real current_;
		real decay_, events_per_second_, seconds_per_event_;

		bool have_pulse() { return *single_input_ports()[0]; }
		bool have_decay() { return *single_input_ports()[1]; }
		bool have_out()   { return *output_ports()[0]; }

		channel & pulse_channel() { return single_input_ports()[0]->buffer()[0]; }
		channel & decay_channel() { return single_input_ports()[1]->buffer()[0]; }
		channel & out_channel()   { return output_ports()[0]->buffer()[0]; }

		ports::output out_port_;
		ports::inputs::single pulse_port_, decay_port_;
};

}}
#include <psycle/detail/decl.hpp>
