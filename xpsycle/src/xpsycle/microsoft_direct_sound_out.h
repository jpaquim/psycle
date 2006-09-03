#pragma once
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/microsoft_direct_sound_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_MICROSOFT_DIRECT_SOUND
#include "audiodriver.h"
namespace psycle
	{
		namespace host
		{
			class MicrosoftDirectSoundOut : public AudioDriver
			{
				public:

					virtual AudioDriverInfo info() const;

					virtual MicrosoftDirectSoundOut* clone()  const; // Uses the copy constructor
			};
	}
}
#endif // !defined XPSYCLE__NO_MICROSOFT_DIRECT_SOUND
