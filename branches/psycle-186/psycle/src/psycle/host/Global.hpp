///\file
///\brief interface file for psycle::host::Global.
#pragma once
#if defined DIVERSALIS__OPERATING_SYSTEM__WINDOWS
	#include <windows.h> // for QueryPerformanceCounter
#else
	#include <universalis/operating_system/clocks.hpp>
#endif
#include <cstdint>
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
		class Song;
		class Player;
		class Configuration;
		class InputHandler;
		namespace vst
		{
			class host;
		}

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
			///\todo use accessors, dont make everything static/global, otherwise we get uninitialised vars, like _cpuHz
			private:
			public:
				Global();
				virtual ~Global() throw();

				static Song * _pSong;
				static Player * pPlayer;
				static Configuration * pConfig;
				static helpers::dsp::Resampler * pResampler;
				///\todo shouldn't we update this value regularly?
				static cpu::cycles_type _cpuHz;
#if !defined WINAMP_PLUGIN
				static InputHandler* pInputHandler;
#endif //!defined WINAMP_PLUGIN
				static vst::host* pVstHost;

				static inline Song           & song() { return *_pSong; }
				static inline Player         & player(){ return *pPlayer; }
				static inline Configuration  & configuration(){ return *pConfig; }
				static inline helpers::dsp::Resampler & resampler(){ return *pResampler; }
				static inline vst::host		 & vsthost(){ return *pVstHost; }

				static inline cpu::cycles_type cpu_frequency() /*const*/ throw() { return _cpuHz; }
				//void inline cpu_frequency(cpu::cycles_type const & value) throw() { cpu_frequency_ = value; }
		};
	}
}