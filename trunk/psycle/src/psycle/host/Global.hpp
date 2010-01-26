///\file
///\brief interface file for psycle::host::Global.
#pragma once

#include "Version.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
	#include <psycle/core/constants.h>
	#include <psycle/core/commands.h>
	namespace psycle {
		namespace core {
			class Song;
			class Player;
		}
		namespace host {
			namespace notecommands { using namespace psycle::core::notetypes; }
			namespace PatternCmd { using namespace psycle::core::commandtypes; }
		}
	}
	using namespace psycle::core;
#else
	#include "Constants.hpp"
#endif

#if defined DIVERSALIS__OS__MICROSOFT
	#include <windows.h> // for QueryPerformanceCounter
#else
	#include <universalis/os/clocks.hpp>
#endif

#include <universalis/stdlib/cstdint.hpp>
#include <psycle/helpers/math.hpp>

namespace psycle {
	namespace helpers {
		namespace dsp {
			class Resampler;
		}
	}
	namespace host {
		using namespace helpers;
		using namespace helpers::math;

		class Configuration;
		class InputHandler;

		#if !PSYCLE__CONFIGURATION__USE_PSYCORE
			class Song;
			class Player;
			namespace vst {
				class AudioMaster;
			}
		#endif

		//\todo: move this source to a better place.
		namespace cpu {
			typedef std::uint64_t cycles_type;

			cycles_type inline cycles() {
				#if defined DIVERSALIS__OS__MICROSOFT
					LARGE_INTEGER result;
					QueryPerformanceCounter(&result);
					return result.QuadPart;
				#else
					return universalis::os::clocks::realtime::current();
				#endif
			}

			cycles_type inline cycles_per_second() {
				#if defined DIVERSALIS__OS__MICROSOFT
					LARGE_INTEGER result;
					QueryPerformanceFrequency(&result);
					return result.QuadPart;
				#else
					return universalis::os::clocks::realtime::frequency();
				#endif
			}
		}

		class Global {
			public:
				Global();
				virtual ~Global() throw();

				static Player * pPlayer;
				static Configuration * pConfig;
				static dsp::Resampler * pResampler;

				///\todo shouldn't we update this value regularly?
				static cpu::cycles_type _cpuHz;

				#if !defined WINAMP_PLUGIN
					static InputHandler* pInputHandler;
				#endif //!defined WINAMP_PLUGIN

				#if !PSYCLE__CONFIGURATION__USE_PSYCORE
					static vst::AudioMaster* pVstHost;
					static inline vst::AudioMaster	 & vsthost(){ return *pVstHost; }
				#endif

				static Song& song();
				static Player& player() { return *pPlayer; }
				static Configuration & configuration() { return *pConfig; }
				static dsp::Resampler & resampler() { return *pResampler; }
				static cpu::cycles_type cpu_frequency() /*const*/ throw() { return _cpuHz; }
				//void cpu_frequency(cpu::cycles_type const & value) throw() { cpu_frequency_ = value; }
		};
	}
}
