// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::multiplier - * operation
#pragma once
#include "bipolar_filter.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__MULTIPLIER
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// * operation
class PSYCLE__DECL multiplier : public bipolar_filter {
	protected: friend class virtual_factory_access;
		multiplier(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
			: bipolar_filter(plugin_library_reference, graph, name, 1) {}
	protected: 
		void do_process() throw(engine::exception) /*override*/;
};

}}
#include <psycle/detail/decl.hpp>
