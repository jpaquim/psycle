// -*- mode:c++; indent-tabs-mode:t -*-
/*

  Winamp .psy Player input plugin
  -------------------------------

  This plugin plays Psycle Song files with Winamp 2.x/5.x
  and compatible players.

*/

#include <psycle/project.private.hpp>
//#include "mfc_wrapped_classes.hpp"
#include "../../version.hpp"
#include "../../global.hpp"
#include "../../configuration.hpp"
#include "../../Song.hpp"
#include "../../player.hpp"
#include "../../machine.hpp"
#include "../../internal_machines.hpp"
#include "../../helpers.hpp"
#include "shrunk_newmachine.hpp"
#include "resources.hpp"
#include "winampdriver.hpp"
#include <psycle/helpers/math.hpp>

#include <winamp-2/in2.h>	// Winamp Input plugin header file


#define WA_PLUGIN_VERSION "1.2"

using namespace psycle::host;

//
// Global Variables.
//
Global _global;
WinampDriver wadriver;

bool loading=false;
char infofileName[_MAX_PATH];

extern In_Module mod;

//
// InModule Functions:
//
BOOL WINAPI CfgProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
BOOL WINAPI InfoProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
bool BrowseForFolder(HWND m_hWnd,std::string& rpath);

void config(HWND w)
{
	DialogBox(mod.hDllInstance,(char*)IDD_CONFIGDLG,w,CfgProc);
}
void about(HWND hwndParent)
{
	MessageBox(hwndParent,"This Plugin plays .psy files using Winamp 2.x/5.x\nBased on Psycle Engine " PSYCLE__VERSION "\n\nCoded by Psycledelics on " __DATE__ "\n\nSome of the code has been gathered from out_wave and in_mpc plugins.\nThanks to their authors.","Psycle Winamp Plugin",MB_OK);
}

void init()
{
	_global.pConfig->_pOutputDriver = &wadriver;
	if (!_global.pConfig->Initialized())
	{
		if (!_global.pConfig->Read())
		{
			config(mod.hMainWindow);
		}
	}
	wadriver.Initialize(mod.hMainWindow,_global.pPlayer->Work,_global.pPlayer);
	CNewMachine::LoadPluginInfo(false);
	_global._pSong->fileName[0] = '\0';
	_global._pSong->New();
}

void quit() { }

int CalcSongLength(Song *pSong)
{
	// take ff and fe commands into account
	
	float songLength = 0;
	int bpm = pSong->BeatsPerMin();
	int tpb = pSong->LinesPerBeat();
	for (int i=0; i <pSong->playLength; i++)
	{
		int pattern = pSong->playOrder[i];
		// this should parse each line for ffxx commands if you want it to be truly accurate
		unsigned char* const plineOffset = pSong->_ppattern(pattern);
		for (int l = 0; l < pSong->patternLines[pattern]*psycle::host::MULTIPLY; l+=psycle::host::MULTIPLY)
		{
			for (int t = 0; t < pSong->SONGTRACKS*5; t+=5)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset+l+t);
				if(pEntry->_note < notecommands::tweak || pEntry->_note == 255) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->_cmd)
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{
							bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
						}
						break;
					case PatternCmd::EXTENDED:
						if((pEntry->_parameter != 0) && ( (pEntry->_parameter&0xE0) == 0 )) // range from 0 to 1F for LinesPerBeat.
						{
							tpb=pEntry->_parameter;
						}
						break;
					}
				}
			}
			songLength += (60.0f/(bpm * tpb));
		}
	}
	
	return psycle::helpers::math::rounded(songLength*1000.0f);
}

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename) // Current Playing
	{
		if (_global.pPlayer->_playing)
		{
			if (title) { sprintf(title,"%s - %s\0",_global._pSong->author.c_str(),_global._pSong->name.c_str()); }
			
			if (length_in_ms) { *length_in_ms = CalcSongLength(_global._pSong); }
		}
	}
	else
	{
		OldPsyFile file;
		char Header[9];

		if (file.Open(filename))
		{
	
			file.Read(&Header, 8);
			Header[8]=0;
			
			if (strcmp(Header,"PSY3SONG")==0)
			{
				Song *pSong;
				pSong=new Song;
				pSong->New();
				file.Seek(0);
				pSong->Load(&file,false);

				if (title) { sprintf(title,"%s - %s\0",pSong->author.c_str(),pSong->name.c_str()); }
				if (length_in_ms)
				{
					*length_in_ms = CalcSongLength(pSong);
				}
//				file.Close(); <- load handles this
				delete pSong;
				return;
			}
			else if (strcmp(Header,"PSY2SONG")==0)
			{
				char Name[33], Author[33];
				int bpm, tpb, spt, num, playLength, patternLines[MAX_PATTERNS];
				unsigned char playOrder[128];
				
				file.Read(Name, 32); Name[32]='\0';
				file.Read(Author, 32); Author[32]='\0';
				if (title) { sprintf(title,"%s - %s\0",Author,Name); }

				if (length_in_ms) { 
					file.Skip(128); // Comment;
					file.Read(&bpm, sizeof(bpm));
					file.Read(&spt, sizeof(spt));
					if ( spt <= 0 )  // Shouldn't happen, but has happened. (bug of 1.1b1)
					{	tpb= 4; spt = 4315;
					}
					else tpb = 44100*15*4/(spt*bpm);

					file.Skip(sizeof(unsigned char)); // currentOctave
					file.Skip(sizeof(unsigned char)*64); // BusMachine

					file.Read(&playOrder, 128);
					file.Read(&playLength, sizeof(playLength));
					file.Skip(sizeof(int));	//SONG_TRACKS

					// Patterns
					//
					file.Read(&num, sizeof(num));
					for (int i=0; i<num; i++)
					{
						file.Read(&patternLines[i], sizeof(patternLines[0]));
						file.Skip(sizeof(char)*32);	// Pattern Name
						file.Skip(patternLines[i]*OLD_MAX_TRACKS*sizeof(PatternEntry)); // Pattern Data
					}
					
					*length_in_ms = 0;
					for (int i=0; i <playLength; i++)
					{
						*length_in_ms += (patternLines[playOrder[i]] * 60000/(bpm * tpb));
					}
					
				}
				file.Close();
				return;
			}
			file.Close();
		}
		if (title)
		{
			char *p=filename+strlen(filename);
			while (*p != '\\' && p >= filename) p--;
			strcpy(title,++p);
		}
		if (length_in_ms ) *length_in_ms = -1000;
	}
}

