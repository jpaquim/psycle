#include <packageneric/pre-compiled.private.hpp>

// Configure the Foobar2000 SDK $PATH before compiling 
#include <foobarSDK/foobar2000/SDK/foobar2000.h>

#include <math.h> // should be <cmath>
#include <shlobj.h>

// psycle
#include <psycle/host/configuration.hpp>
#include <psycle/host/song.hpp>
#include <psycle/host/player.hpp>
#include <psycle/host/machine.hpp>
#include <psycle/host/helpers.hpp>





Global _global;

cfg_int cfg_sampling_rate("sampling_rate",44100);

class input_psycle : public input
{
private:
// variables
	int		bPlaying;
	mem_block_aligned_t<audio_sample> buffers;
	reader *pReader;

public:

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


	void Quantize(float *pin, audio_sample *piout, int c)
	{

		float* inb =pin;
		audio_sample fMult;
		audio_sample* outb =piout;
		
		// saturate negation
		fMult = (1.0F / (audio_sample) 0x7FFF);

		for (int i=0; i<c; i++)
		{
			if ( *inb > 32767.0f) *outb = 1;
			else if ( *inb < -32767.0f ) *outb = -1;
			else *outb = (audio_sample)(*inb)*fMult;		
		
			*inb++; outb++;
		}
	}


	virtual bool test_filename(const char * fn,const char * ext) 
	{
		return !stricmp(ext,"psy");
	}


	// Constructor
	input_psycle()
	{
		pReader = NULL;
		bPlaying = NULL;
		if (!_global.pPlayer->_playing)
		{

			if (!_global.pConfig->Initialized())
			{

				if (!_global.pConfig->Read())
				{
					console::error("[Psycle Plugin] Please set the Plugins directories in the configuration!");
				}
			}
		
			_global._pSong->fileName[0] = '\0';
			_global._pSong->New();
		}
	}

	// Destructor
	~input_psycle()
	{	
			
	}

	int do_info(file_info * info, int full_open)
	{


	if (!full_open && bPlaying)
	{
			Song* pSong;
			int i;

			pSong= _global._pSong;
			i=CalcSongLength(pSong)/1000;

			info->meta_add("Artist",pSong->Author);
			info->meta_add("Title",pSong->Name);
			info->meta_add("Comment",pSong->Comment);

			info->set_length(i);
			info->info_set_int("Filesize", pSong->filesize);
			info->info_set_int("Bpm", pSong->BeatsPerMin);
			info->info_set_int("LinePerBeat", pSong->_ticksPerBeat);
			info->info_set_int("PatternsUsed", pSong->GetNumPatternsUsed());
			return 1;
	} else 
	{
		int length_in_ms;
		char Header[9];

		pReader->read(Header,8);
		Header[8]=0;
			
		if (strcmp(Header,"PSY3SONG")==0)
		{
			OldPsyFile file;
			string_ansi_from_utf8 _f(info->get_file_path()); ;
			char *filename;
			filename = strrchr(_f.get_ptr(), '/');
			if (filename)
				++filename;
			else filename = (char *)_f.get_ptr();

			if (file.Open(filename))
			{
				Song *pSong;
				pSong=new Song;
				pSong->New();
//////////////// Maybe a modification of Song::Load to not load the machines would
//////////////// be nice, to speed up the info loading.

				pSong->Load(&file,false);

				info->meta_add("Artist",pSong->Author);
				info->meta_add("Title",pSong->Name);
				info->meta_add("Comment",pSong->Comment);

				length_in_ms=CalcSongLength(pSong)/1000;
				info->set_length(length_in_ms);
				info->info_set_int("Filesize", pSong->filesize);
				info->info_set_int("Bpm", pSong->BeatsPerMin);
				info->info_set_int("LinePerBeat", pSong->_ticksPerBeat);
				info->info_set_int("PatternsUsed", pSong->GetNumPatternsUsed());
				info->info_set("Fileformat", "1.7.4");
				
//				file.Close(); <- load handles this
				return 1;
			}
		}
		else if (strcmp(Header,"PSY2SONG")==0)
		{
			char Title[33], Artist[33], Comments[129];
			int bpm, tpb, spt, num, playLength, patternLines[MAX_PATTERNS],i;
			unsigned char playOrder[MAX_SONG_POSITIONS];
			
			pReader->read(Title, 32); Title[32]='\0';
			pReader->read(Artist, 32); Artist[32]='\0';
			pReader->read(Comments, 128); Comments[128]='\0';
			info->meta_add("Artist",Artist);
			info->meta_add("Title",Title);
			info->meta_add("Comment",Comments);
			
			pReader->read(&bpm, sizeof(bpm));
			pReader->read(&spt, sizeof(spt));
			if ( spt <= 0 )  // Shouldn't happen, but has happened. (bug of 1.1b1)
			{	tpb= 4; spt = 4315;
			}
			else tpb = 44100*15*4/(spt*bpm);

			info->info_set_int("Filesize", pReader->get_length());
			info->info_set_int("Bpm", bpm);
			info->info_set_int("LinePerBeat", tpb);
			
			pReader->seek(pReader->get_position() + sizeof(unsigned char));
			pReader->seek(pReader->get_position() + sizeof(unsigned char)*MAX_BUSES);
			pReader->read(&playOrder, sizeof(playOrder));
			pReader->read(&playLength, sizeof(playLength));
			pReader->seek(pReader->get_position() + sizeof(int));	//SONG_TRACKS
			
			// Patterns
			pReader->read(&num, sizeof(num));
			
			info->info_set_int("PatternsUsed",num);
			info->info_set("Fileformat", "1.6 or older");
			
			for (i=0; i<num; i++)
			{
				pReader->read(&patternLines[i], sizeof(patternLines[0]));
				pReader->seek(pReader->get_position() + sizeof(char)*32);
				pReader->seek(pReader->get_position() + patternLines[i]*OLD_MAX_TRACKS*sizeof(PatternEntry));
			}
			
			length_in_ms = 0;
			for (i=0; i <playLength; i++)
			{
				length_in_ms += (patternLines[playOrder[i]] * 60000/(bpm * tpb));
			}
			
			info->set_length(length_in_ms/1000);				
			return 1;
		}					
	}
	return 0;
	}

