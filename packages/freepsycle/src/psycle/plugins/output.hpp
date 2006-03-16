///\file
///\brief \interface psycle::plugins::output
#pragma once
#include <psycle/detail/project.hpp>

#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include "outputs/gstreamer.hpp"
#else
	#include "outputs/direct_sound.hpp"
#endif

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
		class output : public output_base
		{
			public:
				typedef output_base base;
				output(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
		};
	}
}
