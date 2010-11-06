// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::resource - an abstract plugin for resources.
#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__RESOURCE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// resource plugin
class PSYCLE__DECL resource : public engine::node {
	protected: friend class virtual_factory_access;
		resource(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(std::exception);
	protected:
		void do_open()  throw(std::exception) /*override*/;
		void do_start() throw(std::exception) /*override*/;
		void do_stop()  throw(std::exception) /*override*/;
		void do_close() throw(std::exception) /*override*/;
};

}}
#include <psycle/detail/decl.hpp>