#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/jack_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_JACK
#include "jackout.h"
namespace psycle
{
	namespace host
	{

		AudioDriverInfo JackOut::info( ) const
		{
			return AudioDriverInfo("jack");
		}

	}
}

#endif // !defined XPSYCLE__NO_JACK
