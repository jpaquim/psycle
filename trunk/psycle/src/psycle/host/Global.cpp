///\file
///\brief implementation file for psycle::host::Global.
#include "Global.hpp"

#include "Configuration.hpp"
#ifdef use_psycore
#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/core/vsthost.h>
using namespace psy::core;
#else
#include "Song.hpp"
#include "Player.hpp"
#include "VstHost24.hpp"
#endif
#if !defined WINAMP_PLUGIN
	#include "InputHandler.hpp"
#endif //!defined WINAMP_PLUGIN

#include <psycle/helpers/dsp.hpp>
namespace psycle
{
	namespace host
	{
		Song * Global::_pSong(0);
		Player * Global::pPlayer(0);
		helpers::dsp::Resampler * Global::pResampler(0);
		Configuration * Global::pConfig(0);
		cpu::cycles_type Global::_cpuHz(cpu::cycles_per_second());
		vst::host *Global::pVstHost(0);
#if !defined WINAMP_PLUGIN
		InputHandler * Global::pInputHandler(0);
#endif //!defined WINAMP_PLUGIN

		Global::Global()
		{
			_cpuHz = cpu::cycles_per_second();
			pConfig = new Configuration();
			pResampler = new helpers::dsp::Cubic();
			pResampler->SetQuality(helpers::dsp::R_LINEAR);
#if !defined WINAMP_PLUGIN
			pInputHandler = new InputHandler();
#endif //!defined WINAMP_PLUGIN

#ifdef use_psycore
			pPlayer = &Player::singleton();
			pVstHost = &vst::host::getInstance(pPlayer);
#else
			_pSong = new Song();
			pPlayer = new Player();
			pVstHost = new vst::host();
#endif // use_psycore
		}

		Global::~Global()
		{
#ifdef use_psycore
#else
			delete _pSong; _pSong = 0;
			delete pPlayer; pPlayer = 0;
			delete pResampler; pResampler = 0;
			delete pConfig; pConfig = 0;
			delete pVstHost; pVstHost = 0;
#if !defined WINAMP_PLUGIN
			delete pInputHandler; pInputHandler = 0;
#endif //!defined WINAMP_PLUGIN
#endif // use_psycore
		}
	}
}


