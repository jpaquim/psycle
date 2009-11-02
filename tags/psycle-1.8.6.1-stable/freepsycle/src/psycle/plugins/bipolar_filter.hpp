// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::bipolar_filter - a node with one multiple input port named "in" and one output port named "out".
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__BIPOLAR_FILTER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

/// a node with one multiple input port named "in" and one output port named "out".
///
/// Note: This is not a 2-pole frequency filter.
///
/// This class is not a plugin. It exists so that code is shared amongst several plugins by
/// having a class derived from this class and linking against a library containing its implementation.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK bipolar_filter : public engine::node {
	protected: friend class virtual_factory_access;
		bipolar_filter(engine::plugin_library_reference &, engine::graph &, std::string const & name, real const & logical_zero);
	protected:
		void channel_change_notification_from_port(engine::port const &) throw(engine::exception) /*override*/;
		void seconds_per_event_change_notification_from_port(engine::port const &) /*override*/;
		void do_process_first() throw(engine::exception) /*override*/;
		void do_process() throw(engine::exception) /*override*/;
	private:
		real logical_zero_;
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
