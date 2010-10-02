// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::bipolar_filter - a node with one multiple input port named "in" and one output port named "out".
#ifndef PSYCLE__PLUGINS__BIPOLAR_FILTER__INCLUDED
#define PSYCLE__PLUGINS__BIPOLAR_FILTER__INCLUDED
#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__BIPOLAR_FILTER
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// a node with one multiple input port named "in" and one output port named "out".
///
/// Note: This is not a 2-pole frequency filter.
///
/// This class is not a plugin. It exists so that code is shared amongst several plugins by
/// having a class derived from this class and linking against a library containing its implementation.
class PSYCLE__DECL bipolar_filter : public node {
	public:
		bipolar_filter(class plugin_library_reference &, name_type const &, real const & logical_zero);
	protected:
		void channel_change_notification_from_port(port const &) throw(exception) /*override*/;
		void seconds_per_event_change_notification_from_port(port const &) /*override*/;
		void do_process_first() throw(exception) /*override*/;
		void do_process() throw(exception) /*override*/;
	private:
		real logical_zero_;
		ports::inputs::multiple multiple_input_port_;
		ports::output output_port_;
};

}}
#include <psycle/detail/decl.hpp>
#endif
