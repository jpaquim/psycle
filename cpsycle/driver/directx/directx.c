// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../driver.h"
#include <windows.h>
//#include <mmsystem.h>
#include <process.h>
#define DIRECTSOUND_VERSION 0x8000
#include <dsound.h>
#include <MMReg.h>

static HWND hwndmain;

#define BYTES_PER_SAMPLE 4	// 2 * 16bits
#define SHORT_MIN	-32768
#define SHORT_MAX	32767

typedef struct {			
	HANDLE Handle;
	unsigned char *pData;
	WAVEHDR *pHeader;
	HANDLE HeaderHandle;
	int Prepared;
} CBlock;

typedef struct {		
	Driver driver;	
	int _dither;
	int _bitDepth;
	unsigned int _samplesPerSec;		
	unsigned int pollSleep_;	
	
	int _initialized;
	int _configured;	
	//controls if the driver is supposed to be running or not
	int _running;
	//informs the real state of the DSound buffer (see the control of buffer play in DoBlocks())
	int _playing;
	//Controls if we want the thread to be running or not
	int _threadRun;
	int _timerActive;		
	void* _callbackContext;	
	GUID device_guid_;
	int _deviceIndex;	
	int _numBuffers;
	int _bufferSize;
	int _dsBufferSize;
	int _currentOffset;
	int _lowMark;
	int _highMark;
	int _buffersToDo;
	int _exclusive;
	/// number of "wraparounds" to compensate the GetCurrentPosition() call.
	int m_readPosWraps;
	LPCGUID _pDsGuid;
	LPDIRECTSOUND _pDs;
	LPDIRECTSOUNDBUFFER _pBuffer;

	int (*error)(int, const char*);
} DXDriver;

static void driver_free(Driver*);
static int driver_init(Driver*);
static void driver_connect(Driver*, void* context, AUDIODRIVERWORKFN callback,void* handle);
static int driver_open(Driver*);
static int driver_close(Driver*);
static int driver_dispose(Driver*);
static void updateconfiguration(Driver*);
static unsigned int samplerate(Driver*);

static void PrepareWaveFormat(WAVEFORMATEX* wf, int channels, int sampleRate, int bits, int validBits);
static void PollerThread(void *pWaveOut);
static void DoBlocks(DXDriver* self);
static void init_properties(Driver* self);
static void apply_properties(DXDriver* self);
static int on_error(int err, const char* msg);
static void Quantize(float *pin, int *piout, int c);


int f2i(float flt) 
{ 
  int i; 
  static const double half = 0.5f; 
  _asm 
  { 
	 fld flt 
	 fsub half 
	 fistp i 
  } 
  return i;
}

int on_error(int err, const char* msg)
{
	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void)
{
	static AudioDriverInfo info;
	info.Flags = 0;
	info.Name = "DirectSound Output Driver";
	info.ShortName = "DXSOUND";
	info.Version = 0;
	return &info;
}

EXPORT Driver* __cdecl driver_create(void)
{
	DXDriver* dx = (DXDriver*) malloc(sizeof(DXDriver));
	memset(dx, 0, sizeof(DXDriver));
	dx->driver.open = driver_open;
	dx->driver.free = driver_free;
	dx->driver.init = driver_init;
	dx->driver.connect = driver_connect;
	dx->driver.open = driver_open;
	dx->driver.close = driver_close;
	dx->driver.dispose = driver_dispose;
	dx->driver.updateconfiguration = updateconfiguration;
	dx->driver.samplerate = samplerate;

	return &dx->driver;
}

void driver_free(Driver* driver)
{
	free(driver);
}

int driver_init(Driver* driver)
{
	DXDriver* self = (DXDriver*) driver;	

	self->error = on_error;
	self->_initialized = FALSE;
	self->_configured = FALSE;
	self->_running = FALSE;
	self->_playing = FALSE;
	self->_timerActive = FALSE;
	self->_pDs = NULL;
	self->_pBuffer = NULL;
	self->driver._pCallback = NULL;
	self->device_guid_ = DSDEVID_DefaultPlayback;
	self->_numBuffers = 4;
	self->_bufferSize = 4096;	
	self->_samplesPerSec= 44100;	
	self->pollSleep_ = 20;
	self->_dither = 0;
	self->_bitDepth = 16;

	init_properties(&self->driver);
//	apply_properties(self);
	return 0;
}

