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

#define _GetProc(fun, type, name)  {                                                        \
                                        fun = (type) GetProcAddress(hDInputDLL,name);       \
                                        if (fun == NULL) { return false; }                  \
                                    }                                                       \


namespace portaudio
{
	static HMODULE hDInputDLL = 0;

	//Dynamic load and unload of avrt.dll, so the executable can run on Windows 2K and XP.
	static bool SetupAVRT()
	{
		using namespace psycle::host;
		hDInputDLL = LoadLibraryA("avrt.dll");
		if (hDInputDLL == NULL)
			return false;

		_GetProc(Global::pAvSetMmThreadCharacteristics,   FAvSetMmThreadCharacteristics,   "AvSetMmThreadCharacteristicsA");
		_GetProc(Global::pAvRevertMmThreadCharacteristics,FAvRevertMmThreadCharacteristics,"AvRevertMmThreadCharacteristics");
	    
		return Global::pAvSetMmThreadCharacteristics && 
			Global::pAvRevertMmThreadCharacteristics;
	}

	// ------------------------------------------------------------------------------------------
	static void CloseAVRT()
	{
		if (hDInputDLL != NULL)
			FreeLibrary(hDInputDLL);
		hDInputDLL = NULL;
	}
}
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
		FAvSetMmThreadCharacteristics    Global::pAvSetMmThreadCharacteristics(NULL);
		FAvRevertMmThreadCharacteristics Global::pAvRevertMmThreadCharacteristics(NULL);


		Global::Global()
		{
			if (Is_Vista_or_Later()) {
				portaudio::SetupAVRT();
			}
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
			portaudio::CloseAVRT();
		}

		BOOL Is_Vista_or_Later() 
		{
		   OSVERSIONINFOEX osvi;
		   DWORDLONG dwlConditionMask = 0;
		   int op=VER_GREATER_EQUAL;

		   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		   osvi.dwMajorVersion = 0;
		   osvi.dwMinorVersion = 0;

		   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
		   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );

		   // Perform the test.

		   return VerifyVersionInfo(
			  &osvi, VER_MAJORVERSION | VER_MINORVERSION,
			  dwlConditionMask);
		}
	}
}

