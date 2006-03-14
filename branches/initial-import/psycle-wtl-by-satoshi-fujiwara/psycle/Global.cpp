/** @file
 *  @brief implementation file
 *  $Date: 2004/11/09 12:40:35 $
 *  $Revision: 1.5 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
	#include "Song.h"
	#include "Instrument.h"
	#include "Player.h"
	#include "Dsp.h"
	#include "Configuration.h"

	Song* Global::_pSong = NULL;
	Player* Global::pPlayer = NULL;
	Resampler* Global::pResampler = NULL;
	Configuration* Global::pConfig = NULL;
	SF::string Global::m_CurrentDirectory = "";

	Global::Global()
	{
		pResampler = new Cubic;
		pResampler->SetQuality(RESAMPLE_LINEAR);
		_pSong = new Song;
		pPlayer = new Player;
		pConfig = new Configuration;
		TCHAR _dir[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH,_dir);
		m_CurrentDirectory = _dir;
	}

	Global::~Global()
	{
		delete _pSong;
		delete pPlayer;
		delete pResampler;
		delete pConfig;
	}

	bool FindFileinDir(char *dllname,CString &path)
	{
		CFindFile finder;
		int loop = finder.FindFile(path + _T("\\*"));	// check for subfolders.
		while (loop) 
		{						
			loop = finder.FindNextFile();
			if (finder.IsDirectory() && !finder.IsDots())
			{
				CString filepath = finder.GetFilePath();
				if (FindFileinDir(dllname,filepath))
				{
					path=filepath;
					return true;
				}
			}
		}
		finder.Close();
		
		if (finder.FindFile(path + _T("\\") + dllname)) //not found in subdirectories, lets see if it's here
		{
			finder.Close();
			path= (path + _T("\\")) + dllname;
			return true;
		}
		finder.Close();
		return false;
	}

#else
	
	#include "global.h"
	#include "Song.h"
	#include "Instrument.h"
	#include "Player.h"
	#include "Configuration.h"
	#include "Dsp.h"
	#include "InputHandler.h"
	unsigned int Global::_cpuHz;
	Configuration* Global::pConfig = NULL;
	Song* Global::_pSong = NULL;
	Player* Global::pPlayer = NULL;
	Resampler* Global::pResampler = NULL;
	InputHandler* Global::pInputHandler = NULL;
	SF::string Global::m_CurrentDirectory = _T("");
	
	Global::Global()
	{
		pConfig = new Configuration;
		_pSong = new Song;
		pPlayer = new Player;
		pResampler = new Cubic;
		pResampler->SetQuality(RESAMPLE_LINEAR);
		pInputHandler = new InputHandler;
		TCHAR _dir[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH,_dir);
		m_CurrentDirectory = _dir;

	}

	Global::~Global()
	{
		delete pConfig;
		delete _pSong;
		delete pPlayer;
		delete pResampler;
		delete pInputHandler;
	}
#endif //  _WINAMP_PLUGIN_
