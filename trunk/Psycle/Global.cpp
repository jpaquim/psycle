#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
//	#include "global.h"
	#include "Song.h"
	#include "Player.h"
	#include "Dsp.h"
	#include "Configuration.h"

	Song* Global::_pSong = NULL;
	Player* Global::pPlayer = NULL;
	Resampler* Global::pResampler = NULL;
	Configuration* Global::pConfig = NULL;
	int Global::_lbc = -1;
	
	Global::Global()
	{
		_pSong = new Song;
		pPlayer = new Player;
		pConfig = new Configuration;
		pResampler = new Cubic;
		pResampler->SetQuality(RESAMPLE_LINEAR);
		_lbc = -1;
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
		CFileFind finder;
		int loop = finder.FindFile(path + "\\*.");	// check for subfolders.
		while (loop) {								// Note: Subfolders with dots won't work.
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
		
		if (finder.FindFile(path + "\\" + dllname)) //not found in subdirectories, lets see if it's here
		{
			finder.Close();
			path= (path + "\\") + dllname;
			return true;
		}
		finder.Close();
		return false;
	}

#else
//	#include "global.h"
	#include "Song.h"
	#include "Player.h"
	#include "Configuration.h"
	#include "Dsp.h"
	#include "InputHandler.h"

	unsigned int Global::_cpuHz;
	int Global::_lbc = -1;
	Song* Global::_pSong = NULL;
	Player* Global::pPlayer = NULL;
	Configuration* Global::pConfig = NULL;
	Resampler* Global::pResampler = NULL;
	InputHandler* Global::pInputHandler = NULL;

	Global::Global()
	{
		_pSong = new Song;
		pPlayer = new Player;
		_lbc = -1;
		pConfig = new Configuration;
		pResampler = new Cubic;
		pResampler->SetQuality(RESAMPLE_LINEAR);
		pInputHandler = new InputHandler;
	}

	Global::~Global()
	{
		delete _pSong;
		delete pPlayer;
		delete pConfig;
		delete pResampler;
		delete pInputHandler;
	}
#endif //  _WINAMP_PLUGIN_
