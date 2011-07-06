///\file
///\brief interface file for psycle::host::Global.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "version.hpp"
#include "Constants.hpp"
#include "SongStructs.hpp"
#include <psycle/helpers/math.hpp>
#include <psycle/helpers/value_mapper.hpp>
#include <Shlwapi.h>

// AVRT is the new "multimedia schedulling stuff"
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
		class InputHandler;
		class PsycleConfig;
		class Song;
		class CMidiInput;
		class Player;
		namespace vst
		{
			class host;
		}


		class Global
		{
			///\todo use accessors, dont make everything static/global, otherwise we get uninitialised vars, like _cpuHz
			private:
			public:
				Global();
				virtual ~Global() throw();

				static Configuration * pConfig;
#if !defined WINAMP_PLUGIN
				static InputHandler* pInputHandler;
#endif //!defined WINAMP_PLUGIN
				static Song * _pSong;
				static helpers::dsp::resampler * pResampler;
#if !defined WINAMP_PLUGIN
				static CMidiInput * pMidiInput;
#endif //!defined WINAMP_PLUGIN
				static Player * pPlayer;
				static vst::host* pVstHost;

				static FAvSetMmThreadCharacteristics    pAvSetMmThreadCharacteristics;
				static FAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics;

				static inline Song           & song() { return *_pSong; }
#if !defined WINAMP_PLUGIN
				static inline CMidiInput     & midi(){ return *pMidiInput; }
#endif //!defined WINAMP_PLUGIN
				static inline Player         & player(){ return *pPlayer; }
				static inline Configuration  & configuration(){ return *pConfig; }
#if !defined WINAMP_PLUGIN
				static inline PsycleConfig  & psycleconf(){ return *reinterpret_cast<PsycleConfig*>(pConfig); }
#endif //!defined WINAMP_PLUGIN
				static inline helpers::dsp::resampler & resampler(){ return *pResampler; }
				static inline vst::host		 & vsthost(){ return *pVstHost; }
		};

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
	}
}
