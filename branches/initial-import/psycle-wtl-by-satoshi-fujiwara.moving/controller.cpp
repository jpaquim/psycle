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

//#include "Song.h"
//#include "Instrument.h"
//#include "Player.h"
//#include "Dsp.h"
//#include "configuration.h"
//#include "MidiInput.h"

#include "controller.h"

Controller::Controller()
{
		m_bOutputActive = false;
	//	m_pResampler = new Cubic;
	//	m_pResampler->SetQuality(RESAMPLE_LINEAR);
	//	m_pSong = new ::Song(this);
	//	m_pPlayer = new ::Player();
	//	m_pConfig = new ::configuration::configuration(this);
		
		TCHAR _dir[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH,_dir);
		m_CurrentDirectory = _dir;

#ifndef _CYRIX_PROCESSOR_

	ULONG cpuHz;
	__asm	rdtsc				// Read time stamp to EAX
	__asm	mov	cpuHz, eax

	Sleep(1000);

	__asm	rdtsc
	__asm	sub		eax,cpuHz	// Find the difference
	__asm	mov		cpuHz, eax

	m_CpuHz = cpuHz;
#else
	m_CpuHz = 1;
#endif // _CYRIX_PROCESSOR_

	// TODO: ドライバー列挙コードの再構築を行う

	_numOutputDrivers = 4;// decrease it if no asio
//	_numOutputDrivers = 1;// decrease it if no asio
//	_outputDriverIndex = 0;

	_ppOutputDrivers = new iAudioDriver*[_numOutputDrivers];
	_ppOutputDrivers[0] = new AudioDriver(this);
	_ppOutputDrivers[1] = new WaveOut(this);
	_ppOutputDrivers[2] = new DirectSound(this);
	_ppOutputDrivers[3] = new ASIOInterface(this);
	
	if (((ASIOInterface*)(_ppOutputDrivers[3]))->drivercount <= 0)
	{
		_numOutputDrivers--;
		delete _ppOutputDrivers[3];
	}
	
	m_pOutputDriver = _ppOutputDrivers[_outputDriverIndex];
	_pMidiInput = new CMidiInput;	// MIDI IMPLEMENTATION

}

Controller::~Controller()
{
	delete m_pSong;
	delete m_pResampler;
	delete m_pSong;
	delete m_pPlayer;
	delete m_pConfig;

	if (_ppOutputDrivers != NULL)
	{
		for (int i=0; i<_numOutputDrivers; i++)
		{
			if (_ppOutputDrivers[i] != NULL)
			{
				delete _ppOutputDrivers[i];	//Deletes each Driver
			}
		}
		delete[] _ppOutputDrivers;	// And deletes the dinamically created table of pointers.
	}

	// MIDI IMPLEMENTATION
	if (_pMidiInput != NULL)
	{
		delete _pMidiInput;
	}

}


void Controller::AddBpm(const int addValue) 
{
	int _cur_bpm = 0;


	if (Player()->IsPlaying() )
	{
		if ( addValue == 0 ){
			_cur_bpm = Player()->Bpm();
		} else {
			Player()->Bpm(Player()->Bpm() + addValue);
			_cur_bpm = Song()->AddBpm(Player()->Bpm() - Song()->BeatsPerMin());
		}
	}
	else
	{
		if ( addValue != 0 ){
			Song()->AddBpm(addValue);

		}
		_cur_bpm = Song()->BeatsPerMin();
	}

	MainFrame()->ChangeBpm(_cur_bpm);
}

void Controller::AddTpb(const int addValue)
{
	m_pSong->AddTpb(addValue);
	m_pPlayer->Tpb(m_pSong->TicksPerBeat());
	
	int _tpb = 0;

	if(m_pController->Player()->IsPlaying()){
		_tpb = m_pPlayer->Tpb();
	} else {
		_tpb = m_pSong->TicksPerBeat();
	}

	m_pMainFrame->ChangeTpb(_tpb);
}

/// ViewからNewFileイベントを受け、処理する
void Controller::NewFile()
{
	SF::CResourceString _newFile(IDS_MSG0023);
	
	if(m_pSong->IsSaved() || !m_pConfig->IsFileSaveRemind() || !m_pMainFrame->CheckUnsavedSong(_newFile))
	{
		InitializeBeforeLoading();

		m_pSong->New();

		IsOutputActive(true);

		if (!m_pConfig->OutputDriver().Enable(true))
		{
			IsOutputActive(false);
		}
		else
		{
			// MIDI IMPLEMENTATION
			m_pConfig->MidiInput().Open();
		}
		m_pSong->SeqBus(0);
		// GUIにイベントを送出
		m_pMainFrame->NewSong();

	}

}

void Controller::SaveFile(const bool bFileSaveAs)
{

	string ifile(m_pSong->FileName());
	
	//CString if2 = ifile.SpanExcluding(_T("\\/:*\"<>|"));
	string delm = _T("\\/:*\"<>|");
	
	int index = std::string::npos;
	
	for(int i = 0;i < static_cast<int>(ifile.length());i++)
	{
		if(delm.find(ifile[i]) != std::string::npos)
		{
			index = i;		
			break;
		}
	}

	string if2;
	
	if(index != std::string::npos)
	{
		if2 = ifile.substr(0,index);
	} else {
		if2 = ifile;
	}
	
	TCHAR _file_name[MAX_PATH];
	::ZeroMemory(_file_name,MAX_PATH);
	_tcsncpy(_file_name,if2.data(),MAX_PATH);
	if(m_pMainFrame->SelectFile(_file_name))
	{
		// TODO SongのSaveメソッドを作成する
		m_pSong->Save(string(_file_name));
	};
}

void Controller::InitializeBeforeLoading()
{
	//KillUndo();
	//KillRedo();

	m_pMainFrame->InitializeBeforeLoading();

	m_pPlayer->Stop();

	Sleep(LOCK_LATENCY);
	
	//_outputActive = false;
	
	m_pConfig->OutputDriver().Enable(false);

	// MIDI IMPLEMENTATION
	m_pConfig->MidiInput().Close();

	Sleep(LOCK_LATENCY);

};

void Controller::ProcessAfterLoading()
{
	m_pSong->SetBPM(
		m_pSong->BeatsPerMin(),
		m_pSong->TicksPerBeat(),
		m_pConfig->OutputDriver()._samplesPerSec);

//	_outputActive = true;
	
	if (!m_pConfig->OutputDriver().Enable(true))
	{
		_outputActive = false;
	}
	else
	{
		// MIDI IMPLEMENTATION
		m_pConfig->MidiInput()->Open();
	}
};

HRESULT Run(ULONG hInstance,BSTR lpstrCmdLine,LONG nCmdShow)
{


}