	virtual bool open(reader * r,file_info * info,unsigned int full_open)
	{

	string_ansi_from_utf8 _f(info->get_file_path());
	char *f;
		f = strrchr(_f.get_ptr(), '/');
		if (f)
			++f;
		else f = (char*)_f.get_ptr();
		
		pReader = r;

		if (!full_open)
			if (!do_info(info, full_open))
			{
				return 0;
			}

		if (full_open)
		{
			int val = 0;
			int songLength;

			_global.pPlayer->Stop();

			OldPsyFile file;
		
			if (!file.Open(f))
			{
				return 0;
			}

			_global._pSong->filesize=file.FileSize();
			_global._pSong->New();
			_global._pSong->Load(&file);
			//file.Close(); Song::Load already closes the file. Should we not do it?

			songLength=CalcSongLength(_global._pSong)/1000;

			info->set_length(songLength);
			info->info_set_int("Filesize", _global._pSong->filesize);
			info->info_set_int("Bpm", _global._pSong->BeatsPerMin);
			info->info_set_int("LinePerBeat", _global._pSong->_ticksPerBeat);
			info->info_set_int("PatternsUsed", _global._pSong->GetNumPatternsUsed());

			info->meta_add("Author",_global._pSong->Author);
			info->meta_add("Title",_global._pSong->Name);
			info->meta_add("Comment",_global._pSong->Comment);


			strcpy(_global._pSong->fileName,f);
			_global._pSong->SetBPM(_global._pSong->BeatsPerMin, _global._pSong->_ticksPerBeat, _global.pConfig->_samplesPerSec);
			
			val = 64;
			_global.pPlayer->Work(_global.pPlayer, val); // Some plugins don't like to receive data without making first a 	// work call. (for example, Phantom)
			_global.pPlayer->Start(0,0);
		}
		
		return 1; 
		

	}

	virtual int run(audio_chunk * chunk)
	{
	float *float_buffer;
	Player* pPlayer = _global.pPlayer;
	int samprate = cfg_sampling_rate;
	int smp2 = _global.pConfig->_samplesPerSec/1000;
	int plug_stream_size = 1024;
	
		if (pPlayer->_playing)
		{

		float_buffer = pPlayer->Work(pPlayer, plug_stream_size);

		buffers.check_size(plug_stream_size * 2);
		Quantize(float_buffer,buffers,plug_stream_size*2);

		chunk->set_data(buffers,plug_stream_size,2,samprate);
		
		}else 
		{
			_global._pSong->New();
			return 0;
		}

		return 1;
	}
 
