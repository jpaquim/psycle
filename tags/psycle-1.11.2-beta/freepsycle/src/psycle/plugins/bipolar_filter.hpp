// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

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
		void channel_change_notification_from_port(port const &) /*override*/;
		void seconds_per_event_change_notification_from_port(port const &) /*override*/;
		void do_process_first() /*override*/;
		void do_process() /*override*/;
	private:
		real logical_zero_;

	///\name input port
	///\{
		protected:
			ports::inputs::multiple const & in_port() const { return in_port_; }
			ports::inputs::multiple & in_port() { return in_port_; }
		private:
			ports::inputs::multiple in_port_;
	///\}

	///\name output port
	///\{
		protected:
			ports::output const & out_port() const { return out_port_; }
			ports::output & out_port() { return out_port_; }
		private:
			ports::output out_port_;
	///\}
};

}}
#include <psycle/detail/decl.hpp>
