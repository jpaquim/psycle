///\file
///\brief interface file for psycle::host::Global.
#pragma once
#include <cstdint>
namespace psycle
{
	namespace host
	{
		class Song;
		class Player;
		class Configuration;
		class InputHandler;
		namespace dsp
		{
			class Resampler;
		}
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
				union result_type
				{
					struct split_type
					{
						std::uint32_t lo, hi;
					} split;
					std::uint64_t value;
				} result;
				///\todo #include <windows.h>
				QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&result));
				return result.value;
			}
		}

		class Global
		{
		private:
		public:
			Global();
			virtual ~Global() throw();

			static Song * _pSong;
			static Player * pPlayer;
			static Configuration * pConfig;
			static dsp::Resampler * pResampler;
			static cpu::cycles_type _cpuHz;
			static InputHandler* pInputHandler;
			static vst::host* pVstHost;

			static inline Song           & song() { return *_pSong; }
			static inline Player         & player(){ return *pPlayer; }
			static inline Configuration  & configuration(){ return *pConfig; }
			static inline dsp::Resampler & resampler(){ return *pResampler; }
			static inline vst::host		 & vsthost(){ return *pVstHost; }

			static inline cpu::cycles_type cpu_frequency(                              ) /*const*/ throw() { return _cpuHz; }
//			void             inline cpu_frequency(cpu::cycles_type const & value)           throw() { cpu_frequency_ = value; }
		};
	}
}
