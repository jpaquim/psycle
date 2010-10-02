// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::resource - an abstract plugin for resources.
#ifndef PSYCLE__PLUGINS__RESOURCE__INCLUDED
#define PSYCLE__PLUGINS__RESOURCE__INCLUDED
#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__RESOURCE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// resource plugin
class PSYCLE__DECL resource : public node {
	public:
		resource(class plugin_library_reference &, name_type const &) throw(std::exception);
	protected:
		void do_open()  throw(std::exception) /*override*/;
		void do_start() throw(std::exception) /*override*/;
		void do_stop()  throw(std::exception) /*override*/;
		void do_close() throw(std::exception) /*override*/;
};

}}
#include <psycle/detail/decl.hpp>
#endif
