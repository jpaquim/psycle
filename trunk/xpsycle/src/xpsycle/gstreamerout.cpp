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
			return AudioDriverInfo("gstreamer");
		}

	}
}


#endif // !defined XPSYCLE__NO_GSTREAMER