int driver_dispose(Driver* driver)
{
	DXDriver* self = (DXDriver*) driver;
	properties_free(self->driver.properties);
	self->driver.properties = 0;
	return 0;
}

static void init_properties(Driver* self)
{	
	Properties* property;	

	self->properties = properties_create_string("name", "directsound");
	properties_append_string(self->properties, "version", "1.0");
	property = properties_append_choice(self->properties, "device", -1);	
	properties_append_int(self->properties, "bitdepth", 16, 0, 32);
	properties_append_int(self->properties, "samplerate", 44100, 0, 0);
	properties_append_int(self->properties, "dither", 0, 0, 1);
	properties_append_int(self->properties, "numbuf", 8, 6, 8);
	properties_append_int(self->properties, "numsamples", 4096, 128, 8193);	
}

static void apply_properties(DXDriver* self)
{
	Properties* property;

	property = properties_read(self->driver.properties, "bitdepth");
	if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
		self->_bitDepth = property->item.value.i;
	}
	property = properties_read(self->driver.properties, "samplerate");
	if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
		self->_samplesPerSec = property->item.value.i;
	}
	property = properties_read(self->driver.properties, "numbuf");
	if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
		self->_numBuffers = property->item.value.i;
	}
	property = properties_read(self->driver.properties, "numsamples");
	if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
		self->_bufferSize = property->item.value.i;
	}	
}

void updateconfiguration(Driver* self)
{
	apply_properties((DXDriver*)self);
}

unsigned int samplerate(Driver* self)
{
	return ((DXDriver*)self)->_samplesPerSec;
}

void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback, void* handle)
{
	DXDriver* self = (DXDriver*) driver;
	self->_callbackContext = context;
	self->driver._pCallback = callback;
	hwndmain = (HWND) handle;
}

int driver_open(Driver* driver)
{
	DSBCAPS caps;
	DSBUFFERDESC desc;
	WAVEFORMATEX format;

	DXDriver* self = (DXDriver*)driver;
	//CSingleLock lock(&_lock, TRUE);	
	if (self->_running)
	{
		return TRUE;
	}
	if (self->driver._pCallback == NULL)
	{
		return FALSE;
	}

	if (FAILED(DirectSoundCreate(self->_pDsGuid, &self->_pDs, NULL)))
	{
		self->error(1, "Failed to create DirectSound object");
		return FALSE;
	}

	if (self->_exclusive)
	{
		if (FAILED(IDirectSound_SetCooperativeLevel(self->_pDs, hwndmain, DSSCL_WRITEPRIMARY)))
		{
		// Don't report this, since we may have simply have lost focus
		//
		//	Error("Failed to set DirectSound cooperative level");
			IDirectSound_Release(self->_pDs);
			self->_pDs = NULL;
			return FALSE;
		}
	}
	else
	{
		if (FAILED(IDirectSound_SetCooperativeLevel(self->_pDs, hwndmain, DSSCL_PRIORITY)))
		{
			self->error(1, "Failed to set DirectSound cooperative level");
			IDirectSound_Release(self->_pDs);
			self->_pDs = NULL;
			return FALSE;
		}
	}

	self->_dsBufferSize = self->_exclusive ? 0 : self->_bufferSize*self->_numBuffers;
	
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.wBitsPerSample = 16;//_bitDepth;
	format.nSamplesPerSec = self->_samplesPerSec;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
	desc.dwFlags |= self->_exclusive ? (DSBCAPS_PRIMARYBUFFER) : (DSBCAPS_GLOBALFOCUS);
	desc.dwBufferBytes = self->_dsBufferSize; 
	desc.dwReserved = 0;
	desc.lpwfxFormat = self->_exclusive ? NULL : &format;
	desc.guid3DAlgorithm = GUID_NULL;
	
	if (FAILED(IDirectSound_CreateSoundBuffer(self->_pDs, &desc, &self->_pBuffer, NULL)))
	{
		self->error(1, "Failed to create DirectSound Buffer(s)");
		IDirectSound_Release(self->_pDs);
		self->_pDs = NULL;
		return FALSE;
	}

	if (self->_exclusive)
	{
		IDirectSoundBuffer_Stop(self->_pBuffer);
		if (FAILED(IDirectSoundBuffer_SetFormat(self->_pBuffer, &format)))
		{
			self->error(1, "Failed to set DirectSound Buffer format");
			IDirectSoundBuffer_Release(self->_pBuffer);
			self->_pBuffer = NULL;
			IDirectSound_Release(self->_pDs);
			self->_pDs = NULL;
			return FALSE;
		}
		caps.dwSize = sizeof(caps);
		if (FAILED(IDirectSoundBuffer_GetCaps(self->_pBuffer, &caps)))
		{
			self->error(1, "Failed to get DirectSound Buffer capabilities");
			IDirectSoundBuffer_Release(self->_pBuffer);
			self->_pBuffer = NULL;
			IDirectSound_Release(self->_pDs);
			self->_pDs = NULL;
			return FALSE;
		}
		self->_dsBufferSize = caps.dwBufferBytes;
		//WriteConfig();
	}
	IDirectSoundBuffer_Initialize(self->_pBuffer, self->_pDs, &desc);

	self->_lowMark = 0;
	self->_highMark = self->_bufferSize;
	if (self->_highMark >= self->_dsBufferSize)
	{
		self->_highMark = self->_dsBufferSize-1;
	}
	self->_currentOffset = 0;
	self->_buffersToDo = self->_numBuffers;
	//_event.ResetEvent();
	self->_timerActive = TRUE;
	_beginthread(PollerThread, 0, self);

	self->_running = TRUE;
	return TRUE;
}