int infoDlg(char *fn, HWND hwnd)
{
	if ( strcmp(fn,_global._pSong->fileName.c_str()) ) // if not the current one
	{
		strcpy(infofileName,fn);
	}
	else infofileName[0]='\0';
	
	DialogBox(mod.hDllInstance,(char*)IDD_INFODLG,hwnd,InfoProc);
	
	return 0;
}

int isourfile(char *fn)
{
	OldPsyFile file;
	char Header[9];
	
	if (file.Open(fn))
	{
		file.Read(&Header, 8);
		Header[8]=0;
		
		if (strcmp(Header,"PSY3SONG")==0)
		{
			file.Close();
			return 1;
		}
		else if (strcmp(Header,"PSY2SONG")==0)
		{
			file.Close();
			return 1;
		}
		file.Close();
	}
	return 0;
}

void stop()
{ 
	while (loading) Sleep(10);
	_global.pConfig->_pOutputDriver->Enable(false);
	_global._pSong->New();
	mod.SAVSADeInit();

}

int play(char *fn)
{
	_global.pPlayer->Stop();//	stop();

	OldPsyFile file;
	if (!file.Open(fn))
	{
		return -1;
	}

	_global._pSong->filesize=file.FileSize();
	loading = true;
//	_global._pSong->New();
	_global._pSong->Load(&file);
	file.Close(); //<- load handles this (but maybe nto always)
	_global._pSong->fileName = fn;
	int val=256;
	_global.pPlayer->Work(_global.pPlayer,val); // Some plugins don't like to receive data without making first a
	// work call. (for example, Phantom)
	_global.pPlayer->Start(0,0);
	_global.pPlayer->_loopSong=false;

	_global.pConfig->_pOutputDriver->Enable(true);
	mod.SetInfo(_global._pSong->BeatsPerMin(),_global.pConfig->GetSamplesPerSec()/1000,2,1);
	mod.SAVSAInit(wadriver.GetOutputLatency(),_global.pConfig->GetSamplesPerSec());
	mod.VSASetInfo(_global.pConfig->GetSamplesPerSec(),2);

	loading = false;
	return 0;
}

void pause() { wadriver.Pause(true); }
void unpause() { wadriver.Pause(false); }
int ispaused() { return wadriver.Paused(); }


