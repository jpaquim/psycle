// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__TEMPLATE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

class PSYCLE__DECL template_plugin : public node {
	public:
		template_plugin(class plugin_library_reference &, name_type const &);
	protected:
		void channel_change_notification_from_port(port const &) /*override*/;
		void seconds_per_event_change_notification_from_port(port const &) /*override*/;
		void do_process() /*override*/;
	private:
		ports::inputs::multiple in_port_;
		ports::inputs::single side_port_;
		ports::output out_port_;
};

}}
#include <psycle/detail/decl.hpp>
