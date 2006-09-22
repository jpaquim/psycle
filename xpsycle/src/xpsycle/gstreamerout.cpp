#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/gstreamer_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_GSTREAMER
#include "gstreamerout.h"
namespace psycle
{
	namespace host
	{

		AudioDriverInfo GStreamerOut::info( ) const
		{
			return AudioDriverInfo("gstreamer","GStreamer Driver","Currently not implemented",false);
		}

		GStreamerOut * GStreamerOut::clone( ) const
		{
			return new GStreamerOut(*this);
		}
	
	} // end of host namespace
} // end of psycle namespace


#endif // !defined XPSYCLE__NO_GSTREAMER