void PrepareWaveFormat(WAVEFORMATEX* wf, int channels, int sampleRate, int bits, int validBits)
{
	// Set up wave format structure. 
	ZeroMemory(wf, sizeof(WAVEFORMATEX));
	wf->nChannels = channels;
	wf->wBitsPerSample = bits;
	wf->nSamplesPerSec = sampleRate;
	wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
	wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;

	if(bits <= 16) {
		wf->wFormatTag = WAVE_FORMAT_PCM;
		wf->cbSize = 0;
	} else {
/*		wf->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wf.Format.cbSize = 0x16;
				wf.Samples.wValidBitsPerSample  = validBits;
				if(channels == 2) {
					wf.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
				}
				if(validBits ==32) {
					wf.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
				}
				else {
					wf.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
				}
			}*/
	}
}

void PollerThread(void * self)
{
	DXDriver* pThis = (DXDriver*)self;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	while (pThis->_timerActive)
	{
		DoBlocks(pThis);
		Sleep(pThis->pollSleep_);
	}
	//_event.SetEvent();
	_endthread();
}

void DoBlocks(DXDriver* self)
{
	int pos;
	HRESULT hr;
	int playing = self->_playing;
	int* pBlock;
	int* pBlock1;
	int* pBlock2;
	int blockSize;
	int blockSize1;	
	int blockSize2;
	int currentOffset;
	
	while(TRUE)
	{
		while(TRUE)
		{
			hr = IDirectSoundBuffer_GetCurrentPosition(self->_pBuffer, (DWORD*)&pos, NULL);
			if (FAILED(hr))
			{
				if (hr == DSERR_BUFFERLOST)
				{
					playing = FALSE;
					if (FAILED(IDirectSoundBuffer_Restore(self->_pBuffer)))
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
					self->error(1, "DirectSoundBuffer::GetCurrentPosition failed");
					return;
				}
			}
			break;
		}

		if (self->_highMark < self->_lowMark)
		{
			if ((pos > self->_lowMark) || (pos < self->_highMark))
			{
				return;
			}
		}
		else if ((pos > self->_lowMark) && (pos < self->_highMark))
		{
			return;
		}


		currentOffset = self->_currentOffset;
		while (self->_buffersToDo != 0)
		{
			while(TRUE)
			{
				hr = IDirectSoundBuffer_Lock(
					self->_pBuffer,
					(DWORD)currentOffset, (DWORD)self->_bufferSize,
					(void**)&pBlock1, (DWORD*)&blockSize1,
					(void**)&pBlock2, (DWORD*)&blockSize2,
					0);
				if (FAILED(hr))
				{
					if (hr == DSERR_BUFFERLOST)
					{
						playing = FALSE;
						if (FAILED(IDirectSoundBuffer_Restore(self->_pBuffer)))
						{
							return;
						}
						continue;
					}
					else
					{
						self->error(1, "Failed to lock DirectSoundBuffer");
						return;
					}
				}
				break;
			}
		
			blockSize = blockSize1 / BYTES_PER_SAMPLE;
			pBlock = pBlock1;
			while(blockSize > 0)
			{
				int n = blockSize;
				float *pFloatBlock = self->driver._pCallback(
					self->_callbackContext, 
					&n);
				if (self->_dither)
				{
//					QuantizeWithDither(pFloatBlock, pBlock, n);
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
				float *pFloatBlock = self->driver._pCallback(self->_callbackContext, &n);
				if (self->_dither)
				{
					//QuantizeWithDither(pFloatBlock, pBlock, n);
				}
				else
				{
					Quantize(pFloatBlock, pBlock, n);
				}
				pBlock += n;
				blockSize -= n;
			}
	
			IDirectSoundBuffer_Unlock(self->_pBuffer, pBlock1, blockSize1, pBlock2, blockSize2);
			self->_currentOffset += self->_bufferSize;
			if (self->_currentOffset >= self->_dsBufferSize)
			{
				self->_currentOffset -= self->_dsBufferSize;
			}
			self->_lowMark += self->_bufferSize;
			if (self->_lowMark >= self->_dsBufferSize)
			{
				self->_lowMark -= self->_dsBufferSize;
			}
			self->_highMark += self->_bufferSize;
			if (self->_highMark >= self->_dsBufferSize)
			{
				self->_highMark -= self->_dsBufferSize;
			}
			self->_buffersToDo--;
		} // while (_buffersToDo != 0)
		self->_buffersToDo = 1;
		if (!playing)
		{
			self->_playing = TRUE;
			IDirectSoundBuffer_Play(self->_pBuffer, 0, 0, DSBPLAY_LOOPING);
		}
	} // while (true)

} 

int driver_close(Driver* driver)
{
	DXDriver* self = (DXDriver*) driver;
	if (!self->_running)
	{
		return TRUE;
	}
	self->_running = FALSE;
	self->_timerActive = FALSE;
	// CSingleLock event(&_event, TRUE);
	// Once we get here, the PollerThread should have stopped
	//
	if (self->_playing)
	{
		IDirectSoundBuffer_Stop(self->_pBuffer);
		self->_playing = FALSE;
	}
	IDirectSoundBuffer_Release(self->_pBuffer);
	self->_pBuffer = NULL;
	IDirectSound_Release(self->_pDs);
	self->_pDs = NULL;

	return TRUE;
}

void Quantize(float *pin, int *piout, int c)
{
//	double const d2i = (1.5 * (1 << 26) * (1 << 26));
	
	do
	{
		int l;
//		double res = ((double)pin[1]) + d2i;
//		int r = *(int *)&res;
		int r = f2i(pin[1]);

		if (r < SHORT_MIN)
		{
			r = SHORT_MIN;
		}
		else if (r > SHORT_MAX)
		{
			r = SHORT_MAX;
		}
//		res = ((double)pin[0]) + d2i;
//		int l = *(int *)&res;
		l = f2i(pin[0]);

		if (l < SHORT_MIN)
		{
			l = SHORT_MIN;
		}
		else if (l > SHORT_MAX)
		{
			l = SHORT_MAX;
		}
		*piout++ = (r << 16) | (WORD)l;
		pin += 2;
	}
	while(--c);
}
