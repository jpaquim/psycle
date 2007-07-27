///\file
///\brief implementation file for psycle::host::Global.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/engine/global.hpp>
#include <psycle/helpers/dsp.hpp>
#include <psycle/engine/song.hpp>
#include <psycle/engine/player.hpp>
//todo:
//#include <psycle/engine/dllfinder.hpp>
//#include <psycle/engine/configuration.hpp>
#include <psycle/host/cacheddllfinder.hpp>
#include <psycle/host/uiconfiguration.hpp>

namespace psycle
{
	namespace host
	{
			Song *            Global::pSong(0);
			Player *          Global::pPlayer(0);
			dsp::Resampler *  Global::pResampler(0);
			UIConfiguration *   Global::pConfig(0);
			cpu::cycles_type  Global::cpu_frequency_(0 /*GetCPUFreq()*/);
			CachedDllFinder * Global::pDllFinder(0);

			Global::Global()
			{
				pSong = new Song;
				pPlayer = new Player(*pSong); // [bohan] afaik song is never deleted/recreated from the gui, so we don't even have to care about updating the player's reference.
				//todo:
				//pConfig = new Configuration;
				pConfig = new UIConfiguration;
				pResampler = new dsp::Cubic;
				pResampler->SetQuality(dsp::R_LINEAR);
				//todo
				//pDllFinder = new DllFinder;
				pDllFinder = new CachedDllFinder;
				cpu_frequency_ = CalculateCPUFreq();
			}

			Global::~Global()
			{
				delete pSong;
				delete pPlayer;
				delete pResampler;
				delete pConfig;
				delete pDllFinder;
		}

		cpu::cycles_type Global::CalculateCPUFreq()
		{
			return GetNaiveCPUFreq();
		}
	}
}
