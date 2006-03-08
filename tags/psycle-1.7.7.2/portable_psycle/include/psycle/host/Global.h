#pragma once
#include "LoggingWindow.h"
///\file
///\brief interface file for psycle::host::Global.
namespace psycle
{
	namespace host
	{
		class Song;
		class Player;
		class Configuration;
		class Resampler;

		#if defined _WINAMP_PLUGIN_
			bool FindFileinDir(char *dllname,CString &path);
		#endif

		#if !defined _WINAMP_PLUGIN_
			class InputHandler;
		#endif

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
			#if !defined _WINAMP_PLUGIN_
				static unsigned int _cpuHz;
				static InputHandler* pInputHandler;
			#endif
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

	/// safer version of delete that clears the pointer. Don't use it for arrays!
	/// automatically.
	/// \param ptr pointer to single object to be deleted.
	/// \param newValue the new value ptr will be set to. By default it is null.

	template<class T>
	T* zapObject(T*& ptr, T* newValue=NULL)
	{
		if(ptr)
			delete ptr;
		return ptr=newValue;
	}

	/// safer version of delete[] that clears the pointer. Only use it for arrays!
	/// automatically.
	/// \param ptr pointer to array to be deleted.
	/// \param newValue the new value ptr will be set to. By default it is null.
	template<class T>
	T* zapArray(T *&ptr, T* newValue=NULL)
	{
		if(ptr)
			delete [] ptr;
		return ptr=newValue;
	}

}
