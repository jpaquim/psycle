#ifndef _GLOBAL_H
#define _GLOBAL_H

#if defined(_WINAMP_PLUGIN_)

	#include <windows.h>
	#include "WinampPlugin\resource.h"

	class Song;
	class Player;
	class Configuration;

	bool FindFileinDir(char *dllname,char* path);

	class Global
	{
	public:
		Global();
		~Global();

		static Song* _pSong;
		static Player* pPlayer;
		static int _lbc;
		static Configuration* pConfig;
	};

	#define CPoint POINT
	#define TRACE //


#else

	class Song;
	class Player;
	class Configuration;
	class Resampler;
	class InputHandler;

	class Global
	{
	public:
		Global();
		~Global();

		static unsigned int _cpuHz;
		static Song* _pSong;
		static Player* pPlayer;
		static int _lbc;
		static Configuration* pConfig;
		static Resampler* pResampler;
		static InputHandler* pInputHandler;
	};
#endif // _WINAMP_PLUGIN_

#endif