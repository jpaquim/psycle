///\file
///\brief implementation file for psycle::host::Global.
#include <psycle/project.private.hpp>
#include "global.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "Configuration.hpp"
#include "InputHandler.hpp"
#include "vsthost24.hpp"
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
		InputHandler * Global::pInputHandler(0);
		vst::host *Global::pVstHost(0);

		Global::Global()
		{
			_cpuHz = cpu::cycles_per_second();
			_pSong = new Song;
			pPlayer = new Player;
			pConfig = new Configuration;
			pResampler = new helpers::dsp::Cubic;
			pResampler->SetQuality(helpers::dsp::R_LINEAR);
			pInputHandler = new InputHandler;
			pVstHost = new vst::host;
		}

		Global::~Global()
		{
			delete _pSong; _pSong = 0;
			delete pPlayer; pPlayer = 0;
			delete pResampler; pResampler = 0;
			delete pConfig; pConfig = 0;
			delete pInputHandler; pInputHandler = 0;
			delete pVstHost; pVstHost = 0;
		}
	}
}
