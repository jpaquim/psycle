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

		//class logger
		//{
			inline static void logger(const int & level, const std::string & string) throw()
			{
				try
				{
					if(Global::pLogWindow) Global::pLogWindow->AddEntry(level, string);
				}
				catch(...)
				{
					// oh dear!
					std::cerr << "logger crashed" << std::endl;
				}
			}
		//};
	}
}
