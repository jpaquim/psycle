// DirectSound stuff based on buzz code

#include "stdafx.h"
#include "DirectSound.h"
#include "resource.h"
#include "DSoundConfig.h"
#include "Registry.h"
#include "Configuration.h"
#include <process.h>

#define BYTES_PER_SAMPLE 4

AudioDriverInfo DirectSound::_info = { "DirectSound Output" };
AudioDriverEvent DirectSound::_event;
CCriticalSection DirectSound::_lock;

DirectSound::DirectSound()
{
	_initialized = false;
	_configured = false;
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

void DirectSound::Error(char const *msg)
{
	MessageBox(NULL,msg, "DirectX driver", MB_OK);
}

bool DirectSound::Start()
{
	CSingleLock lock(&_lock, TRUE);
	if (_running)
	{
		return true;
	}
	if (_pCallback == NULL)
	{
		return false;
	}

	if (FAILED(DirectSoundCreate(_pDsGuid, &_pDs, NULL)))
	{
		Error("Failed to create DirectSound object");
		return false;
	}

	if (_exclusive)
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
			Error("Failed to set DirectSound cooperative level");
			_pDs->Release();
			_pDs = NULL;
			return false;
		}
	}

	_dsBufferSize = _exclusive ? 0 : _bufferSize*_numBuffers;

	DSBCAPS caps;
	DSBUFFERDESC desc;
	WAVEFORMATEX format;

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.wBitsPerSample = _bitDepth;
	format.nSamplesPerSec = _samplesPerSec;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
	desc.dwFlags |= _exclusive ? (DSBCAPS_PRIMARYBUFFER) : (DSBCAPS_GLOBALFOCUS);
	desc.dwBufferBytes = _dsBufferSize; 
	desc.dwReserved = 0;
	desc.lpwfxFormat = _exclusive ? NULL : &format;
	desc.guid3DAlgorithm = GUID_NULL;
	
	if (FAILED(_pDs->CreateSoundBuffer(&desc, &_pBuffer, NULL)))
	{
		Error("Failed to create DirectSound Buffer(s)");
		_pDs->Release();
		_pDs = NULL;
		return false;
	}

	if (_exclusive)
	{
		_pBuffer->Stop();
		if (FAILED(_pBuffer->SetFormat(&format)))
		{
			Error("Failed to set DirectSound Buffer format");
			_pBuffer->Release();
			_pBuffer = NULL;
			_pDs->Release();
			_pDs = NULL;
			return false;
		}
		caps.dwSize = sizeof(caps);
		if (FAILED(_pBuffer->GetCaps(&caps)))
		{
			Error("Failed to get DirectSound Buffer capabilities");
			_pBuffer->Release();
			_pBuffer = NULL;
			_pDs->Release();
			_pDs = NULL;
			return false;
		}
		_dsBufferSize = caps.dwBufferBytes;
		WriteConfig();
	}

	_lowMark = 0;
	_highMark = _bufferSize;
	if (_highMark >= _dsBufferSize)
	{
		_highMark = _dsBufferSize-1;
	}
	_currentOffset = 0;
	_buffersToDo = _numBuffers;
	_event.ResetEvent();
	_timerActive = true;
	_beginthread(PollerThread, 0, this);

	_running = true;
	return true;
}

bool DirectSound::Stop()
{
	CSingleLock lock(&_lock, TRUE);

	if (!_running)
	{
		return true;
	}
	_running = false;
	_timerActive = false;
	CSingleLock event(&_event, TRUE);
	// Once we get here, the PollerThread should have stopped
	//
	if (_playing)
	{
		_pBuffer->Stop();
		_playing = false;
	}
	_pBuffer->Release();
	_pBuffer = NULL;
	_pDs->Release();
	_pDs = NULL;

	return true;
}

