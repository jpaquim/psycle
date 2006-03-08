#ifndef _GLOBAL_H
#define _GLOBAL_H

#if defined(_WINAMP_PLUGIN_)
	#define WIN32_MEAN_AND_LEAN

	#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

	#include <afxwin.h>
	#include "WinampPlugin\resource.h"
	
	class Song;
	class Player;
	class Configuration;

	#define MAX_CONNECTIONS		12

	bool FindFileinDir(char *dllname,CString &path);

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

#else

	class Song;
	class Player;
	class Configuration;
	class Resampler;
	class InputHandler;

	#define MAX_CONNECTIONS		12

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