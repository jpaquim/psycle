///\file
///\brief interface file for psycle::host::Global.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "version.hpp"
#include "Constants.hpp"
#include "SongStructs.hpp"
#include <Shlwapi.h>

// AVRT is the new "multimedia scheduling stuff"
typedef HANDLE (WINAPI *FAvSetMmThreadCharacteristics)   (LPCTSTR,LPDWORD);
typedef BOOL   (WINAPI *FAvRevertMmThreadCharacteristics)(HANDLE);

namespace psycle
{
	namespace helpers
	{
		namespace dsp
		{
			class resampler;
		}
	}
	namespace host
	{
		class Configuration;
		class Song;
		class Player;
		class MachineLoader;
		namespace vst
		{
			class host;
		}

		extern BOOL Is_Vista_or_Later();

		inline BOOL IsWin64() {
			SYSTEM_INFO si;
			ZeroMemory(&si, sizeof(SYSTEM_INFO));
			GetSystemInfo(&si);
			return (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64);
		}
		inline BOOL IsWow64() {
			return IsOS(OS_WOW6432);
		}


		class Global
		{
			public:
				Global();
				virtual ~Global() throw();

				static FAvSetMmThreadCharacteristics    pAvSetMmThreadCharacteristics;
				static FAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics;

				static inline Song           & song() { return *pSong; }
				static inline Player         & player(){ return *pPlayer; }
				static inline Configuration  & configuration(){ return *pConfig; }
				static inline helpers::dsp::resampler & resampler(){ return *pResampler; }
				static inline vst::host		 & vsthost(){ return *pVstHost; }
				static inline MachineLoader  & machineload() { return *pMacLoad; }

			///\todo use accessors, dont make everything static/global, otherwise we get uninitialised vars, like _cpuHz
			protected:
				static Configuration * pConfig;
				static Song * pSong;
				static helpers::dsp::resampler * pResampler;
				static Player * pPlayer;
				static vst::host * pVstHost;
				static MachineLoader * pMacLoad;
		};
	}
}
