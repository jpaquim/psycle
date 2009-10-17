// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::template_plugin
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__TEMPLATE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

class UNIVERSALIS__COMPILER__DYNAMIC_LINK template_plugin : public engine::node {
	protected: friend class virtual_factory_access;
		template_plugin(engine::plugin_library_reference &, engine::graph &, std::string const & name);
	protected:
		void do_process() throw(engine::exception) /*override*/;
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

