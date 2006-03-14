/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */
// DirectSound stuff based on buzz code

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

#include "DirectSound.h"
#include "resource.h"
#include "DSoundConfig.h"
#include <process.h>

#define BYTES_PER_SAMPLE 4
namespace model {
	namespace audiodriver {

//const GUID configuration::DirectSound::GUID_NULL = {0,0,0,{0,0,0,0,0,0,0,0}};
configuration::DirectSound::DeviceList configuration::DirectSound::m_DeviceLists;
boost::once_flag configuration::DirectSound::m_Once = BOOST_ONCE_INIT;

string DirectSound::m_DriverName = _T("DirectSound Output");
string DirectSound::m_DriverDescription = _T("DirectSound Driver");

boost::mutex DirectSound::m_Mutex;

DirectSound::DirectSound(::model::iController *pController ) : m_Config(*this)
{
	m_pController = pController;
	m_IsInitialized = false;
	_running = false;
	_playing = false;
	_timerActive = false;
	_pDs = NULL;
	_pBuffer = NULL;
	_pCallback = NULL;
}


DirectSound::~DirectSound()
{
	Reset();
}

void DirectSound::Error(TCHAR const *msg) const
{
	MessageBox(NULL,msg, _T("DirectX Driver"), MB_OK);
}

const bool DirectSound::Start()
{
	boost::mutex::scoped_lock _lock(m_Mutex);

	//CComCritSecLock<CComCriticalSection> lock(_lock,true);
	if (_running)
	{
		return true;
	}

	if (_pCallback == NULL)
	{
		return false;
	}

	LPGUID _pDsGuid = NULL;
	GUID _DsGuid = m_Config.DsGuid;

	if(_DsGuid != GUID_NULL)
	{
		_pDsGuid = &_DsGuid;
	}

	if (FAILED(DirectSoundCreate(_pDsGuid, &_pDs, NULL)))
	{
		Error(SF::CResourceString(IDS_ERR_MSG0028));
		return false;
	}

	if (m_Config.IsExclusive)
	{
		if (FAILED(_pDs->SetCooperativeLevel(_hwnd, DSSCL_WRITEPRIMARY)))
		{
		// Don't report this, since we may have simply have lost focus
		//
		//	Error("Failed to set DirectSound cooperative level");
			_pDs->Release();
			_pDs = NULL;
			return false;
		}
	}
	else
	{
		if (FAILED(_pDs->SetCooperativeLevel(_hwnd, DSSCL_PRIORITY)))
		{
			Error(SF::CResourceString(IDS_ERR_MSG0029));
			_pDs->Release();
			_pDs = NULL;
			return false;
		}
	}

	_dsBufferSize = m_Config.IsExclusive ? 0 : m_Config.BufferSize * m_Config.NumBuffers;

	DSBCAPS caps;
	DSBUFFERDESC desc;
	WAVEFORMATEX format;

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.wBitsPerSample = 16;//_bitDepth;
	format.nSamplesPerSec = m_Config.SamplesPerSec;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
	desc.dwFlags |= m_Config.IsExclusive ? (DSBCAPS_PRIMARYBUFFER) : (DSBCAPS_GLOBALFOCUS);
	desc.dwBufferBytes = _dsBufferSize; 
	desc.dwReserved = 0;
	desc.lpwfxFormat = m_Config.IsExclusive ? NULL : &format;
	desc.guid3DAlgorithm = GUID_NULL;
	
	if (FAILED(_pDs->CreateSoundBuffer(&desc, &_pBuffer, NULL)))
	{
		Error(SF::CResourceString(IDS_ERR_MSG0030));
		_pDs->Release();
		_pDs = NULL;
		return false;
	}

	if (m_Config.IsExclusive)
	{
		_pBuffer->Stop();
		if (FAILED(_pBuffer->SetFormat(&format)))
		{
			Error(SF::CResourceString(IDS_ERR_MSG0031));
			_pBuffer->Release();
			_pBuffer = NULL;
			_pDs->Release();
			_pDs = NULL;
			return false;
		}
		caps.dwSize = sizeof(caps);
		if (FAILED(_pBuffer->GetCaps(&caps)))
		{
			Error(SF::CResourceString(IDS_ERR_MSG0032));
			_pBuffer->Release();
			_pBuffer = NULL;
			_pDs->Release();
			_pDs = NULL;
			return false;
		}
		_dsBufferSize = caps.dwBufferBytes;
		m_Config.Save(m_DriverName.c_str());
	}

	_lowMark = 0;
	_highMark = m_Config.BufferSize;
	if (_highMark >= _dsBufferSize)
	{
		_highMark = _dsBufferSize - 1;
	}
	_currentOffset = 0;
	_buffersToDo = m_Config.NumBuffers;

//	_event.Reset();
	_timerActive = true;

	m_Handle = reinterpret_cast<HANDLE>(_beginthread(&PollerThread, 0, this));

	_running = true;
	return true;
}

const bool DirectSound::Stop()
{
	boost::mutex::scoped_lock _lock(m_Mutex);	

	if (!_running)
	{
		return true;
	}
	_running = false;
	_timerActive = false;

//	CComCritSecLock<CComCriticalSection> event(_event,true);
	::WaitForSingleObject(m_Handle,INFINITE);

	// Once we get here, the PollerThread should have stopped
	//
	if (_playing)
	{
		_pBuffer->Stop();
		_playing = false;
	}

	// リソースの解放
	_pBuffer->Release();
	_pBuffer = NULL;

	_pDs->Release();
	_pDs = NULL;

	return true;
}

void DirectSound::PollerThread(void *pDirectSound)
{
	DirectSound* pThis = (DirectSound*)pDirectSound;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	while (pThis->_timerActive)
	{
		pThis->DoBlocks();
		Sleep(10);// TODO Waitは10msでいいのか？
	}
//	_event.Set();
	_endthread();
}

void DirectSound::DoBlocks()
{
	int pos;
	HRESULT hr;
	bool playing = _playing;

	while(true)
	{
		while(true)
		{
			hr = _pBuffer->GetCurrentPosition((DWORD*)&pos, NULL);
			if (FAILED(hr))
			{
				if (hr == DSERR_BUFFERLOST)
				{
					playing = false;
					if (FAILED(_pBuffer->Restore()))
					{
						// Don't inform about this error, because it will
						// appear each time the Psycle window loses focus in exclusive mode
						//
						return;
					}
					continue;
				}
				else
				{
					Error(SF::CResourceString(IDS_ERR_MSG0033));
					return;
				}
			}
			break;
		}

		if (_highMark < _lowMark)
		{
			if ((pos > _lowMark) || (pos < _highMark))
			{
				return;
			}
		}
		else if ((pos > _lowMark) && (pos < _highMark))
		{
			return;
		}

		int* pBlock1;
		int blockSize1;
		int* pBlock2;
		int blockSize2;

		int currentOffset = _currentOffset;
		while (_buffersToDo != 0)
		{
			while(true)
			{
				hr = _pBuffer->Lock((DWORD)currentOffset, (DWORD)m_Config.BufferSize,
									(void**)&pBlock1, (DWORD*)&blockSize1,
									(void**)&pBlock2, (DWORD*)&blockSize2,
									0);
				if (FAILED(hr))
				{
					if (hr == DSERR_BUFFERLOST)
					{
						playing = false;
						if (FAILED(_pBuffer->Restore()))
						{
							return;
						}
						continue;
					}
					else
					{
						Error(SF::CResourceString(IDS_ERR_MSG0034));
						return;
					}
				}
				break;
			}
		
			int blockSize = blockSize1 / BYTES_PER_SAMPLE;
			int* pBlock = pBlock1;
			while(blockSize > 0)
			{
				int n = blockSize;
				float *pFloatBlock = _pCallback(_callbackContext, n);
				if (m_Config.IsDither)
				{
					QuantizeWithDither(pFloatBlock, pBlock, n);
				}
				else
				{
					Quantize(pFloatBlock, pBlock, n);
				}
				pBlock += n;
				blockSize -= n;
			}
				
			blockSize = blockSize2 / BYTES_PER_SAMPLE;
			pBlock = pBlock2;
			while(blockSize > 0)
			{
				int n = blockSize;
				float *pFloatBlock = _pCallback(_callbackContext, n);
				if (m_Config.IsDither)
				{
					QuantizeWithDither(pFloatBlock, pBlock, n);
				}
				else
				{
					Quantize(pFloatBlock, pBlock, n);
				}
				pBlock += n;
				blockSize -= n;
			}
	
			_pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);
			
			_currentOffset += m_Config.BufferSize;

			if (_currentOffset >= _dsBufferSize)
			{
				_currentOffset -= _dsBufferSize;
			}

			_lowMark += m_Config.BufferSize;

			if (_lowMark >= _dsBufferSize)
			{
				_lowMark -= _dsBufferSize;
			}

			_highMark += m_Config.BufferSize;
			if (_highMark >= _dsBufferSize)
			{
				_highMark -= _dsBufferSize;
			}

			_buffersToDo--;
		} // while (_buffersToDo != 0)

		_buffersToDo = 1;

		if (!playing)
		{
			_playing = true;
			_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
		}

	} // while (true)
}

void DirectSound::Initialize(
	const HWND hwnd,
	const AUDIODRIVERWORKFN pCallback,
	const void* context)
{
	boost::mutex::scoped_lock _lock(m_Mutex);

	_callbackContext = const_cast<void *>(context);
	_pCallback = pCallback;
	_running = false;
	_hwnd = hwnd;
	m_Config.Load(DriverName);
	m_IsInitialized = true;
}

void DirectSound::Reset()
{
	Stop();
}

/** 設定ダイアログを呼びだし、ユーザに設定させる*/
void configuration::DirectSound::Configure()
{

	Load(m_DirectSound.DriverName.c_str());
	create_modal_dlg<view::dsound_config_dlg,model::audiodriver::configuration::DirectSound>(*this,null_wnd).wait();

}


void configuration::DirectSound::Apply(const configuration::DirectSound& backup)
{

	IsConfigured = true;

	if (m_DirectSound.IsInitialized)
	{
		m_DirectSound.Stop();
	}

//	m_NumBuffers = dlg.m_numBuffers;
//	m_BufferSize = dlg.m_bufferSize;
//	m_DeviceIndex = dlg.m_deviceIndex;
//	m_pDsGuid = dlg.m_pDeviceGuid;		// ｿ!!ｿｿ!ｿ!ｿ!ｿｿ!ｿ!

//	if (m_pDsGuid != NULL)				// This pointer is temporary!!!
//	{
//		memcpy(&m_DsGuid, &dlg.m_deviceGuid, sizeof(GUID));
//		m_pDsGuid = &m_DsGuid;
//	}

//	m_Dither = *(bool*)(&dlg.m_dither);
//	m_Exclusive = *(bool*)(&dlg.m_exclusive);
//	m_SamplesPerSec = dlg.m_sampleRate;

	if (m_DirectSound.IsInitialized)
	{
		if (m_DirectSound.Start())
		{
			Save(m_DirectSound.DriverName);
		}
		else
		{
			MessageBeep(MB_OK);
			Cancel(backup);
			m_DirectSound.Start();
		}
	}
}

const int DirectSound::GetPlayPos() const
{
	// 事前条件
	ATLASSERT(_running);

	if (!_running)
	{
		return 0;
	}

	int playPos;
	if (FAILED(_pBuffer->GetCurrentPosition((DWORD*)&playPos, NULL)))
	{
		Error(SF::CResourceString(IDS_ERR_MSG0033));
		return 0;
	}
	return playPos;
}

const int DirectSound::GetWritePos() const
{
	// 事前条件
	ATLASSERT(_running);

	if (!_running)
	{
		return 0;
	}

	int writePos;

	if (FAILED(_pBuffer->GetCurrentPosition(NULL, (DWORD*)&writePos)))
	{
		Error(SF::CResourceString(IDS_ERR_MSG0033));
		return 0;
	}

	return writePos;
}

const bool DirectSound::Enable(const bool e)
{
	return e ? Start() : Stop();
}

/** サウンドデバイスを列挙する */
BOOL CALLBACK
configuration::DirectSound::EnumDSoundDevices(
	LPGUID lpGUID,
	const TCHAR* psDesc,
	const TCHAR* psDrvName,
	void* context)
{

	GUID _guid;

	if(lpGUID != NULL){
		memcpy(&_guid, lpGUID, sizeof(GUID));
	} else {
		memset(&_guid,0,sizeof(GUID));
	}

	// デバイスのCapsを得る 
	CComPtr<IDirectSound8> _pds;
	HRESULT _hr = E_FAIL;

	if(lpGUID != NULL){
		_hr = DirectSoundCreate8(&_guid,&_pds,NULL);
	} else {
		_guid = GUID_NULL;
		_hr = DirectSoundCreate8(NULL,&_pds,NULL);
	}

	if(SUCCEEDED(_hr)){
		DSCAPS _dscaps;

		ZeroMemory(&_dscaps,sizeof(_dscaps));
		_dscaps.dwSize = sizeof(DSCAPS);

		if(SUCCEEDED(_pds->GetCaps(&_dscaps))){
			model::audiodriver::configuration::DirectSound::DeviceInfo& _info
				= m_DeviceLists[string(psDesc)];

			_info.Name = string(psDesc);
			_info.Capabilities = _dscaps;
			_info.Guid = _guid;
		}
	};


	return TRUE;
}

void configuration::DirectSound::Load(const string& fileName)
{
	bool configured = false;

	// Default configuration
	//
	m_NumBuffers = 4;
	m_BufferSize = 4096;
	m_DsGuid = GUID_NULL;
	m_Dither = false;
	m_SamplesPerSec = 44100;
	m_BitDepth = 16;
	m_ChannelMode = 3;
	m_Exclusive = false;


	string _file_path = fileName;
	_file_path += L".xml";
	_file_path = m_DirectSound.Controller.CurrentDirectory() + _file_path;

	std::wifstream _wifs(CT2A(_file_path.c_str()));
	if(!_wifs.is_open())
	{
		return;
	}

	boost::archive::xml_wiarchive _archive(_wifs);
	_archive >> boost::serialization::make_nvp("DirectSound",*this);
	_wifs.close();

	// XMLファイルより、設定を読み込む
	
//	MSXML::IXMLDOMDocumentPtr _pDoc;
//	_pDoc.CreateInstance(CLSID_DOMDocument);
//	_pDoc->async = VARIANT_FALSE;
//	
////	_pDoc->put_charset(_bstr_t(L"utf-8"));
//	VARIANT_BOOL _result = VARIANT_FALSE;
//	
//	string _file_path = fileName;
//	_file_path += _T(".xml");
//	_file_path = m_DirectSound.Controller.CurrentDirectory() + _file_path;
//
//	try {
//		if(_pDoc->load(CComVariant(_file_path.data())) != VARIANT_TRUE){
//			return;
//		}
//	} catch (_com_error& e) {
//		return;
//	}
//	
//	MSXML::IXMLDOMElementPtr _pElemRoot = _pDoc->documentElement;
//	if(NULL == _pElemRoot)
//    {
//        return;
//    }
//
//	configured = true;
//
//	MSXML::IXMLDOMNodePtr _pNode = _pElemRoot->selectSingleNode(_bstr_t(_T("DirectSound")));
//	if(_pNode != NULL)
//	{
//		try {
//			configured &= SF::GetNodeData(_pNode,_T("NumBuffers"),m_NumBuffers);
//			configured &= SF::GetNodeData(_pNode,_T("BufferSize"),m_BufferSize);
//			configured &= SF::GetNodeData(_pNode,_T("DeviceGuid"),m_DsGuid);
//			configured &= SF::GetNodeData(_pNode,_T("DeviceDescription"),m_DeviceDescription);
//			configured &= SF::GetNodeData(_pNode,_T("Dither"),m_Dither);
//			configured &= SF::GetNodeData(_pNode,_T("Exclusive"),m_Exclusive);
//			configured &= SF::GetNodeData(_pNode,_T("SamplesPerSec"),m_SamplesPerSec);
//
//		} catch (...) {
//			configured = false;
//		}
//	};
//	
	m_IsConfigured = configured;
}

void configuration::DirectSound::Save(const string& fileName)
{
	if(!IsConfigured)
		return;

	// Backup をとる
	string _file_path = fileName;
	_file_path += _T(".xml");
	_file_path = m_DirectSound.Controller.CurrentDirectory() + _file_path;

	std::wofstream wofs(CT2A(_file_path.c_str()),std::ios_base::out | std::ios_base::trunc);
	boost::archive::xml_woarchive oa(wofs);
	oa << boost::serialization::make_nvp("DirectSound", *this);

	//{
	//	MSXML::IXMLDOMDocumentPtr _pdoc_backup;
	//	_pdoc_backup.CreateInstance(CLSID_DOMDocument);
	//	_pdoc_backup->async = VARIANT_FALSE;
	////	_pDoc->put_charset(_bstr_t(L"utf-8"));

	//	VARIANT_BOOL _result = VARIANT_FALSE;
	//
	//	if(_pdoc_backup->load(CComVariant(_file_path.data())) == VARIANT_TRUE)
	//	{
	//		string _backup_path = _file_path + _T(".bak");
	//		_pdoc_backup->save(_backup_path.data());
	//	}
	//}

	//MSXML::IXMLDOMDocumentPtr _pdoc;
	//_pdoc.CreateInstance(CLSID_DOMDocument);
	//_pdoc->appendChild(
 //     _pdoc->createProcessingInstruction(
 //       _T("xml"), _T("version='1.0' encoding='utf-8'")));

	//MSXML::IXMLDOMNodePtr _node = _pdoc->createElement(_T("DirectSound"));
	//_pdoc->appendChild(_node);

	//SF::SetNodeData(_node,_T("NumBuffers"),m_NumBuffers);
	//SF::SetNodeData(_node,_T("BufferSize"),m_BufferSize);
	//SF::SetNodeData(_node,_T("DeviceGuid"),m_DsGuid);
	//SF::SetNodeData(_node,_T("DeviceDescription"),m_DeviceDescription);
	//SF::SetNodeData(_node,_T("Dither"),m_Dither);
	//SF::SetNodeData(_node,_T("Exclusive"),m_Exclusive);
	//SF::SetNodeData(_node,_T("SamplesPerSec"),m_SamplesPerSec);

	//if(_pdoc->save(_file_path.data()) == VARIANT_FALSE)
	//{
	//	m_DirectSound.Error(SF::CResourceString(IDS_ERR_MSG0035));
	//}
}
}
}