void
DirectSound::PollerThread(
	void *pDirectSound)
{
	DirectSound* pThis = (DirectSound*)pDirectSound;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	while (pThis->_timerActive)
	{
		pThis->DoBlocks();
		Sleep(10);
	}
	_event.SetEvent();
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
					Error("DirectSoundBuffer::GetCurrentPosition failed");
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
				hr = _pBuffer->Lock((DWORD)currentOffset, (DWORD)_bufferSize,
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
						Error("Failed to lock DirectSoundBuffer");
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
				if (_dither)
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
				if (_dither)
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
			_currentOffset += _bufferSize;
			if (_currentOffset >= _dsBufferSize)
			{
				_currentOffset -= _dsBufferSize;
			}
			_lowMark += _bufferSize;
			if (_lowMark >= _dsBufferSize)
			{
				_lowMark -= _dsBufferSize;
			}
			_highMark += _bufferSize;
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

void DirectSound::ReadConfig()
{
	bool configured;
	DWORD type;
	DWORD numData;
	Registry reg;

	// Default configuration
	//
	_numBuffers = 4;
	_bufferSize = 4096;
	_deviceIndex = 0;
	_pDsGuid = NULL;
	_dither = false;
	_samplesPerSec = 44100;
	_bitDepth = 16;
	_channelmode = 3;
	_exclusive = false;

	if (reg.OpenRootKey(HKEY_CURRENT_USER, CONFIG_ROOT_KEY) != ERROR_SUCCESS)
	{
		return;
	}
	if (reg.OpenKey("DirectSound") != ERROR_SUCCESS)
	{
		return;
	}
	configured = true;
	numData = sizeof(_numBuffers);
	configured &= (reg.QueryValue("NumBuffers", &type, (BYTE*)&_numBuffers, &numData) == ERROR_SUCCESS);
	numData = sizeof(_bufferSize);
	configured &= (reg.QueryValue("BufferSize", &type, (BYTE*)&_bufferSize, &numData) == ERROR_SUCCESS);
	numData = sizeof(_deviceIndex);
	configured &= (reg.QueryValue("DeviceIndex", &type, (BYTE*)&_deviceIndex, &numData) == ERROR_SUCCESS);
	numData = sizeof(GUID);
	if (reg.QueryValue("DeviceGuid", &type, (BYTE*)&_dsGuid, &numData) == ERROR_SUCCESS)
	{
		_pDsGuid = &_dsGuid;
	}
	numData = sizeof(_dither);
	configured &= (reg.QueryValue("Dither", &type, (BYTE*)&_dither, &numData) == ERROR_SUCCESS);
	numData = sizeof(_exclusive);
	configured &= (reg.QueryValue("Exclusive", &type, (BYTE*)&_exclusive, &numData) == ERROR_SUCCESS);
	numData = sizeof(_samplesPerSec);
	configured &= (reg.QueryValue("SamplesPerSec", &type, (BYTE*)&_samplesPerSec, &numData) == ERROR_SUCCESS);

	numData = sizeof(_bitDepth);
	(reg.QueryValue("BitDepth", &type, (BYTE*)&_bitDepth, &numData) == ERROR_SUCCESS);


	reg.CloseKey();
	reg.CloseRootKey();
	_configured = configured;
}

void DirectSound::WriteConfig()
{
	Registry reg;
	if (reg.OpenRootKey(HKEY_CURRENT_USER, CONFIG_ROOT_KEY) != ERROR_SUCCESS)
	{
		Error("Unable to write configuration to the registry");
		return;
	}
	if (reg.OpenKey("DirectSound") != ERROR_SUCCESS)
	{
		if (reg.CreateKey("DirectSound") != ERROR_SUCCESS)
		{
			Error("Unable to write configuration to the registry");
			return;
		}
	}
	reg.SetValue("NumBuffers", REG_DWORD, (BYTE*)&_numBuffers, sizeof(_numBuffers));
	reg.SetValue("BufferSize", REG_DWORD, (BYTE*)&_bufferSize, sizeof(_bufferSize));
	reg.SetValue("DeviceIndex", REG_DWORD, (BYTE*)&_deviceIndex, sizeof(_deviceIndex));
	if (_pDsGuid != NULL)
	{
		reg.SetValue("DeviceGuid", REG_BINARY, (BYTE*)_pDsGuid, sizeof(GUID));
	}
	else
	{
		reg.DeleteValue("DeviceGuid");
	}
	reg.SetValue("Dither", REG_BINARY, (BYTE*)&_dither, sizeof(_dither));
	reg.SetValue("Exclusive", REG_BINARY, (BYTE*)&_exclusive, sizeof(_exclusive));
	reg.SetValue("SamplesPerSec", REG_DWORD, (BYTE*)&_samplesPerSec, sizeof(_samplesPerSec));
	reg.SetValue("BitDepth", REG_DWORD, (BYTE*)&_bitDepth, sizeof(_bitDepth));

	reg.CloseKey();
	reg.CloseRootKey();
}


void DirectSound::Initialize(
	HWND hwnd,
	AUDIODRIVERWORKFN pCallback,
	void* context)
{
	_callbackContext = context;
	_pCallback = pCallback;
	_running = false;
	_hwnd = hwnd;
	ReadConfig();
	_initialized = true;
}

void DirectSound::Reset()
{
	Stop();
}

void DirectSound::Configure()
{
	ReadConfig();

	CDSoundConfig dlg;
	dlg.m_numBuffers = _numBuffers;
	dlg.m_bufferSize = _bufferSize;
	dlg.m_deviceIndex = _deviceIndex;
	dlg.m_dither = _dither;
	dlg.m_exclusive = _exclusive;
	dlg.m_sampleRate = _samplesPerSec;

	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	
	int numBuffers = _numBuffers;
	int bufferSize = _bufferSize;
	int deviceIndex = _deviceIndex;
	bool dither = _dither;
	bool exclusive = _exclusive;
	int samplesPerSec = _samplesPerSec;
	
	_configured = true;

	GUID dsGuid;
	memcpy(&dsGuid, &_dsGuid, sizeof(GUID));

	if (_initialized)
	{
		Stop();
	}

	_numBuffers = dlg.m_numBuffers;
	_bufferSize = dlg.m_bufferSize;
	_deviceIndex = dlg.m_deviceIndex;
	_pDsGuid = dlg.m_pDeviceGuid;		// ¿!!¿¿!¿!¿!¿¿!¿!
	if (_pDsGuid != NULL)				// This pointer is temporary!!!
	{
		memcpy(&_dsGuid, &dlg.m_deviceGuid, sizeof(GUID));
		_pDsGuid = &_dsGuid;
	}
	_dither = *(bool*)(&dlg.m_dither);
	_exclusive = *(bool*)(&dlg.m_exclusive);
	_samplesPerSec = dlg.m_sampleRate;

	if (_initialized)
	{
		if (Start())
		{
			WriteConfig();
		}
		else
		{
			_numBuffers = numBuffers;
			_bufferSize = bufferSize;
			_deviceIndex = deviceIndex;
			_dither = dither;
			_exclusive = exclusive;
			_samplesPerSec = samplesPerSec;
			memcpy(&_dsGuid, &dsGuid, sizeof(GUID));

			Start();
		}
	}
	else
	{
		WriteConfig();
	}
	
}

int DirectSound::GetPlayPos()
{
	if (!_running)
	{
		return 0;
	}

	int playPos;
	if (FAILED(_pBuffer->GetCurrentPosition((DWORD*)&playPos, NULL)))
	{
		Error("DirectSoundBuffer::GetCurrentPosition failed");
		return 0;
	}
	return playPos;
}

int DirectSound::GetWritePos()
{
	if (!_running)
	{
		return 0;
	}

	int writePos;
	if (FAILED(_pBuffer->GetCurrentPosition(NULL, (DWORD*)&writePos)))
	{
		Error("DirectSoundBuffer::GetCurrentPosition failed");
		return 0;
	}
	return writePos;
}

bool DirectSound::Enable(bool e)
{
	return e ? Start() : Stop();
}
