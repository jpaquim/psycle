/*

  "Winamp .psy Player input plugin"

  This plugin plays Psycle Song files with Winamp 2.

*/

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/host/configuration.hpp>
#include <psycle/host/song.hpp>
#include <psycle/host/player.hpp>
#include <psycle/host/machine.hpp>
#include <psycle/host/helpers.hpp>

#include <math.h> // should be <cmath>

#include <winamp-2/in.h>	// Winamp Input plugin header file

#define WA_PLUGIN_VERSION "1.1a"

// post this to the main window at end of file (after playback has stopped)
#define WM_WA_PSY_EOF WM_USER+2

//
// Global Variables.
//
#define WA_STREAM_SIZE 576
DWORD WINAPI __stdcall PlayThread(void *b);
BOOL WINAPI CfgProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
BOOL WINAPI InfoProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

Global _global;
short stream_buffer[WA_STREAM_SIZE*4];
extern In_Module mod;

int killDecodeThread=0;
HANDLE thread_handle=INVALID_HANDLE_VALUE;

int paused;
int worked;
bool loading=false;
char infofileName[_MAX_PATH];

//
// InModule Functions:
//

void config(HWND w)
{
	DialogBox(mod.hDllInstance,(char*)IDD_CONFIGDLG,w,CfgProc);
}
void about(HWND hwndParent)
{
	MessageBox(hwndParent,"This Plugin plays .psy files using Winamp 2\nBased on Psycle Engine " VERSION_NUMBER "\n\nCoded by Psycledelics on " __DATE__ "\n\nSome of the code has been gathered from out_wave and in_mpc plugins.\nThanks to their authors.","Psycle Winamp 2 Plugin",MB_OK);
}

void init()
{
	if (!_global.pConfig->Initialized())
	{
		if (!_global.pConfig->Read())
		{
			config(mod.hMainWindow);
		}
	}
	_global._pSong->fileName[0] = '\0';
	_global._pSong->New();
}

void quit() { }

int CalcSongLength(Song *pSong)
{
	// take ff and fe commands into account
	
	float songLength = 0;
	int bpm = pSong->BeatsPerMin;
	int tpb = pSong->_ticksPerBeat;
	for (int i=0; i <pSong->playLength; i++)
	{
		int pattern = pSong->playOrder[i];
		// this should parse each line for ffxx commands if you want it to be truly accurate
		unsigned char* const plineOffset = pSong->_ppattern(pattern);
		for (int l = 0; l < pSong->patternLines[pattern]*MULTIPLY; l+=MULTIPLY)
		{
			for (int t = 0; t < pSong->SONGTRACKS*5; t+=5)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset+l+t);
				switch (pEntry->_cmd)
				{
				case 0xFF:
					if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
					{
						bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
					}
					break;
					
				case 0xFE:
					if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
					{
						tpb=pEntry->_parameter;
					}
				}
			}
			songLength += (60.0f/(bpm * tpb));
		}
	}
	
	return f2i(songLength*1000.0f);
}

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename) // Current Playing
	{
		if (_global.pPlayer->_playing)
		{
			if (title) { sprintf(title,"%s - %s\0",_global._pSong->Author,_global._pSong->Name); }
			
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
				if (title) { sprintf(title,"%s - %s\0",pSong->Author,pSong->Name); }
				if (length_in_ms)
				{
					*length_in_ms = CalcSongLength(pSong);
				}
//				file.Close(); <- load handles this
				return;
			}
			else if (strcmp(Header,"PSY2SONG")==0)
			{
				char Name[33], Author[33];
				int bpm, tpb, spt, num, playLength, patternLines[MAX_PATTERNS];
				unsigned char playOrder[MAX_SONG_POSITIONS];
				
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
					file.Skip(sizeof(unsigned char)*MAX_BUSES); // BusMachine

					file.Read(&playOrder, sizeof(playOrder));
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
					for (i=0; i <playLength; i++)
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
	if ( strcmp(fn,_global._pSong->fileName) ) // if not the current one
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
	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		killDecodeThread=1;
		if (WaitForSingleObject(thread_handle,INFINITE) == WAIT_TIMEOUT)
		{
			MessageBox(mod.hMainWindow,"error asking thread to die!\n","error killing decode thread",0);
			TerminateThread(thread_handle,0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}
	_global._pSong->New();
	mod.outMod->Close();
	mod.SAVSADeInit();

}

int play(char *fn)
{
	int maxlatency;
	unsigned long tmp;

	_global.pPlayer->Stop();//	stop();

	OldPsyFile file;
	if (!file.Open(fn))
	{
		return -1;
	}
/*	while ( mod.outMod->IsPlaying())
	{
		Sleep(10);
	}*/
	_global._pSong->filesize=file.FileSize();
	loading = true;
//	_global._pSong->New();
	_global._pSong->Load(&file);
	file.Close(); //<- load handles this (but maybe nto always)
	strcpy(_global._pSong->fileName,fn);
	_global._pSong->SetBPM(_global._pSong->BeatsPerMin, _global._pSong->_ticksPerBeat, _global.pConfig->_samplesPerSec);
	int val=64;
	_global.pPlayer->Work(_global.pPlayer,val); // Some plugins don't like to receive data without making first a
								// work call. (for example, Phantom)
	_global.pPlayer->Start(0,0);
	_global.pPlayer->_loopSong=false;

	paused=0; worked=false;
	memset(stream_buffer,0,sizeof(stream_buffer));

	maxlatency = mod.outMod->Open(_global.pConfig->_samplesPerSec,2,16, -1,-1);
	if (maxlatency < 0)
	{
		return 1;
	}
	mod.SetInfo(_global._pSong->BeatsPerMin,_global.pConfig->_samplesPerSec/1000,2,1);
	mod.SAVSAInit(maxlatency,_global.pConfig->_samplesPerSec);
	mod.VSASetInfo(_global.pConfig->_samplesPerSec,2);
	mod.outMod->SetVolume(-666);

	killDecodeThread=0;
	thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) PlayThread,(void *) &killDecodeThread,0,&tmp);
	loading = false;
	return 0;
}

