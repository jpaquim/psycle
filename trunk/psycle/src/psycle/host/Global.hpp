///\file
///\brief interface file for psycle::host::Global.
#pragma once

#include "Version.hpp"

#include <psycle/core/constants.h>
#include <psycle/core/commands.h>

#include <psycle/helpers/math.hpp>
#include <universalis/os/clocks.hpp>
#include <universalis/stdlib/cstdint.hpp>

namespace psycle {
	namespace core {
		class Song;
		class Player;
	}

	namespace helpers { namespace dsp {
		class Resampler;
	}}

	namespace host {
		namespace notecommands = core::notetypes;
		namespace PatternCmd = core::commandtypes;

		using namespace core;
		using namespace helpers;
		using namespace helpers::math;

		class Configuration;
		class InputHandler;

		class Global {
			public:
				Global();
				virtual ~Global() throw();

				static Player * pPlayer;
				static Configuration * pConfig;
				static dsp::Resampler * pResampler;

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
		};
	}
}