	virtual bool seek(double time_in_ms)
	{
	Song* pSong = _global._pSong;
	int time_left = (int)time_in_ms*1000;
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

		return 1;
	}

	virtual bool can_seek() { return pReader->can_seek();}
    virtual input::set_info_t set_info(reader *r,const file_info * info) { return SET_INFO_FAILURE; }
 
};

static const int srate_tab[]={11025,12000,22050,24000,44100,48000,88200,96000};
LPSTR	m_lpPluginsDir, m_lpVstDir;


int CALLBACK BrowseWnd(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{


TCHAR szDir[MAX_PATH];
	
	// see if we have already browsed for a folder
	// no at this time
	if ((LPSTR)lpData != NULL)
		strcpy(szDir,(LPSTR)lpData);
	else
		// yes !
		GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir);


         switch(msg) {
            case BFFM_INITIALIZED: 
				{

                  SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szDir);

               break;
	            }
            case BFFM_SELCHANGED: {
               // Set the status window to the currently selected path.
               if (SHGetPathFromIDList((LPITEMIDLIST) lParam ,szDir)) {
                  SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
               }
               break;
            }
            default:
               break;
         }
         return 0;

}

LPSTR BrowseForFolder(HWND hwnd, LPSTR m_searchDir)
{
	LPMALLOC pMalloc;

	if (SHGetMalloc(&pMalloc) == NOERROR)
	{
		char* szBuffer;					
		char pszBuffer[MAX_PATH];	
		LPITEMIDLIST   lpIDList;
		BROWSEINFO br;

	memset(&br, 0, sizeof(br));
	br.hwndOwner = hwnd;
	br.pidlRoot  = NULL;
	br.lpszTitle = "Choose a directory\0";
	br.ulFlags = BIF_RETURNONLYFSDIRS + BIF_DONTGOBELOWDOMAIN + BIF_STATUSTEXT;
	br.lpfn = BrowseWnd;
	br.lParam = (LPARAM)m_searchDir;

	lpIDList = SHBrowseForFolder(&br);

		if (lpIDList)
		{
			if (SHGetPathFromIDList(lpIDList,pszBuffer)) 
			{
				szBuffer = strdup(pszBuffer); 
				return szBuffer;
			}
			pMalloc->Free(lpIDList); 
			pMalloc->Release();
		}

	}

	return NULL;

}

