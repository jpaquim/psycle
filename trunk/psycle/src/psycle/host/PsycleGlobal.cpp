///\file
///\brief implementation file for psycle::host::Global.
#include <psycle/host/detail/project.private.hpp>
#include "PsycleGlobal.hpp"

#include "PsycleConfig.hpp"
#include "InputHandler.hpp"
#include "DPI.hpp"

#include "Song.hpp"
#include "MidiInput.hpp"
#include "Player.hpp"
#include "VstHost24.hpp"
#include "plugincatcher.hpp"
#include <psycle/helpers/dsp.hpp>
namespace psycle
{
	namespace host
	{
		CMidiInput * PsycleGlobal::pMidiInput(0);
		InputHandler * PsycleGlobal::pInputHandler(0);
		CDPI PsycleGlobal::dpi;

		PsycleGlobal::PsycleGlobal() : Global()
		{
			pConfig = new PsycleConfig();
			pSong = new Song();
			pResampler = new helpers::dsp::cubic_resampler();
			pResampler->quality(helpers::dsp::resampler::quality::linear);
			pMidiInput = new CMidiInput();
			pVstHost = new vst::host();
			pPlayer = new Player();
			pInputHandler = new InputHandler();
			pMacLoad = new PluginCatcher();
		}

		PsycleGlobal::~PsycleGlobal()
		{
			delete pSong; pSong = 0;
			//player has to be deleted before vsthost
			delete pPlayer; pPlayer = 0;
			//vst host has to be deleted after song.
			delete pVstHost; pVstHost = 0;
			delete pMacLoad; pMacLoad = 0;
			delete pMidiInput; pMidiInput = 0;
			delete pResampler; pResampler = 0;
			delete pInputHandler; pInputHandler = 0;
			delete pConfig; pConfig = 0;
		}
	}
}

