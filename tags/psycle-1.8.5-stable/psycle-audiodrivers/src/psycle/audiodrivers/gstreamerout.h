#pragma once
#if defined PSYCLE__GSTREAMER_AVAILABLE
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
#endif // defined XPSYCLE__GSTREAMER_AVAILABLE
