///\file
///\brief \interface psycle::plugins::output
#pragma once
#include <psycle/detail/project.hpp>

#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include "outputs/gstreamer.hpp"
#else
	#include "outputs/direct_sound.hpp"
#endif

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		typedef outputs::
			#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				gstreamer
			#else
				direct_sound
			#endif
			output_base;
			
		/// default output.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base
		{
			public:
				typedef output_base base;
			protected: friend class factory;
				output(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(engine::exception);
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