void pause() { paused=1; mod.outMod->Pause(1); }
void unpause() { paused=0; mod.outMod->Pause(0); }
int ispaused() { return paused; }


int getlength() { return CalcSongLength(Global::_pSong); }
int getoutputtime() { return mod.outMod->GetOutputTime(); }
void setoutputtime(int time_in_ms)
{
	Song* pSong = _global._pSong;
	int time_left = time_in_ms;
	int patline=-1;
	for ( int i=0;i<pSong->playLength;i++)
	{
		int pattern = pSong->playOrder[i];
		int tmp;
		if ((tmp = pSong->patternLines[pattern] * 60000/(pSong->BeatsPerMin * pSong->_ticksPerBeat)) >= time_left )
		{
			patline = time_left * (pSong->BeatsPerMin * pSong->_ticksPerBeat)/60000;
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
	"Psycle Winamp 2 Plugin " WA_PLUGIN_VERSION ,
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

void Quantize(float *pin, short *piout, int c)
{
	float* inb =pin;
	short* outb =piout;
	for (int i=0; i<c; i++)
	{
		if ( *inb > 32767.0f) *outb = 32767;
		else if ( *inb < -32767.0f ) *outb = -32768;
		else *outb = f2i(*inb);
		*inb++; outb++;
	}
}

DWORD WINAPI __stdcall PlayThread(void *b)
{
	float *float_buffer;
	Player* pPlayer = _global.pPlayer;
	int samprate = _global.pConfig->_samplesPerSec;
	int smp2 = _global.pConfig->_samplesPerSec/1000;
//	int plug_stream_size = samprate/200;
	int plug_stream_size = WA_STREAM_SIZE;
//	int plug_stream_size = 1024;

	while (! *((int *)b) )  // While !killDecodeThread
	{
		if ( !worked)
		{
			if (pPlayer->_playing)
			{	int bmp = _global._pSong->BeatsPerMin;
				float_buffer = pPlayer->Work(pPlayer,plug_stream_size);
				Quantize(float_buffer,stream_buffer,plug_stream_size*2);
				if ( bmp != _global._pSong->BeatsPerMin ) mod.SetInfo(_global.pPlayer->bpm,smp2,2,1);
				worked=true;
			}
			else
			{
				mod.outMod->CanWrite();
				if (!mod.outMod->IsPlaying())
				{
					PostMessage(mod.hMainWindow,WM_WA_PSY_EOF,0,0);
					return 0;
				}
				Sleep(10);
			}
		}
		else if (mod.outMod->CanWrite() >= (plug_stream_size<<(mod.dsp_isactive()?3:2)))
		{
			int t;
			if (mod.dsp_isactive()) t=mod.dsp_dosamples(stream_buffer,plug_stream_size,16,2,samprate)*4;
			else t=plug_stream_size*4;

			int s=mod.outMod->GetWrittenTime();
			mod.SAAddPCMData((char*)stream_buffer,2,16,s);
			mod.VSAAddPCMData((char*)stream_buffer,2,16,s);

			mod.outMod->Write((char*)stream_buffer,t);
			worked=false;
		}
		else Sleep(20);
	}
	return 0;
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
		w=GetDlgItem(wnd,IDC_SAMP_RATE);
		char valstr[10];
		for (c=0;c<4;c++)
		{
			sprintf(valstr,"%i",(int)(11025*powf(2.0f,(float)c)));
			SendMessage(w,CB_ADDSTRING,0,(long)valstr);
			sprintf(valstr,"%i",(int)(12000*powf(2.0f,(float)c)));
			SendMessage(w,CB_ADDSTRING,0,(long)valstr);
		}
		switch (_global.pConfig->_samplesPerSec)
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
		
		// Directories.
		
		SetDlgItemText(wnd,IDC_EDIT_NATIVE,_global.pConfig->GetPluginDir());
		SetDlgItemText(wnd,IDC_EDIT_VST,_global.pConfig->GetVstDir());
		
		return 1;
		break;

	case WM_COMMAND:
		switch(wp)
		{
		case IDOK:
			if (_global.pPlayer->_playing ) stop();

			c = SendDlgItemMessage(wnd,IDC_SAMP_RATE,CB_GETCURSEL,0,0);
			if ( (c % 2) == 0) _global.pConfig->_samplesPerSec = (int)(11025*powf(2.0f,(float)(c/2)));
			else _global.pConfig->_samplesPerSec = (int)(12000*powf(2.0f,(float)(c/2)));
			
			GetDlgItemText(wnd,IDC_EDIT_NATIVE,tmptext,_MAX_PATH);
			_global.pConfig->SetPluginDir(tmptext);
			GetDlgItemText(wnd,IDC_EDIT_VST,tmptext,_MAX_PATH);
			_global.pConfig->SetVstDir(tmptext);
			EndDialog(wnd,1);
			break;
		case IDCANCEL:
			EndDialog(wnd,0);
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
				strcpy(pSong->fileName,infofileName);
//				file.Close(); <- load handles this
			}
		}
		else pSong= _global._pSong;
		
		SetWindowText(wnd,"Psycle Winamp Plugin Info Dialog");
		char tmp2[20];
		SetDlgItemText(wnd,IDC_SONGFILENAME,pSong->fileName);
		SetDlgItemText(wnd,IDC_SONGARTIST,pSong->Author);
		SetDlgItemText(wnd,IDC_SONGTITLE,pSong->Name);
		SetDlgItemText(wnd,IDC_SONGCOMMENT,pSong->Comment);

		for( i=0;i<MAX_MACHINES;i++)
		{
			if(pSong->_pMachine[i])
			{
				switch( pSong->_pMachine[i]->_type )
				{
					case MACH_VST: strcpy(tmp2,"V");break;
					case MACH_VSTFX: strcpy(tmp2,"V");break;
					case MACH_PLUGIN: strcpy(tmp2,"P");break;
					default: strcpy(tmp2,"_"); break;
				}
				
				if ( pSong->_pMachine[i]->_type == MACH_DUMMY )
				{
					if ( ((Dummy*)pSong->_pMachine[i])->wasVST )
					{
						sprintf(valstr,"%.02i:[*]  %s",i,pSong->_pMachine[i]->_editName);
					}
					else sprintf(valstr,"%.02i:[?]  %s",i,pSong->_pMachine[i]->_editName);
				}
				else sprintf(valstr,"%.02i:[%s]  %s",i,tmp2,pSong->_pMachine[i]->_editName);
				
				SendDlgItemMessage(wnd,IDC_MACHINELIST,LB_ADDSTRING,0,(long)valstr);
				j++;
			}
		}
		
		i=CalcSongLength(pSong)/1000;

		sprintf(valstr,"Filesize: %i\nBeatsPerMin: %i\nLinesPerBeat: %i\n\
Song Length: %02i:%02i\nPatternsUsed: %i\nMachines Used: %i",
			pSong->filesize,
			pSong->BeatsPerMin,
			pSong->_ticksPerBeat,
			i / 60, i % 60,
			pSong->GetNumPatternsUsed(),
			j);
		SetDlgItemText(wnd,IDC_EXTINFO,valstr);
		
		
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