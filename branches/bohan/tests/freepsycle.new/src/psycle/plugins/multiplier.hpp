// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::multiplier - * operation
#ifndef PSYCLE__PLUGINS__MULTIPLIER__INCLUDED
#define PSYCLE__PLUGINS__MULTIPLIER__INCLUDED
#pragma once
#include "bipolar_filter.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__MULTIPLIER
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// * operation
class PSYCLE__DECL multiplier : public bipolar_filter {
	public:
		multiplier(class plugin_library_reference & plugin_library_reference, name_type const & name)
			: bipolar_filter(plugin_library_reference, name, 1) {}
	protected: 
		void do_process() /*override*/;
};

}}
#include <psycle/detail/decl.hpp>
#endif
