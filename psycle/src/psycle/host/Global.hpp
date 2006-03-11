///\interface psycle::host::Global
#pragma once
#include "detail/project.hpp"
#include <cstdint>
namespace psycle
{
	/// \todo
	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint8_t" )
	typedef std::uint8_t byte;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint8_t" )
//	typedef ::UCHAR UCHAR;
	/// \todo
	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint16_t")
	typedef std::uint16_t word;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint16_t")
//	typedef ::WORD WORD;
	/// \todo
	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
	typedef std::uint32_t dword;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
//	typedef ::DWORD DWORD;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
//	typedef ::UINT UINT;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
//	typedef ::ULONG ULONG;
	/// \todo
	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint64_t")
	typedef ::ULONGLONG ULONGLONG;

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

			public:// should be private, not static
				static Song * _pSong;
				static Player * pPlayer;
				static Configuration * pConfig;
				static dsp::Resampler * pResampler;
				static InputHandler* pInputHandler;

			public:
				cpu::cycles_type static inline cpu_frequency(                              ) /*const*/ throw() { return cpu_frequency_; }
				void             static inline cpu_frequency(cpu::cycles_type const & value)           throw() { cpu_frequency_ = value; }
			private:
				cpu::cycles_type static        cpu_frequency_;
		};

		namespace loggers
		{
			/// very low level, debug, flooding output.
			///\see levels::trace
			void trace(const std::string & string) throw();
			/// normal, informative output.
			///\see levels::info
			void info(const std::string & string) throw();
			/// warnings.
			///\see levels::warning
			void warning(const std::string & string) throw();
			/// exceptions thrown from software, via "throw some_exception;".
			///\see levels::exception
			void exception(const std::string & string) throw();
			/// exceptions thrown from cpu/os.
			/// They are translated into c++ exceptions, see operating_system::exceptions::translated.
			///\see levels::crash
			void crash(const std::string & string) throw();
			/// levels of importance.
			namespace levels
			{
				enum level
				{
					trace, ///< very low level, debug, flooding output.
					info, ///< normal, informative output.
					warning, ///< warnings.
					exception, ///< exceptions thrown from software, via "throw some_exception;".
					crash ///< exceptions thrown from cpu/os. They are translated into c++ exceptions, see operating_system::exceptions::translated.
				};
			}
		}

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
