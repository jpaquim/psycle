#ifndef _GLOBAL_H
#define _GLOBAL_H
/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#if !defined(_PORTING)
class Song;
class Player;
class Resampler;
#endif
class Configuration;

#if defined(_WINAMP_PLUGIN_)

	bool FindFileinDir(char *dllname,CString &path);
	
	class Global
	{
	public:
		Global();
		~Global();
#if !defined(_PORTING)
		static Song* _pSong;
		static Player* pPlayer;
		static Configuration* pConfig;
#endif
		static Resampler* pResampler;
	};
	
#else

#if !defined(_PORTING)
	class InputHandler;
#endif

	class Global
	{
	public:
		Global();
		~Global();

		static unsigned int _cpuHz;
		static Configuration* pConfig;
#if !defined(_PORTING)
		static Song* _pSong;
		static Player* pPlayer;
		static Resampler* pResampler;
		static InputHandler* pInputHandler;
#endif
	};
#endif // _WINAMP_PLUGIN_

#endif