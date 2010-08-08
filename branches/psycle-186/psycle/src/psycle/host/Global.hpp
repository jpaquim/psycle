///\file
///\brief interface file for psycle::host::Global.
#pragma once

#include "namespaces.hpp"
#include "Version.hpp"

#include "Constants.hpp"
#include "SongStructs.hpp"

#include <psycle/helpers/math.hpp>
#include <psycle/helpers/value_mapper.hpp>

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
		class Song;
		class Player;
		class Configuration;
		class InputHandler;
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

				static Song * _pSong;
				static Player * pPlayer;
				static Configuration * pConfig;
				static helpers::dsp::resampler * pResampler;
#if !defined WINAMP_PLUGIN
				static InputHandler* pInputHandler;
#endif //!defined WINAMP_PLUGIN
				static vst::host* pVstHost;

				static inline Song           & song() { return *_pSong; }
				static inline Player         & player(){ return *pPlayer; }
				static inline Configuration  & configuration(){ return *pConfig; }
				static inline helpers::dsp::resampler & resampler(){ return *pResampler; }
				static inline vst::host		 & vsthost(){ return *pVstHost; }
		};
	}
}
