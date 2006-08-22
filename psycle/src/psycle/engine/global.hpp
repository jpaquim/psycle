///\interface psycle::host::Global
#pragma once
#include <psycle/engine/detail/project.hpp>
#include <cstdint>

namespace psycle
{
	namespace host
	{
		class Song;
		class Player;
		//todo:
		//class Configuration;
		class UIConfiguration;
		//todo:
		//class DllFinder;
		class CachedDllFinder;

		namespace dsp
		{
			class Resampler;
		}

		//\todo: move this source to a better place.
		namespace cpu
		{
			//\todo: Microsoft Specific:
			// QueryPerformanceFrequency()
			// QueryPerformanceCounter()
			typedef std::int64_t cycles_type;
			cycles_type inline cycles()
			{
				#if !defined DIVERSALIS__PROCESSOR__X86
					#error "sorry, contains x86-specific asm code"
				#endif
				#if !defined DIVERSALIS__COMPILER__MICROSOFT // or intel?
					#error "sorry, asm code is not written in at&t syntax"
				#endif
				union result_type
				{
					std::uint64_t value;
					struct split_type
					{
						std::uint32_t lo, hi;
					} split;
				} result;
				__asm
				{
					rdtsc; // copies the x86 64-bit cpu cycle time stamp counter to edx and eax
					mov result.split.hi, edx;
					mov result.split.lo, eax;
				}
				return result.value;
			}
		}

		class Global
		{
		public:
			Global();
			~Global() throw();

			static Song * pSong;
			static Player * pPlayer;
			//todo:
			//static Configuration * pConfig;
			static UIConfiguration * pConfig;
			static dsp::Resampler * pResampler;
			//todo:
			//static DllFinder* pDllFinder;
			static CachedDllFinder* pDllFinder;

			static inline Song           & song() { return *pSong; }
			static inline Player         & player(){ return *pPlayer; }
			///\todo:
//			static inline Configuration  & configuration(){ return *pConfig; }
			static inline UIConfiguration  & configuration(){ return *pConfig; }
			static inline dsp::Resampler & resampler(){ return *pResampler; }
			///\todo:
//			static inline DllFinder      & dllfinder(){ return *pFinder; }
			static inline CachedDllFinder      & dllfinder(){ return *pDllFinder; }
/*
			virtual Song           & song();
			virtual Player         & player();
			virtual Configuration  & configuration();
			virtual dsp::Resampler & resampler();
			virtual DllFinder      & dllfinder();
*/
			static cpu::cycles_type inline cpu_frequency(                              ) /*const*/ throw() { return cpu_frequency_; }
//			void             inline cpu_frequency(cpu::cycles_type const & value)           throw() { cpu_frequency_ = value; }
		protected:
			virtual cpu::cycles_type CalculateCPUFreq();
			//\todo: move these source to a better place.
			cpu::cycles_type GetNaiveCPUFreq()
			{
				cpu::cycles_type before(cpu::cycles());
				::Sleep(1000); ///\todo wastes one second to startup :-(
				return cpu::cycles() - before;
			}
		private:
			cpu::cycles_type static        cpu_frequency_;
		};

		//////////////////////////////////////////////////////////////////////////
		//************************************************************************	
		// Implement the following line with your derived class of Global,
		// in the main file of your program.
		// In the case of psycle, this is done in psycle.cpp.
		//Global& psy() { static DerivedGlobal instance; return instance; }
		//************************************************************************
		//////////////////////////////////////////////////////////////////////////
		extern Global& engine();



		//\todo: move this code to a better place.

		/// Safer version of delete that clears the pointer automatically. Don't use it for arrays!
		/// \param pointer pointer to single object to be deleted.
		/// \param new_value the new value pointer will be set to. By default it is null.
		template<typename single_object> inline single_object * zapObject(single_object *& pointer, single_object * const new_value = 0)
		{
			if(pointer) delete pointer;
			return pointer = new_value;
		}

		/// Safer version of delete[] that clears the pointer automatically. Only use it for arrays!
		/// \param pointer pointer to array to be deleted.
		/// \param new_value the new value pointer will be set to. By default it is null.
		template<typename object_array> inline object_array * zapArray(object_array *& pointer, object_array * const new_value = 0)
		{
			if(pointer) delete [] pointer;
			return pointer = new_value;
		}
	}
}
