///\file
///\brief interface file for psycle::host::Global.
#pragma once

#include "namespaces.hpp"
#include "Version.hpp"

#include <psycle/core/constants.h>
#include <psycle/core/commands.h>
#include <psycle/helpers/math.hpp>

namespace psycle {
namespace core {
	class Song;
	class Player;
}
namespace helpers { namespace dsp {
	class Resampler;
}}
namespace host {
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

		static Song& song();
		static Player& player() { return *pPlayer; }
		static Configuration & configuration() { return *pConfig; }
		static dsp::Resampler & resampler() { return *pResampler; }
};

}}
