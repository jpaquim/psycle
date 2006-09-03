#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/microsoft_direct_sound_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_MICROSOFT_DIRECT_SOUND
#include "microsoft_direct_sound_out.h"
namespace psycle
{
	namespace host
	{

		AudioDriverInfo MicrosoftDirectSoundOut::info( ) const
		{
			return AudioDriverInfo("microsoft-direct-sound");
		}

		MicrosoftDirectSoundOut * MicrosoftDirectSoundOut::clone( ) const
		{
			return new MicrosoftDirectSoundOut(*this);
		}
	
	} // end of host namespace
} // end of psycle namespace


#endif // !defined XPSYCLE__NO_MICROSOFT_DIRECT_SOUND
