#include <stdafx.h>
#include <global.h>
#include <Song.h>
#include <Player.h>
#include <Dsp.h>
#include <Configuration.h>
#include "LoggingWindow.h"
#if !defined _WINAMP_PLUGIN_
	#include <InputHandler.h>
#endif
///\file
///\brief implementation file for psycle::host::Global.
namespace psycle
{
	namespace host
	{
		Song * Global::_pSong(0);
		Player * Global::pPlayer(0);
		Resampler * Global::pResampler(0);
		Configuration * Global::pConfig(0);
		CLoggingWindow * Global::pLogWindow(0);
		#if !defined _WINAMP_PLUGIN_
			unsigned int Global::_cpuHz;
			InputHandler * Global::pInputHandler(0);
		#endif
			
		Global::Global()
		{
			_pSong = new Song;
			pPlayer = new Player;
			pConfig = new Configuration;
			pResampler = new Cubic;
			pResampler->SetQuality(RESAMPLE_LINEAR);
			#if !defined _WINAMP_PLUGIN_
				pInputHandler = new InputHandler;
			#endif
		}

		Global::~Global()
		{
			zapObject(_pSong);
			zapObject(pPlayer);
			zapObject(pResampler);
			zapObject(pConfig);
			zapObject(pLogWindow);
			#if !defined _WINAMP_PLUGIN_
				zapObject(pInputHandler);
			#endif
		}

		logger::logger(const int & level, const std::string & string) throw()
		{
			try
			{
				if(level > info)
					std::cerr << string << std::endl;
				else
					std::cout << string << std::endl;
				if(Global::pLogWindow) Global::pLogWindow->AddEntry(level, string);
			}
			catch(...)
			{
				// oh dear!
				std::cerr << "logger crashed" << std::endl;
			}
		}

		#if defined _WINAMP_PLUGIN_
			bool FindFileinDir(char *dllname,CString &path)
			{
				CFileFind finder;
				int loop = finder.FindFile(path + "\\*"); // check for subfolders.
				while(loop) 
				{						
					loop = finder.FindNextFile();
					if(finder.IsDirectory() && !finder.IsDots())
					{
						CString filepath = finder.GetFilePath();
						if(FindFileinDir(dllname,filepath))
						{
							path = filepath;
							return true;
						}
					}
				}
				finder.Close();
				if(finder.FindFile(path + "\\" + dllname)) // not found in subdirectories, lets see if it's here
				{
					finder.Close();
					path= (path + "\\") + dllname;
					return true;
				}
				finder.Close();
				return false;
			}
		#endif
	}
}
