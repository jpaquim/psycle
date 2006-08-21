#pragma once
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/jack_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_JACK
#include "audiodriver.h"
namespace psycle
	{
		namespace host
		{
			class JackOut : public AudioDriver
			{
				public:

						virtual AudioDriverInfo info() const;

			};
	}
}
#endif // !defined XPSYCLE__NO_JACK
