///\interface psycle::host::Global
#pragma once
#include <psycle/host/detail/project.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		class Song;
		class Player;
		class Configuration;
		class InputHandler;
		class DllFinder;
		namespace dsp
		{
			class Resampler;
		}

		namespace cpu
		{
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
			cpu::cycles_type GetNaiveCPUFreq()
			{
				cpu::cycles_type before(cpu::cycles());
				::Sleep(1000); ///\todo wastes one second to startup :-(
				return cpu::cycles() - before;
			}
		}

		class Global
		{
			public:
				Global();
				~Global() throw();

				///\todo use singleton pattern instead of static public vars
				//Global static inline & singleton() { static Global instance; return instance; }

				Song             static inline & song         () throw() { return *_pSong; }
				Player           static inline & player       () throw() { return *pPlayer; }
				Configuration    static inline & configuration() throw() { return *pConfig; }
				dsp::Resampler   static inline & resampler    () throw() { return *pResampler; }
				InputHandler     static inline & input_handler() throw() { return *pInputHandler; }
				// For other implementations, you might wish to use Mapped or simply DllFinder instead of the Cached one.
				DllFinder  static inline & dllfinder()	 throw() { return *pDllFinder; }

			PSYCLE__PRIVATE:// shouldn't be static either
				static Song * _pSong;
				static Player * pPlayer;
				static Configuration * pConfig;
				static dsp::Resampler * pResampler;
				static InputHandler* pInputHandler;
				static DllFinder* pDllFinder;

			public:
				cpu::cycles_type static inline cpu_frequency(                              ) /*const*/ throw() { return cpu_frequency_; }
				void             static inline cpu_frequency(cpu::cycles_type const & value)           throw() { cpu_frequency_ = value; }
			private:
				cpu::cycles_type static        cpu_frequency_;
		};


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
