/*

  "Winamp .psy Player input plugin"

  This plugin plays Psycle Song files with Winamp.

  Note: Source Code should be common for both, the tracker and the plugin.
		There will be common sources in the future.

*/

#include "..\Global.h"

#include "..\configuration.h"
#include "..\Song.h"
#include "..\player.h"


#include "in2.h"	// Winamp Input plugin header file

// post this to the main window at end of file (after playback as stopped)
#define WM_WA_MPEG_EOF WM_USER+2

//
// Global Variables.
//

DWORD WINAPI __stdcall PlayThread(void *b);

Global _global;
short stream_buffer[576<<2];
extern In_Module mod;

int killDecodeThread=0;
HANDLE thread_handle=INVALID_HANDLE_VALUE;

int paused;
int worked;

//
// InModule Functions:
//

void config(HWND hwndParent)
{
}
void about(HWND hwndParent)
{
	MessageBox(hwndParent,"This Plugin plays .psy files using Winamp\nBased on Psycle Engine 1.6Plus\n\nCoded by [JAZ] on " __DATE__,"Psycle Winamp 2 Plugin",MB_OK);
}

void init()
{
	if (!_global.pConfig->Initialized())
	{
		if (!_global.pConfig->Read())
		{
			//Need of manual configuration
		}
	}
	_global._pSong->fileName[0] = '\0';
	_global._pSong->fileName[0] = '\0';
	_global._pSong->New();
}

void quit() { }

int CalcSongLength(Song *pSong)
{
	int songsize = 0;
	for (int i=0; i <pSong->playLength; i++)
	{
		int pattern = pSong->playOrder[i];
		songsize += (pSong->patternLines[pattern] * 60000/(pSong->BeatsPerMin * pSong->_ticksPerBeat));
	}
	return songsize;
}

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename) // Current Playing
	{
		if (title) { sprintf(title,"%s - %s",_global._pSong->Author,_global._pSong->Name); }

		if (length_in_ms) { *length_in_ms = CalcSongLength(_global._pSong); }
	}
	else
	{
		OldPsyFile file;
		if (!file.Open(filename) || !file.Expect("PSY2SONG", 8))
		{
			if (title)
			{
				char *p=filename+strlen(filename);
				while (*p != '\\' && p >= filename) p--;
				strcpy(title,++p);
			}
			if (length_in_ms ) *length_in_ms = -1000;
		}
		else
		{
			char Name[33], Author[33];
			int bpm, tpb, spt, num, playLength, patternLines[MAX_PATTERNS];
			unsigned char playOrder[MAX_SONG_POSITIONS];
			
			file.Read(Name, 32); Name[33]='\0';
			file.Read(Author, 32); Author[33]='\0';
			if (title) { sprintf(title,"%s - %s",Author,Name); }

			if (length_in_ms) { 
				file.Skip(128); // Comment;
				file.Read(&bpm, sizeof(bpm));
				file.Read(&spt, sizeof(spt));
				if ( spt <= 0 ) { // Shouldn't happen but has happened. (bug of 1.1b1)
					tpb= 4; spt = 4315;
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
					file.Skip(patternLines[i]*MAX_TRACKS*sizeof(PatternEntry)); // Pattern Data
				}
				
				*length_in_ms = 0;
				for (i=0; i <playLength; i++)
				{
					*length_in_ms += (patternLines[playOrder[i]] * 60000/(bpm * tpb));
				}
				
			}
		}
		file.Close();
	}
}

int infoDlg(char *fn, HWND hwnd)
{
	return 0;
}

int isourfile(char *fn) {  return 0;  }

void stop()
{ 
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

	mod.outMod->Close();
	mod.SAVSADeInit();
	
	_global._pSong->New();
}

int play(char *fn)
{
	int maxlatency;
	unsigned long tmp;

//	stop();

	OldPsyFile file;
	if (!file.Open(fn))
	{
		return -1;
	}
	_global._pSong->Load(&file);
	file.Close();
	strcpy(_global._pSong->fileName,fn);
	_global._pSong->SetBPM(_global._pSong->BeatsPerMin, _global._pSong->_ticksPerBeat, _global.pConfig->_samplesPerSec);

	_global.pPlayer->Start(0,0);

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
	_global.pPlayer->Start(patline);
	_global.pPlayer->_playPosition=i;
	_global.pPlayer->_playPattern=pSong->playOrder[i];

	mod.outMod->Flush(time_in_ms);
}

void setvolume(int volume) { mod.outMod->SetVolume(volume); }
void setpan(int pan) { mod.outMod->SetPan(pan); }

void eq_set(int on, char data[10], int preamp) { }

In_Module mod = 
{
	IN_VER,
	"Psycle Winamp 2 Plugin beta 5",
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
	for (int i=0; i<c; i++)
	{
		if ( *pin > 32767.0) *piout = 32767;
		else if ( *pin < -32767.0 ) *piout = -32768;
		else *piout = f2i(*pin);
		*pin++; piout++;
	}
}

DWORD WINAPI __stdcall PlayThread(void *b)
{
	float *float_buffer;
	Player* pPlayer = _global.pPlayer;
	int samprate = _global.pConfig->_samplesPerSec;
	int plug_stream_size = 1152;

	while (! *((int *)b) )  // While !killDecodeThread
	{
		if ( !worked)
		{
			if (pPlayer->_playing)
			{
				float_buffer = pPlayer->Work(pPlayer,plug_stream_size);
				Quantize(float_buffer,stream_buffer,plug_stream_size<<1);
				mod.SetInfo(_global.pPlayer->bpm,_global.pConfig->_samplesPerSec/1000,2,1);
				worked=true;
			}
			else
			{
				mod.outMod->CanWrite();
				if (!mod.outMod->IsPlaying())
				{
					PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
					return 0;
				}
				Sleep(10);
			}
		}
		else if (mod.outMod->CanWrite() >= (plug_stream_size<<(mod.dsp_isactive()?3:2)))
		{
			int t;
			if (mod.dsp_isactive()) t=mod.dsp_dosamples(stream_buffer,plug_stream_size,16,2,samprate)<<2;
			else t=plug_stream_size<<2;

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