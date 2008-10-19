// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; erodix ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::output
#pragma once
#include <psycle/detail/project.hpp>

#if defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__GSTREAMER
	#include "outputs/gstreamer.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__JACK
	#include "outputs/jack.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__ALSA
	#include "outputs/alsa.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__DIRECT_SOUND
	#include "outputs/direct_sound.hpp"
#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__DUMMY
	#include "outputs/dummy.hpp"
#else
	#error no default output
#endif

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

typedef outputs::
	#if defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__GSTREAMER
		gstreamer
	#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__JACK
		jack
	#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__ALSA
		alsa
	#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__DIRECT_SOUND
		direct_sound
	#elif defined PSYCLE__PLUGINS__OUTPUTS__DEFAULT__DUMMY
		dummy
	#else
		#error no default output
	#endif
	output_base;
	
/// default output.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base {
	public:
		typedef output_base base;
	protected: friend class virtual_factory_access;
		output(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(engine::exception);
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
