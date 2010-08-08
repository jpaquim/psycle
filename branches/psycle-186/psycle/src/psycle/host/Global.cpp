///\file
///\brief implementation file for psycle::host::Global.
#include "Global.hpp"

#include "Configuration.hpp"
#if !defined WINAMP_PLUGIN
	#include "InputHandler.hpp"
#endif //!defined WINAMP_PLUGIN

#include "Song.hpp"
#include "Player.hpp"
#include "VstHost24.hpp"
#include <psycle/helpers/dsp.hpp>

namespace psycle
{
	namespace host
	{
		Song * Global::_pSong(0);
		Player * Global::pPlayer(0);
		helpers::dsp::resampler * Global::pResampler(0);
		Configuration * Global::pConfig(0);
#if !defined WINAMP_PLUGIN
		InputHandler * Global::pInputHandler(0);
#endif //!defined WINAMP_PLUGIN

		vst::host *Global::pVstHost(0);

		Global::Global()
		{
			_pSong = new Song;
			pPlayer = new Player;
			pConfig = new Configuration;
			pResampler = new helpers::dsp::cubic_resampler;
			pResampler->quality(helpers::dsp::resampler::quality::linear);
#if !defined WINAMP_PLUGIN
			pInputHandler = new InputHandler;
#endif //!defined WINAMP_PLUGIN
			pVstHost = new vst::host;
		}

		Global::~Global()
		{
			delete _pSong; _pSong = 0;
			delete pPlayer; pPlayer = 0;
			delete pResampler; pResampler = 0;
			delete pConfig; pConfig = 0;
#if !defined WINAMP_PLUGIN
			delete pInputHandler; pInputHandler = 0;
#endif //!defined WINAMP_PLUGIN
			delete pVstHost; pVstHost = 0;
		}
	}
}
