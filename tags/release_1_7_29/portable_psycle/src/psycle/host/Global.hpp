///\file
///\brief interface file for psycle::host::Global.
#pragma once
#include "LoggingWindow.hpp"
namespace psycle
{
	namespace host
	{
		class Song;
		class Player;
		class Configuration;
		class Resampler;
		class InputHandler;

		class Global
		{
		private:
		public:
			Global();
			virtual ~Global() throw();
			static CLoggingWindow * pLogWindow;
			static Song * _pSong;
			static Player * pPlayer;
			static Configuration * pConfig;
			static Resampler * pResampler;
			static unsigned int _cpuHz;
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
					exception, ///< exceptions thrown from software, via "throw some_exception;".
					crash ///< exceptions thrown from cpu/os. They are translated into c++ exceptions, see operating_system::exceptions::translated.
				};
			}
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
