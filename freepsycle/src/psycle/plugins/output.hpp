// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\interface psycle::plugins::output
#pragma once
#include <psycle/detail/project.hpp>

#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include "outputs/gstreamer.hpp"
#else
	#include "outputs/direct_sound.hpp"
#endif

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

	typedef outputs::
		#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			gstreamer
		#else
			direct_sound
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