int getlength() { return CalcSongLength(Global::_pSong); }
int getoutputtime() { return mod.outMod->GetOutputTime(); }
void setoutputtime(int time_in_ms)
{
	Song* pSong = _global._pSong;
	int time_left = time_in_ms;
	int patline=-1;
	int i;
	for (i=0;i<pSong->playLength;i++)
	{
		int pattern = pSong->playOrder[i];
		int tmp;
		if ((tmp = pSong->patternLines[pattern] * 60000/(pSong->BeatsPerMin() * pSong->LinesPerBeat())) >= time_left )
		{
			patline = time_left * (pSong->BeatsPerMin() * pSong->LinesPerBeat())/60000;
			break;
		}
		else time_left-=tmp;
	}
	_global.pPlayer->Start(i,patline);

	mod.outMod->Flush(time_in_ms);
}

void setvolume(int volume) { mod.outMod->SetVolume(volume); }
void setpan(int pan) { mod.outMod->SetPan(pan); }

void eq_set(int on, char data[10], int preamp) { }

In_Module mod = 
{
	IN_VER,
	"Psycle Winamp Plugin v" WA_PLUGIN_VERSION ,
	NULL,
	NULL,
	"psy\0Psycle Song (*.psy)\0",
	1,
	1,
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	
	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0, // vis stuff

	0,0, // dsp

	eq_set,

	NULL,// setinfo

	NULL // out_mod

};

//
// Exported Symbols
//

extern "C" __declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &mod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}


BOOL WINAPI CfgProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	HWND w;
	int c;
	char tmptext[_MAX_PATH];
	
	switch(msg)
	{
	case WM_INITDIALOG:
		
		//	Sample Rate Combobox
		w=GetDlgItem(wnd,IDC_SRATE);
		char valstr[10];
		for (c=0;c<4;c++)
		{
			sprintf(valstr,"%i",(int)(11025*powf(2.0f,(float)c)));
			SendMessage(w,CB_ADDSTRING,0,(long)valstr);
			sprintf(valstr,"%i",(int)(12000*powf(2.0f,(float)c)));
			SendMessage(w,CB_ADDSTRING,0,(long)valstr);
		}
		switch (_global.pConfig->GetSamplesPerSec())
		{
		case 11025: SendMessage(w,CB_SETCURSEL,0,0);break;
		case 12000: SendMessage(w,CB_SETCURSEL,1,0);break;
		case 22050: SendMessage(w,CB_SETCURSEL,2,0);break;
		case 24000: SendMessage(w,CB_SETCURSEL,3,0);break;
		case 44100: SendMessage(w,CB_SETCURSEL,4,0);break;
		case 48000: SendMessage(w,CB_SETCURSEL,5,0);break;
		case 88200: SendMessage(w,CB_SETCURSEL,6,0);break;
		case 96000: SendMessage(w,CB_SETCURSEL,7,0);break;
		}
		
		// Autostop
		w=GetDlgItem(wnd,IDC_AUTOSTOP);
		SendMessage(w,BM_SETCHECK,_global.pConfig->autoStopMachines?1:0,0);
		
		
		// Directories.
		SetDlgItemText(wnd,IDC_NATIVEPATH,_global.pConfig->GetPluginDir().c_str());
		SetDlgItemText(wnd,IDC_VSTPATH,_global.pConfig->GetVstDir().c_str());
		
		// Cache
		SetDlgItemText(wnd,IDC_CACHEVALID,CNewMachine::IsLoaded()?"File Exists.":"Not Found. Rengerate.");

		return 1;
		break;

	case WM_COMMAND:
		switch(wp)
		{
		case IDOK:
			if (_global.pPlayer->_playing ) stop();

			c = SendDlgItemMessage(wnd,IDC_SRATE,CB_GETCURSEL,0,0);
			if ( (c % 2) == 0) _global.pConfig->SetSamplesPerSec((int)(11025*powf(2.0f,(float)(c/2))));
			else _global.pConfig->SetSamplesPerSec((int)(12000*powf(2.0f,(float)(c/2))));

			c = SendDlgItemMessage(wnd,IDC_AUTOSTOP,BM_GETCHECK,0,0);
			_global.pConfig->autoStopMachines=c>0?true:false;

			GetDlgItemText(wnd,IDC_NATIVEPATH,tmptext,_MAX_PATH);
			_global.pConfig->SetPluginDir(tmptext);
			GetDlgItemText(wnd,IDC_VSTPATH,tmptext,_MAX_PATH);
			_global.pConfig->SetVstDir(tmptext);
			EndDialog(wnd,1);
			break;
		case IDCANCEL:
			EndDialog(wnd,0);
			break;
		case IDC_REGENERATE:
			CNewMachine::Regenerate();
			SetDlgItemText(wnd,IDC_CACHEVALID,CNewMachine::IsLoaded()?"File Exists.":"Not Found. Rengerate.");
			break;
		case IDC_BWNATIVE:
			{
				std::string plugdir = _global.pConfig->GetPluginDir();
				if (BrowseForFolder(wnd,plugdir))
				{
					_global.pConfig->SetPluginDir(plugdir);
				};
			}
			break;
		case IDC_BWVST:
			{
				std::string plugdir = _global.pConfig->GetVstDir();
				if (BrowseForFolder(wnd,plugdir))
				{
					_global.pConfig->SetVstDir(plugdir);
				};
			}
			break;
		}
		break;
	}
	return 0;
}

