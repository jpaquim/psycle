///\interface psycle::host::Global
#pragma once
#include <universalis/compiler.hpp>
#include <universalis/compiler/numeric.hpp>
namespace psycle
{
	/// \todo
	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint8_t" )
	typedef universalis::compiler::numeric< 8>::unsigned_int byte;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint8_t" )
//	typedef ::UCHAR UCHAR;
	/// \todo
	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint16_t")
	typedef universalis::compiler::numeric<16>::unsigned_int word;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint16_t")
//	typedef ::WORD WORD;
	/// \todo
	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
	typedef universalis::compiler::numeric<32>::unsigned_int dword;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
//	typedef ::DWORD DWORD;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
//	typedef ::UINT UINT;
	/// \todo
//	UNIVERSALIS__COMPILER__DEPRECATED("same as std::uint32_t")
//	typedef ::ULONG ULONG;

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
				unsigned __int64 static inline   cpu_frequency() throw() { return _cpuHz; }
				InputHandler     static inline & input_handler() throw() { return *pInputHandler; }

			public:// should be private, not static
				static Song * _pSong;
				static Player * pPlayer;
				static Configuration * pConfig;
				static dsp::Resampler * pResampler;
				UNIVERSALIS__COMPILER__DEPRECATED("check this usage. mustn't be used with 32-bit computations")
				static unsigned __int64 _cpuHz;
				static InputHandler* pInputHandler;
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
