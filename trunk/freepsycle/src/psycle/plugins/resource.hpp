// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::plugins::resource - an abstract plugin for resources.
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__RESOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

/// resource plugin
class UNIVERSALIS__COMPILER__DYNAMIC_LINK resource : public engine::node {
	protected: friend class virtual_factory_access;
		resource(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(std::exception);
	protected:
		void do_open()  throw(std::exception) /*override*/;
		void do_start() throw(std::exception) /*override*/;
		void do_stop()  throw(std::exception) /*override*/;
		void do_close() throw(std::exception) /*override*/;
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

