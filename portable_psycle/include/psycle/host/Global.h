#pragma once
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
		class CLoggingWindow;

		#if defined _WINAMP_PLUGIN_
			bool FindFileinDir(char *dllname,CString &path);
		#endif

		#if !defined _WINAMP_PLUGIN_
			class InputHandler;
		#endif

		class Global
		{
		public:
			Global();
			virtual ~Global() throw();
			static Song* _pSong;
			static Player* pPlayer;
			static Configuration* pConfig;
			static Resampler* pResampler;
			static CLoggingWindow* pLogWindow;
			#if !defined _WINAMP_PLUGIN_
				static unsigned int _cpuHz;
				static InputHandler* pInputHandler;
			#endif
		};
	}
}
