///\file
///\brief interface file for psycle::host::Global.
#pragma once
#include "Version.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/constants.h>
#include <psycle/core/commands.h>
namespace psy {
	namespace core {
		class Song;
		class Player;
	}
}

namespace psycle { namespace host {
namespace notecommands {
	using namespace psy::core::notetypes;
}
namespace PatternCmd {
	using namespace psy::core::commandtypes;
}
}}
using namespace psy::core;
//typedef unsigned char byte;
//typedef unsigned short word;
//typedef unsigned long dword;
#else
#include "Constants.hpp"
#endif

#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h> // for QueryPerformanceCounter
#else
	#include <universalis/operating_system/clocks.hpp>
#endif
//#include <cstdint>

namespace psycle
{
	namespace helpers
	{
		namespace dsp
		{
			class Resampler;
		}
	}
	namespace host
	{
		class Configuration;
		class InputHandler;


#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		class Song;
		class Player;
		namespace vst
		{
			class AudioMaster;
		}
#endif
		//\todo: move this source to a better place.
		namespace cpu
		{
			typedef std::uint64_t cycles_type;

			cycles_type inline cycles()
			{
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					LARGE_INTEGER result;
					QueryPerformanceCounter(&result);
					return result.QuadPart;
				#else
					return universalis::operating_system::clocks::thread::current();
				#endif
			}

			cycles_type inline cycles_per_second()
			{
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					LARGE_INTEGER result;
					QueryPerformanceFrequency(&result);
					return result.QuadPart;
				#else
					return universalis::operating_system::clocks::thread::frequency();
				#endif
			}
		}

		class Global
		{
			private:
			public:
				Global();
				virtual ~Global() throw();

				static Player * pPlayer;
				static Configuration * pConfig;
				static helpers::dsp::Resampler * pResampler;
				///\todo shouldn't we update this value regularly?
				static cpu::cycles_type _cpuHz;
#if !defined WINAMP_PLUGIN
				static InputHandler* pInputHandler;
#endif //!defined WINAMP_PLUGIN

#if !PSYCLE__CONFIGURATION__USE_PSYCORE
				static vst::AudioMaster* pVstHost;

				static inline vst::AudioMaster	 & vsthost(){ return *pVstHost; }
#endif
				static Song           & song();
				static inline Player         & player(){ return *pPlayer; }
				static inline Configuration  & configuration(){ return *pConfig; }
				static inline helpers::dsp::Resampler & resampler(){ return *pResampler; }
				static inline cpu::cycles_type cpu_frequency() /*const*/ throw() { return _cpuHz; }
				//void inline cpu_frequency(cpu::cycles_type const & value) throw() { cpu_frequency_ = value; }
		};
	}
}
