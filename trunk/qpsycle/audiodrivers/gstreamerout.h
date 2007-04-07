#pragma once
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/gstreamer_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_GSTREAMER
#include "audiodriver.h"
namespace psy
	{
		namespace core
		{
			class GStreamerOut : public AudioDriver
			{
				public:

					virtual AudioDriverInfo info() const;

					virtual GStreamerOut* clone()  const; // Uses the copy constructor
			};
	}
}
#endif // !defined XPSYCLE__NO_GSTREAMER
