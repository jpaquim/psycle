#if defined PSYCLE__GSTREAMER_AVAILABLE
#include "gstreamerout.h"
namespace psy
{
	namespace core
	{

		AudioDriverInfo GStreamerOut::info( ) const
		{
			return AudioDriverInfo("gstreamer","GStreamer Driver","Currently not implemented",false);
		}

		GStreamerOut * GStreamerOut::clone( ) const
		{
			return new GStreamerOut(*this);
		}
	
	}
}
#endif // defined PSYCLE__GSTREAMER_AVAILABLE
