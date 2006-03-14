#ifndef _GLOBAL_H
#define _GLOBAL_H
#pragma unmanaged
/** @file
 *  @brief implementation file
 *  $Date: 2004/10/07 21:27:21 $
 *  $Revision: 1.4 $
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
		static Song* _pSong;
		static Player* pPlayer;
		static Configuration* pConfig;
		static Resampler* pResampler;
	};
	
#else

	class InputHandler;
	class Global
	{

	public:
		Global();
		~Global();

		static unsigned int _cpuHz;
		static Configuration* pConfig;
		static Song* _pSong;
		static Player* pPlayer;
		static Resampler* pResampler;
		static InputHandler* pInputHandler;
		static const SF::string& CurrentDirectory(){return m_CurrentDirectory;}; 
	private:
		static SF::string m_CurrentDirectory;
	};
#endif // _WINAMP_PLUGIN_

#endif