BOOL WINAPI InfoProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	int i,j=0;
	Song* pSong;
	
	switch(msg)
	{
	case WM_INITDIALOG:
		char valstr[255];
		if ( infofileName[0]!='\0' )
		{
			pSong=new Song;
			OldPsyFile file;
			pSong->New(); // this is NOT done in Load for the winamp plugin.
			if (file.Open(infofileName))
			{
				pSong->filesize=file.FileSize();
				pSong->Load(&file,false);
				pSong->fileName = infofileName;
//				file.Close(); <- load handles this
			}
		}
		else pSong= _global._pSong;
		
		SetWindowText(wnd,"Psycle Song Information");
		char tmp2[20];
		SetDlgItemText(wnd,IDC_FILENAME,pSong->fileName.c_str());
		SetDlgItemText(wnd,IDC_ARTIST,pSong->author.c_str());
		SetDlgItemText(wnd,IDC_TITLE,pSong->name.c_str());
		SetDlgItemText(wnd,IDC_COMMENTS,pSong->comments.c_str());

		for( i=0;i<MAX_MACHINES;i++)
		{
			if(pSong->_pMachine[i])
			{
				switch( pSong->_pMachine[i]->_type )
				{
					case MACH_VST: strcpy(tmp2,"V");break;
					case MACH_VSTFX: strcpy(tmp2,"V");break;
					case MACH_PLUGIN: strcpy(tmp2,"N");break;
					case MACH_MASTER: strcpy(tmp2,"M");break;
					default: strcpy(tmp2,"I"); break;
				}
				
				if ( pSong->_pMachine[i]->_type == MACH_DUMMY && ((Dummy*)pSong->_pMachine[i])->wasVST )
				{
					sprintf(valstr,"%.02x:[!]  %s",i,pSong->_pMachine[i]->_editName);
				}
				else sprintf(valstr,"%.02x:[%s]  %s",i,tmp2,pSong->_pMachine[i]->_editName);
				
				SendDlgItemMessage(wnd,IDC_MACHINES,LB_ADDSTRING,0,(long)valstr);
				j++;
			}
		}
		
		i=CalcSongLength(pSong)/1000;

		sprintf(valstr,"Filesize: %i\nBeatsPerMin: %i\nLinesPerBeat: %i\n\
Song Length: %02i:%02i\nPatternsUsed: %i\nMachines Used: %i",
			pSong->filesize,
			pSong->BeatsPerMin(),
			pSong->LinesPerBeat(),
			i / 60, i % 60,
			pSong->GetNumPatternsUsed(),
			j);
		SetDlgItemText(wnd,IDC_INFO,valstr);
		
		
		if ( infofileName[0]!='\0' ) delete pSong;
			
		break;
	case WM_COMMAND:
		switch(wp)
		{
		case IDCANCEL:
			EndDialog(wnd,0);
			break;
		}
		break;
	}
	return 0;
}

bool BrowseForFolder(HWND m_hWnd,std::string& rpath)
{
	bool val=false;

	LPMALLOC pMalloc;
	// Gets the Shell's default allocator
	//
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		BROWSEINFO bi;
		char pszBuffer[MAX_PATH];
		LPITEMIDLIST pidl;
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		//
		bi.hwndOwner = m_hWnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = _T("Select Directory");
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
		bi.lpfn = NULL;
		bi.lParam = 0;
		// This next call issues the dialog box.
		//
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
			{
				// At this point pszBuffer contains the selected path
				//
				val = true;
				rpath =pszBuffer;
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			//
			pMalloc->Free(pidl);
		}
		// Release the shell's allocator.
		//
		pMalloc->Release();
	}
	return val;
}

