/** @file 
 *  @brief waveout stuff based on buzz code
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
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

#include "WaveOut.h"
#include "resource.h"
#include "WaveOutDialog.h"
#include "Registry.h"
//#include "configuration.h"
//#include "MidiInput.h"
#include <process.h>

#import "msxml.dll" named_guids

#define BYTES_PER_SAMPLE 4	// 2 * 16bits

AudioDriverInfo WaveOut::m_Info = { _T("Windows WaveOut MME") };
AudioDriverEvent WaveOut::m_Event;
ATL::CComAutoCriticalSection WaveOut::m_Lock;

WaveOut::WaveOut(iController *pController) : AudioDriver(pController)
{
	m_Initialized = false;
	m_Configured = false;
	m_bRunning = false;
	m_pCallback = NULL;
}


WaveOut::~WaveOut()
{
	if (m_Initialized) Reset();
}

void WaveOut::Error(TCHAR const *msg)
{
	MessageBox(NULL, msg, SF::CResourceString(IDS_MSG0012), MB_OK);
}

bool WaveOut::Start()
{
	CComCritSecLock<CComAutoCriticalSection> lock(m_Lock, TRUE);
	if (m_bRunning)
	{
		return true;
	}

	if (m_pCallback == NULL)
	{
		return false;
	}

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = 16;//_bitDepth;
	format.nSamplesPerSec = m_SamplesPerSec;
	format.cbSize = 0;
	format.nChannels = 2;

	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	if (waveOutOpen(&m_Handle, m_DeviceID, &format, NULL, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		Error(SF::CResourceString(IDS_ERR_MSG0052));
		return false;
	}

	m_CurrentBlock = 0;
	m_WritePos = 0;

	// allocate blocks
	for (CBlock *pBlock = m_Blocks; pBlock < m_Blocks + m_NumBlocks; pBlock++)
	{
		pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, m_BlockSize);
		pBlock->pData = (byte *)GlobalLock(pBlock->Handle);
	}

	// allocate block headers
	for (pBlock = m_Blocks; pBlock < m_Blocks + m_NumBlocks; pBlock++)
	{
		pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
		pBlock->pHeader = (WAVEHDR *)GlobalLock(pBlock->HeaderHandle);

		WAVEHDR *ph = pBlock->pHeader;
		ph->lpData = (char *)pBlock->pData;
		ph->dwBufferLength = m_BlockSize;
		ph->dwFlags = WHDR_DONE;
		ph->dwLoops = 0;

		pBlock->Prepared = false;
	}

	m_bStopPolling = false;
	m_Event.Reset();
	_beginthread(PollerThread, 0, this);
	m_bRunning = true;
	// TODO CMidiInput::Instance()->ReSync() を元に戻す。
	// CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
	return true;
}

bool WaveOut::Stop()
{
	CComCritSecLock<CComAutoCriticalSection> lock(m_Lock, TRUE);
	if (!m_bRunning)
	{
		return true;
	}

	m_bStopPolling = true;
	WaitForSingleObject((HANDLE)(m_Event),INFINITE);
//		CComCritSecLock<CCriticalSection> event(&m_Event, TRUE);

	if (waveOutReset(m_Handle) != MMSYSERR_NOERROR)
	{
		Error(SF::CResourceString(IDS_ERR_MSG0053));
		return false;
	}

	while (1)
	{
		bool alldone = true;

		for (CBlock *pBlock = m_Blocks; pBlock < m_Blocks + m_NumBlocks; pBlock++)
		{
			if ((pBlock->pHeader->dwFlags & WHDR_DONE) == 0)
			{
				alldone = false;
			}
		}
		if (alldone)
		{
			break;
		}
		Sleep(20);
	}
	for (CBlock *pBlock = m_Blocks; pBlock < m_Blocks + m_NumBlocks; pBlock++)
	{
		if (pBlock->Prepared)
		{
			if (waveOutUnprepareHeader(m_Handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				Error(SF::CResourceString(IDS_ERR_MSG0054));
			}
		}
	}
	if (waveOutClose(m_Handle) != MMSYSERR_NOERROR)
	{
		Error(SF::CResourceString(IDS_ERR_MSG0055));
		return false;
	}
	for (pBlock = m_Blocks; pBlock < m_Blocks + m_NumBlocks; pBlock++)
	{
		GlobalUnlock(pBlock->Handle);
		GlobalFree(pBlock->Handle);
		GlobalUnlock(pBlock->HeaderHandle);
		GlobalFree(pBlock->HeaderHandle);
	}
	m_bRunning = false;
	return true;
}

void
WaveOut::PollerThread(
	void *pWaveOut)
{
	WaveOut* pThis = (WaveOut*)pWaveOut;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	while (!pThis->m_bStopPolling)
	{
		pThis->DoBlocks();
		::Sleep(pThis->m_PollSleep);
	}
	m_Event.Set();
	_endthread();
}

void WaveOut::DoBlocks()
{
	CBlock *pb = m_Blocks + m_CurrentBlock;

	while(pb->pHeader->dwFlags & WHDR_DONE)
	{
		if (pb->Prepared)
		{
			if (waveOutUnprepareHeader(m_Handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				Error(SF::CResourceString(IDS_ERR_MSG0054));
			}
			pb->Prepared = false;
		}
		int *pOut = (int *)pb->pData;
		int bs = m_BlockSize / BYTES_PER_SAMPLE;

		do
		{
			int n = bs;
			float *pBuf = m_pCallback(m_CallbackContext, n);
			if (m_Dither)
			{
				QuantizeWithDither(pBuf, pOut, n);
			}
			else
			{
				Quantize(pBuf, pOut, n);
			}
			pOut += n;
			bs -= n;
		}
		while (bs > 0);

		m_WritePos += m_BlockSize/BYTES_PER_SAMPLE;

		pb->pHeader->dwFlags = 0;
		pb->pHeader->lpData = (char *)pb->pData;
		pb->pHeader->dwBufferLength = m_BlockSize;
		pb->pHeader->dwFlags = 0;
		pb->pHeader->dwLoops = 0;

		if (waveOutPrepareHeader(m_Handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			Error(SF::CResourceString(IDS_ERR_MSG0054));
		}
		pb->Prepared = true;

		if (waveOutWrite(m_Handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			Error(SF::CResourceString(IDS_ERR_MSG0056));
		}
		pb++;
		if (pb == m_Blocks + m_NumBlocks)
		{
			pb = m_Blocks;
		}
	}
	m_CurrentBlock = pb - m_Blocks;
}

void WaveOut::ReadConfig()
{
	bool configured = false;

	// Default configuration
	//
	m_SamplesPerSec=44100;
	m_DeviceID=0;
	m_NumBlocks = 7;
	m_BlockSize = 4096;
	m_PollSleep = 20;
	m_Dither = 0;
	_channelmode = 3;
	_bitDepth = 16;

	// XMLファイルより、設定を読み込む

	
	MSXML::IXMLDOMDocumentPtr _pDoc;
	_pDoc.CreateInstance(CLSID_DOMDocument);
	_pDoc->async = VARIANT_FALSE;
	
//	_pDoc->put_charset(_bstr_t(L"utf-8"));
	VARIANT_BOOL _result = VARIANT_FALSE;
	
	string _file_path = _T("WaveOut.xml");
	_file_path += m_pController->CurrentDirectory();

	try {
		if(_pDoc->load(CComVariant(_file_path.data())) != VARIANT_TRUE){
			return;
		}
	} catch (_com_error& e) {
		return;
	}
	
	MSXML::IXMLDOMElementPtr _pElemRoot = _pDoc->documentElement;
	if(NULL == _pElemRoot)
    {
        return;
    }
	
	MSXML::IXMLDOMNodePtr _pNode = _pElemRoot->selectSingleNode(_bstr_t(_T("WaveOut")));
	if(_pNode != NULL)
	{
		try {
			configured &= GetNodeData(_pNode,_T("NumBlocks"),m_NumBlocks);
			configured &= GetNodeData(_pNode,_T("BlockSize"),m_BlockSize);
			configured &= GetNodeData(_pNode,_T("DeviceID"),m_DeviceID);
			configured &= GetNodeData(_pNode,_T("PollSleep"),m_PollSleep);
			
			configured &= GetNodeData(_pNode,_T("Dither"),m_Dither);
			configured &= GetNodeData(_pNode,_T("SamplesPerSec"),m_SamplesPerSec);

		} catch (...) {
			configured = false;
		}
	};
	m_Configured = configured;
}

void WaveOut::WriteConfig()
{
	// Backup をとる
	{
		MSXML::IXMLDOMDocumentPtr _pdoc_backup;
		_pdoc_backup.CreateInstance(CLSID_DOMDocument);
		_pdoc_backup->async = VARIANT_FALSE;
	//	_pDoc->put_charset(_bstr_t(L"utf-8"));

		VARIANT_BOOL _result = VARIANT_FALSE;

		string _file_path = _T("WaveOut.xml");
		_file_path = m_pController->CurrentDirectory() + _file_path;
	
		if(_pdoc_backup->load(CComVariant(_file_path.data())) == VARIANT_TRUE)
		{
			string _backup_path = _T("WaveOut.xml.bak");
			_backup_path = m_pController->CurrentDirectory() + _backup_path;
			_pdoc_backup->save(_backup_path.data());
		}
	}

	MSXML::IXMLDOMDocumentPtr _pdoc;
	_pdoc.CreateInstance(CLSID_DOMDocument);
	_pdoc->appendChild(
      _pdoc->createProcessingInstruction(
        _T("xml"), _T("version='1.0' encoding='utf-8'")));

	MSXML::IXMLDOMNodePtr _node = _pdoc->createElement(_T("WaveOut"));
	_pdoc->appendChild(_node);

	SetNodeData(_node,_T("NumBlocks"),m_NumBlocks);
	SetNodeData(_node,_T("BlockSize"),m_BlockSize);
	SetNodeData(_node,_T("DeviceID"),m_DeviceID);
	SetNodeData(_node,_T("PollSleep"),m_PollSleep);
	SetNodeData(_node,_T("Dither"),m_Dither);
	SetNodeData(_node,_T("SamplesPerSec"),m_SamplesPerSec);

	string _file_path = _T("WaveOut.xml");
	_file_path = m_pController->CurrentDirectory() + _file_path;
	if(_pdoc->save(_file_path.data()) == VARIANT_FALSE){
		Error(SF::CResourceString(IDS_ERR_MSG0035));
	}
}


void WaveOut::Initialize(
	const HWND hwnd,
	const AUDIODRIVERWORKFN pCallback,
	const void * context)
{
	m_CallbackContext = const_cast<void *>(context);
	m_pCallback = pCallback;
	m_bRunning = false;
	ReadConfig();
	m_Initialized = true;
}

void WaveOut::Reset()
{
	if (m_bRunning) Stop();
}

void WaveOut::Configure()
{
	ReadConfig();

	CWaveOutDialog dlg;
	dlg.m_BufNum = m_NumBlocks;
	dlg.m_BufSize = m_BlockSize;
	dlg.m_Device = m_DeviceID;
	dlg.m_Dither = m_Dither;
	dlg.m_SampleRate = m_SamplesPerSec;

	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	
	int oldnb = m_NumBlocks;
	int oldbs = m_BlockSize;
	int olddid = m_DeviceID;
	int olddither = m_Dither;
	int oldsps = m_SamplesPerSec;

	if (m_Initialized)
	{
		Stop();
	}

	m_NumBlocks = dlg.m_BufNum;
	m_BlockSize = dlg.m_BufSize;
	m_DeviceID = dlg.m_Device;
	m_Dither = dlg.m_Dither;
	m_SamplesPerSec = dlg.m_SampleRate;

	m_Configured = true;

	if (m_Initialized)
	{
		if (Start())
		{
			WriteConfig();
		}
		else
		{
			m_NumBlocks = oldnb;
			m_BlockSize = oldbs;
			m_DeviceID = olddid;
			m_Dither = olddither;
			m_SamplesPerSec = oldsps;

			Start();
		}
	}
	else
	{
		WriteConfig();
	}
	
}

const int WaveOut::PlayPos()
{
	if (!m_bRunning)
	{
		return 0;
	}
	MMTIME time;
	time.wType = TIME_SAMPLES;

	if (waveOutGetPosition(m_Handle, &time, sizeof(MMTIME)) != MMSYSERR_NOERROR)
	{
		Error(SF::CResourceString(IDS_ERR_MSG0057));
	}
	if (time.wType != TIME_SAMPLES)
	{
		Error(SF::CResourceString(IDS_ERR_MSG0058));
	}
	return (time.u.sample & ((1 << 23) - 1));
}

const int WaveOut::WritePos()
{
	if (!m_bRunning)
	{
		return 0;
	}
	return (m_WritePos & ((1 << 23) - 1));
}

const bool WaveOut::Enable(const bool e)
{
	return e ? Start() : Stop();
}

