///\file
///\brief implementation file for psycle::host::Global.
#include "Global.hpp"

#include "Configuration.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/host/MainFrm.hpp>

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
		Player * Global::pPlayer(0);
		helpers::dsp::Resampler * Global::pResampler(0);
		Configuration * Global::pConfig(0);
		cpu::cycles_type Global::_cpuHz(cpu::cycles_per_second());
#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		vst::AudioMaster *Global::pVstHost(0);
#endif
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

#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pPlayer = &Player::singleton();
#else
			pPlayer = new Player();
			pVstHost = new vst::AudioMaster();
#endif // PSYCLE__CONFIGURATION__USE_PSYCORE
		}

		Global::~Global()
		{
#if !PSYCLE__CONFIGURATION__USE_PSYCORE
			delete pPlayer; pPlayer = 0;
			delete pResampler; pResampler = 0;
			delete pConfig; pConfig = 0;
			delete pVstHost; pVstHost = 0;
#if !defined WINAMP_PLUGIN
			delete pInputHandler; pInputHandler = 0;
#endif //!defined WINAMP_PLUGIN
#endif // PSYCLE__CONFIGURATION__USE_PSYCORE
		}

		extern CPsycleApp theApp;

		static inline Song&  song() { return ((CMainFrame*)theApp.m_pMainWnd)->projects()->active_project()->song(); }

	}
}


