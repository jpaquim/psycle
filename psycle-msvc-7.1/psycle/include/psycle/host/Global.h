#ifndef PSYCLE__GLOBAL
#define PSYCLE__GLOBAL
#pragma once
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
public:
	Global();
	virtual ~Global() throw();
	static Song* _pSong;
	static Player* pPlayer;
	static Configuration* pConfig;
	static Resampler* pResampler;
#	if !defined _WINAMP_PLUGIN_
		static unsigned int _cpuHz;
		static InputHandler* pInputHandler;
#	endif
};
#endif
