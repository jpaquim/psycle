// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::additioner - + operation
#pragma once
#include "bipolar_filter.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__ADDITIONER
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// + operation
class PSYCLE__DECL additioner : public bipolar_filter {
	public:
		additioner(engine::plugin_library_reference & plugin_library_reference, name_type const & name)
			: bipolar_filter(plugin_library_reference, name, 0) {}
	protected:
		void do_process() /*override*/;
};

}}
#include <psycle/detail/decl.hpp>