static BOOL CALLBACK ConfigProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	HWND w;
	int c;
	char tmptext[_MAX_PATH];
	static HCURSOR hcOverCursor;
	static HBRUSH hBrushStatic;

	switch(msg)
	{
	case WM_INITDIALOG:
		{
		//	Sample Rate Combobox
		w=GetDlgItem(wnd,IDC_COMBO_SAMPLING_RATE);
		char valstr[10];
		for (c=0;c<4;c++)
		{
			sprintf(valstr,"%i",(int)(11025*powf(2.0f,(float)c)));
			SendMessage(w,CB_ADDSTRING,0,(long)valstr);
			sprintf(valstr,"%i",(int)(12000*powf(2.0f,(float)c)));
			SendMessage(w,CB_ADDSTRING,0,(long)valstr);
		}
		if (cfg_sampling_rate  < 11025) cfg_sampling_rate = 11025;

		_global.pConfig->_samplesPerSec = cfg_sampling_rate;

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

		SetDlgItemInt(wnd,IDC_COMBO_SAMPLING_RATE,cfg_sampling_rate,0);
		// Directories.
		if (!_global.pConfig->GetPluginDir()) {
			console::warning("Psycle doesn't seem to be installed in you computer, please go to http://psycle.pastnotecut.org/ then set the plugin paths directory");
			EnableWindow(GetDlgItem(wnd, IDC_BROWSE_VST), TRUE);
		}

		if (!_global.pConfig->GetVstDir()) {
			console::warning("Please set the VST plugins directory before using the plugin");
			EnableWindow(GetDlgItem(wnd, IDC_BROWSE_NATIVE), TRUE);
		}

		SetDlgItemText(wnd,IDC_EDIT_NATIVE,_global.pConfig->GetPluginDir());
		SetDlgItemText(wnd,IDC_EDIT_VST,_global.pConfig->GetVstDir());

		hBrushStatic = ::CreateSolidBrush(::GetSysColor (COLOR_BTNFACE));
		hcOverCursor = ::LoadCursor(NULL,(LPCTSTR)IDC_UPARROW); 

		}
		break;

		case WM_CTLCOLORSTATIC:
		if ((HWND)lp == ::GetDlgItem(wnd,IDC_URL) )
	       {
				::SetBkColor((HDC)wp, GetSysColor(COLOR_3DFACE));
				::SetTextColor((HDC)wp, RGB(0,0,255));
	 
	         return (LRESULT) hBrushStatic;
	       }
	       else
	         return (LRESULT) NULL;


	case WM_MOUSEMOVE:
		{

			POINT pnt;
			::GetCursorPos(&pnt);

			RECT rctURL;
			::GetWindowRect( ::GetDlgItem(wnd,IDC_URL), &rctURL);

			if (::PtInRect(&rctURL,pnt) ) {
				::SetCursor(hcOverCursor);
	         }

			

		} break;

	case WM_LBUTTONUP:
		{

		POINT pnt;
			::GetCursorPos(&pnt);

			RECT rctURL;
			::GetWindowRect( ::GetDlgItem(wnd,IDC_URL), &rctURL);

			TCHAR Url[200];
			bool bUrl = false;
			if (::PtInRect(&rctURL,pnt)) {
				::GetWindowText(::GetDlgItem(wnd,IDC_URL),Url,sizeof(Url));
				bUrl = true;
			}   

			if (bUrl) {
				ShellExecute(0 ,"open", "http://psycle.pastnotecut.org/", NULL, NULL, SW_MAXIMIZE);
			}

		} break;

	case WM_COMMAND:
		switch(wp)
		{
		case IDC_APPLY:
			if (_global.pPlayer->_playing ) {
				int val = 64;
				//_global.pPlayer->Stop();
				_global._pSong->New();
				//_global.pPlayer->Work(_global.pPlayer, val);
				//_global.pPlayer->Start(0,0);
			}
		
			c = SendDlgItemMessage(wnd,IDC_COMBO_SAMPLING_RATE,CB_GETCURSEL,0,0);
			if ( (c % 2) == 0) _global.pConfig->_samplesPerSec = (int)(11025*powf(2.0f,(float)(c/2)));
			else _global.pConfig->_samplesPerSec = (int)(12000*powf(2.0f,(float)(c/2)));
			
			GetDlgItemText(wnd,IDC_EDIT_NATIVE,tmptext,_MAX_PATH);
			_global.pConfig->SetPluginDir(tmptext);
			GetDlgItemText(wnd,IDC_EDIT_VST,tmptext,_MAX_PATH);
			_global.pConfig->SetVstDir(tmptext);

			break;

		case (CBN_SELCHANGE<<16)|IDC_COMBO_SAMPLING_RATE:
				cfg_sampling_rate = srate_tab[SendMessage((HWND)lp,CB_GETCURSEL,0,0)];
				break;
				
		case IDC_BROWSE_NATIVE:
			{
				m_lpPluginsDir = BrowseForFolder(wnd, _global.pConfig->GetPluginDir());
				if (m_lpPluginsDir != NULL) SetDlgItemText(wnd, IDC_EDIT_NATIVE, m_lpPluginsDir);
				break;
			}
		case IDC_BROWSE_VST:
			{
				m_lpVstDir = BrowseForFolder(wnd, _global.pConfig->GetVstDir());
				if (m_lpVstDir != NULL) SetDlgItemText(wnd, IDC_EDIT_VST, m_lpVstDir);
				break;
			}
		} break;
	}
	return 0;
}


class config_input_psycle : public config
{
public:
	virtual HWND create(HWND parent)
	{
		return CreateDialog(service_factory_base::get_my_instance(),MAKEINTRESOURCE(IDD_CONFIG),parent,ConfigProc);
	}
	virtual const char * get_name() {return "Psycle Decoder";}
	virtual const char * get_parent_name() {return "Input";}
};

static service_factory_t<input,input_psycle> foo;
static service_factory_t<config,config_input_psycle> foo2;
DECLARE_COMPONENT_VERSION("Psycle Module Decoder","1.0","Psycle Module Decoder for foobar2000\nDeveloped by Psycledelics 2001-2003");