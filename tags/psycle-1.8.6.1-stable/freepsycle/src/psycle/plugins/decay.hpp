// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::decay - a decaying pulse
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__DECAY
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

/// a decaying pulse
class UNIVERSALIS__COMPILER__DYNAMIC_LINK decay : public engine::node {

	public:
		void decay_per_second(real const & decay_per_second) {
			this->decay_ = std::pow(decay_per_second, seconds_per_event_);
		}
		real decay_per_second() const {
			return std::pow(decay_, events_per_second_);
		}

	protected: friend class virtual_factory_access;
		decay(engine::plugin_library_reference &, engine::graph &, std::string const & name);

	protected:
		void seconds_per_event_change_notification_from_port(engine::port const &) /*override*/;
		void do_process() throw(engine::exception) /*override*/;

	private:
		template<channel::flags::type, channel::flags::type>
		void do_process_template() throw(engine::exception);
		
		real current_;
		real decay_, events_per_second_, seconds_per_event_;

		bool have_pulse() { return *single_input_ports()[0]; }
		bool have_decay() { return *single_input_ports()[1]; }
		bool have_out()   { return *output_ports()[0]; }

		channel & pulse_channel() { return single_input_ports()[0]->buffer()[0]; }
		channel & decay_channel() { return single_input_ports()[1]->buffer()[0]; }
		channel & out_channel()   { return output_ports()[0]->buffer()[0]; }
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
