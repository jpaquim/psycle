// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::additioner - + operation
#pragma once
#include "bipolar_filter.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__ADDITIONER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

/// + operation
class UNIVERSALIS__COMPILER__DYNAMIC_LINK additioner : public bipolar_filter {
	protected: friend class virtual_factory_access;
		additioner(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
			: bipolar_filter(plugin_library_reference, graph, name, 0) {}
	protected:
		void do_process() throw(engine::exception) /*override*/;
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

