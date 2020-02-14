// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#define DIRECTSOUND_VERSION 0x8000

// linking
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

// includes
#include <string.h>
#include "../driver.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>

#define BYTES_PER_SAMPLE 4	// 2 * 16bits
#define SHORT_MIN	-32768
#define SHORT_MAX	32767

// AVRT is the new "multimedia scheduling stuff"
typedef HANDLE(WINAPI* FAvSetMmThreadCharacteristics)   (LPCTSTR, LPDWORD);
typedef BOOL(WINAPI* FAvRevertMmThreadCharacteristics)(HANDLE);

static FAvSetMmThreadCharacteristics pAvSetMmThreadCharacteristics = NULL;
static FAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics = NULL;
static HMODULE hDInputDLL = 0;

#define _GetProc(fun, type, name) \
{                                                  \
    fun = (type) GetProcAddress(hDInputDLL,name);  \
    if (fun == NULL) { return FALSE; }             \
}

//Dynamic load and unload of avrt.dll, so the executable can run on Windows 2K and XP.
static BOOL SetupAVRT(void)
{	
	hDInputDLL = LoadLibraryA("avrt.dll");
	if (hDInputDLL == NULL)
		return FALSE;

	_GetProc(pAvSetMmThreadCharacteristics, FAvSetMmThreadCharacteristics, "AvSetMmThreadCharacteristicsA");
	_GetProc(pAvRevertMmThreadCharacteristics, FAvRevertMmThreadCharacteristics, "AvRevertMmThreadCharacteristics");

	return pAvSetMmThreadCharacteristics &&
		pAvRevertMmThreadCharacteristics;
}

// ------------------------------------------------------------------------------------------
static void CloseAVRT(void)
{
	if (hDInputDLL != NULL)
		FreeLibrary(hDInputDLL);
	hDInputDLL = NULL;
}


typedef struct {			
	HANDLE Handle;
	unsigned char *pData;
	WAVEHDR *pHeader;
	HANDLE HeaderHandle;
	int Prepared;
} CBlock;

typedef struct {		
	psy_AudioDriver driver;	
	HWND m_hWnd;
	int _dither;
	int _bitDepth;
	unsigned int _samplesPerSec;		
	unsigned int pollSleep_;	
	
	int _initialized;
	int _configured;	
	// controls if the driver is supposed to be running or not
	int _running;
	// informs the real state of the DSound buffer (see the control of buffer play in DoBlocks())
	int _playing;
	// Controls if we want the thread to be running or not
	int _threadRun;	
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
	LPDIRECTSOUND8 _pDs;
	LPDIRECTSOUNDBUFFER8 _pBuffer;
	HANDLE hEvent;
	int (*error)(int, const char*);
} DXDriver;

static void driver_deallocate(psy_AudioDriver*);
static int driver_init(psy_AudioDriver*);
static void driver_connect(psy_AudioDriver*, void* context, AUDIODRIVERWORKFN callback,void* handle);
static int driver_open(psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, psy_Properties*);
static unsigned int driver_samplerate(psy_AudioDriver*);

static unsigned int totalbufferbytes(DXDriver*s);
static void PrepareWaveFormat(WAVEFORMATEX* wf, int channels, int sampleRate, int bits, int validBits);
static DWORD WINAPI NotifyThread(void* pDirectSound);
static DWORD WINAPI PollerThread(void* pDirectSound);
static void DoBlocks(DXDriver*);
static void init_properties(psy_AudioDriver* self);
static int on_error(int err, const char* msg);
static void Quantize(float *pin, int *piout, int c);
static BOOL isvistaorlater(void);
static BOOL WantsMoreBlocks(DXDriver*);

int f2i(float flt) 
{ 
#if defined(_WIN64)
	return (int)flt;
#else
  int i; 
  static const double half = 0.5f; 
  _asm 
  { 
	 fld flt 
	 fsub half 
	 fistp i 
  } 
  return i;
#endif
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
	info.Name = "DirectSound Output psy_AudioDriver";
	info.ShortName = "DXSOUND";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	DXDriver* dx = (DXDriver*) malloc(sizeof(DXDriver));
	if (dx != 0) {
		memset(dx, 0, sizeof(DXDriver));
		dx->driver.open = driver_open;
		dx->driver.deallocate = driver_deallocate;
		dx->driver.connect = driver_connect;
		dx->driver.open = driver_open;
		dx->driver.close = driver_close;
		dx->driver.dispose = driver_dispose;
		dx->driver.configure = driver_configure;
		dx->driver.samplerate = driver_samplerate;
		driver_init(&dx->driver);
		return &dx->driver;
	}
	return 0;
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver->dispose(driver);
	free(driver);
}

int driver_init(psy_AudioDriver* driver)
{
	DXDriver* self = (DXDriver*) driver;	

	self->error = on_error;
	self->_initialized = FALSE;
	self->_configured = FALSE;
	self->_running = FALSE;
	self->_playing = FALSE;	
	self->_pDs = NULL;
	self->_pBuffer = NULL;
	self->driver._pCallback = NULL;
	self->device_guid_ = DSDEVID_DefaultPlayback;
	self->_numBuffers = 6;
	self->_bufferSize = 4096;	
	self->_samplesPerSec= 44100;	
	self->_dither = 0;
	self->_bitDepth = 16;	

	init_properties(&self->driver);
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	SetupAVRT();
//	driver_configure(&self->driver);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	DXDriver* self = (DXDriver*) driver;
	properties_free(self->driver.properties);
	self->driver.properties = 0;
	CloseHandle(self->hEvent);
	CloseAVRT();
	return 0;
}

static void init_properties(psy_AudioDriver* self)
{	
	psy_Properties* property;	

	self->properties = psy_properties_create();
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "name", "directsound"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "vendor", "Psycedelics"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "version", "1.0"),
		PSY_PROPERTY_HINT_READONLY);
	property = psy_properties_append_choice(self->properties, "device", -1);	
	psy_properties_append_int(self->properties, "bitdepth", 16, 0, 32);
	psy_properties_append_int(self->properties, "samplerate", 44100, 0, 0);
	psy_properties_append_int(self->properties, "dither", 0, 0, 1);
	psy_properties_append_int(self->properties, "numbuf", 8, 6, 8);
	psy_properties_append_int(self->properties, "numsamples", 4096, 128, 8193);	
}

void driver_configure(psy_AudioDriver* driver, psy_Properties* config)
{
	DXDriver* self;
	psy_Properties* property;

	self = (DXDriver*) driver;
	if (config) {
		properties_free(self->driver.properties);
		self->driver.properties = psy_properties_clone(config, 1);
	} else {
		property = psy_properties_read(self->driver.properties, "bitdepth");
		if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
			self->_bitDepth = property->item.value.i;
		}
		property = psy_properties_read(self->driver.properties, "samplerate");
		if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
			self->_samplesPerSec = property->item.value.i;
		}
		property = psy_properties_read(self->driver.properties, "numbuf");
		if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
			self->_numBuffers = property->item.value.i;
		}
		property = psy_properties_read(self->driver.properties, "numsamples");
		if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
			self->_bufferSize = property->item.value.i;
		}
	}
}

unsigned int driver_samplerate(psy_AudioDriver* self)
{
	return ((DXDriver*)self)->_samplesPerSec;
}

void driver_connect(psy_AudioDriver* driver, void* context, AUDIODRIVERWORKFN callback, void* handle)
{	
	driver->_callbackContext = context;
	driver->_pCallback = callback;
	((DXDriver*)driver)->m_hWnd = (HWND) handle;
}

int driver_open(psy_AudioDriver* driver)
{
	DSBCAPS caps;
	DSBUFFERDESC desc;
	WAVEFORMATEX format;
	LPDIRECTSOUNDBUFFER pBufferGen;
	HRESULT hr;
	DWORD dwThreadId;

	DXDriver* self = (DXDriver*)driver;
	//CSingleLock lock(&_lock, TRUE);	
	if (self->_running) {
		return TRUE;
	}
	if (self->m_hWnd == NULL) {
		return FALSE;
	}
	if (self->driver._pCallback == NULL) {
		return FALSE;
	}
	if (FAILED(DirectSoundCreate8(&self->device_guid_, &self->_pDs, NULL)))
	{
		self->error(1, "Failed to create DirectSound object");
		return FALSE;
	}		
	if (FAILED(IDirectSound_SetCooperativeLevel(self->_pDs, self->m_hWnd, DSSCL_PRIORITY))) {
			self->error(1, "Failed to set DirectSound cooperative level");
			IDirectSound_Release(self->_pDs);
			self->_pDs = NULL;
			return FALSE;
	}
	self->_dsBufferSize = totalbufferbytes(self);
	PrepareWaveFormat(&format, 2, self->_samplesPerSec, self->_bitDepth, self->_bitDepth);			
	ZeroMemory(&desc, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	desc.dwBufferBytes = self->_dsBufferSize;
	desc.dwReserved = 0;
	desc.lpwfxFormat = &format;
	desc.guid3DAlgorithm = GUID_NULL;	

	if (FAILED(IDirectSound_CreateSoundBuffer(self->_pDs, &desc, &pBufferGen, NULL)))
	{
		self->error(1, "Failed to create DirectSound psy_audio_Buffer(s)");
		IDirectSound_Release(self->_pDs);
		self->_pDs = NULL;
		return FALSE;
	}		
	hr = IDirectSound_QueryInterface(pBufferGen, &IID_IDirectSoundBuffer8, (LPVOID*)&self->_pBuffer);
	IDirectSound_Release(pBufferGen);
	if (FAILED(hr))
	{
		self->error(1, "Failed to obtain version 8 interface for Buffer");
		self->_pBuffer = 0;
		IDirectSound_Release(self->_pDs);
		self->_pDs = 0;
		return FALSE;
	}

	if (self->_exclusive)
	{
		IDirectSoundBuffer_Stop(self->_pBuffer);
		if (FAILED(IDirectSoundBuffer_SetFormat(self->_pBuffer, &format)))
		{
			self->error(1, "Failed to set DirectSound psy_audio_Buffer format");
			IDirectSoundBuffer_Release(self->_pBuffer);
			self->_pBuffer = NULL;
			IDirectSound_Release(self->_pDs);
			self->_pDs = NULL;
			return FALSE;
		}
		caps.dwSize = sizeof(caps);
		if (FAILED(IDirectSoundBuffer_GetCaps(self->_pBuffer, &caps)))
		{
			self->error(1, "Failed to get DirectSound psy_audio_Buffer capabilities");
			IDirectSoundBuffer_Release(self->_pBuffer);
			self->_pBuffer = NULL;
			IDirectSound_Release(self->_pDs);
			self->_pDs = NULL;
			return FALSE;
		}
		self->_dsBufferSize = caps.dwBufferBytes;
		//WriteConfig();
	}

	ResetEvent(self->hEvent);
	self->_threadRun = TRUE;
	self->_playing = FALSE;	

#define DIRECTSOUND_POLLING P
#ifdef DIRECTSOUND_POLLING
	CreateThread(NULL, 0, PollerThread, self, 0, &dwThreadId);
#else
	CreateThread(NULL, 0, NotifyThread, self, 0, &dwThreadId);
#endif
	self->_running = TRUE;
	return TRUE;

	/*self->_lowMark = 0;
	self->_highMark = self->_bufferSize;
	if (self->_highMark >= self->_dsBufferSize)
	{
		self->_highMark = self->_dsBufferSize-1;
	}
	self->_currentOffset = 0;
	self->_buffersToDo = self->_numBuffers;
	ResetEvent(self->hEvent);
	self->_timerActive = TRUE;
	_beginthread(PollerThread, 0, self);

	self->_running = TRUE;
	return TRUE;*/
}

unsigned int totalbufferbytes(DXDriver* self)
{
	return self->_bufferSize * self->_numBuffers;
}

void PrepareWaveFormat(WAVEFORMATEX* wf, int channels, int sampleRate, int bits, int validBits)
{
	// Set up wave format structure. 
	ZeroMemory(wf, sizeof(WAVEFORMATEX));
	wf->wFormatTag = WAVE_FORMAT_PCM;
	wf->nChannels = channels;
	wf->wBitsPerSample = bits;
	wf->nSamplesPerSec = sampleRate;
	wf->nBlockAlign = channels * bits / 8;
	wf->nAvgBytesPerSec = sampleRate * wf->nBlockAlign;
	wf->cbSize = 0;
	if(bits <= 16) {		
		wf->cbSize = 0;
	} else {
/*		wf->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wf.Format.cbSize = 0x16;
				wf.psy_audio_Samples.wValidBitsPerSample  = validBits;
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

DWORD WINAPI PollerThread(void* self)
{	
	DXDriver* pThis = (DXDriver*)self;
	HANDLE hTask = NULL;
	int i;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	// Ask MMCSS to temporarily boost the thread priority
	// to reduce glitches while the low-latency stream plays.	
	if (isvistaorlater())
	{
		DWORD taskIndex = 0;
		hTask = pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
	}
	// Prefill buffer:
	pThis->_lowMark = 0;
	pThis->m_readPosWraps = 0;
	pThis->_highMark = pThis->_bufferSize;	
	pThis->_currentOffset = 0;
	for (i = 0; i < pThis->_numBuffers; ++i) {
		// Directsound playback buffer is started here.
		DoBlocks(pThis);
	}
	while (pThis->_threadRun)
	{
		int runs = 0;

		while (WantsMoreBlocks(pThis))
		{
			// First, run the capture buffers so that audio is available to wavein machines.
			
			// Next, proceeed with the generation of audio
			DoBlocks(pThis);
			if (++runs > pThis->_numBuffers)
				break;
		}
		Sleep(1);
	}	
	SetEvent(pThis->hEvent);	
	return 0;
}

DWORD WINAPI NotifyThread(void* self)
{
	return 0;
}

BOOL WantsMoreBlocks(DXDriver* self)
{
	// [_lowMark,_highMark) is the next buffer to be filled.
	// if (play) pos is still inside, we have to wait.
	int pos = 0;
	HRESULT hr;
	
	hr = IDirectSoundBuffer8_GetCurrentPosition(self->_pBuffer, &pos, 0);
	if (FAILED(hr)) return FALSE;
	if (self->_lowMark <= pos && pos < self->_highMark)	return FALSE;
	return TRUE;
}

void DoBlocks(DXDriver* self)
{
	int* pBlock1, * pBlock2;
	unsigned long blockSize1, blockSize2;
	HRESULT hr;
	
	hr = IDirectSoundBuffer_Lock(
		self->_pBuffer,
		(DWORD)self->_lowMark, (DWORD)self->_bufferSize,
		(void**)&pBlock1, (DWORD*)&blockSize1,
		(void**)&pBlock2, (DWORD*)&blockSize2,
		0);	
	if (hr == DSERR_BUFFERLOST)
	{
		// If DSERR_BUFFERLOST is returned, restore and retry lock. 
		self->_playing = FALSE;
		IDirectSoundBuffer_Restore(self->_pBuffer);
		if (self->_highMark < self->_dsBufferSize) {
			IDirectSoundBuffer8_SetCurrentPosition(self->_pBuffer, self->_highMark);
		} else {
			IDirectSoundBuffer8_SetCurrentPosition(self->_pBuffer, 0);			
		}
		hr = IDirectSoundBuffer_Lock(
			self->_pBuffer,
			(DWORD)self->_lowMark, (DWORD)self->_bufferSize,
			(void**)&pBlock1, (DWORD*)&blockSize1,
			(void**)&pBlock2, (DWORD*)&blockSize2,
			0);
	}
	if (SUCCEEDED(hr))
	{
		// Generate audio and put it into the buffer
		unsigned int _sampleValidBits = self->_bitDepth;
		int numSamples = blockSize1 / 4; // GetSampleSizeBytes();
		int hostisplaying;
		float* pFloatBlock = 
			self->driver._pCallback(
				self->driver._callbackContext, &numSamples, &hostisplaying);			
		if (_sampleValidBits == 32) {			
			// dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pBlock1), numSamples * 2, 1.f / 32768.f);
		}
		else if (_sampleValidBits == 24) {
			// Quantize24in32Bit(pFloatBlock, pBlock1, numSamples);
		}
		else if (_sampleValidBits == 16) {
			Quantize(pFloatBlock, pBlock1, numSamples);
			// if (settings_->dither()) Quantize16WithDither(pFloatBlock, pBlock1, numSamples);
			// else Quantize16(pFloatBlock, pBlock1, numSamples);
		}
		self->_lowMark += blockSize1;
		if (blockSize2 > 0)
		{
			float* pFloatBlock;
			numSamples = blockSize2 / 4; // GetSampleSizeBytes();
			pFloatBlock = self->driver._pCallback(
					self->driver._callbackContext, &numSamples, &hostisplaying);
				
				//_pCallback(self->driver._callbackContext, numSamples);
			if (_sampleValidBits == 32) {
				// dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pBlock2), numSamples * 2, 1.f / 32768.f);
			}
			else if (_sampleValidBits == 24) {
				// Quantize24in32Bit(pFloatBlock, pBlock2, numSamples);
			}
			else if (_sampleValidBits == 16) {
				//if (settings_->dither()) Quantize16WithDither(pFloatBlock, pBlock2, numSamples);
				//else Quantize16(pFloatBlock, pBlock2, numSamples);
				Quantize(pFloatBlock, pBlock2, numSamples);
			}
			self->_lowMark += blockSize2;
		}
		// Release the data back to DirectSound. 		
		hr = IDirectSoundBuffer_Unlock(self->_pBuffer, pBlock1, blockSize1, pBlock2, blockSize2);
		if (self->_lowMark >= self->_dsBufferSize) {
			self->_lowMark -= self->_dsBufferSize;
			self->m_readPosWraps++;
#if defined _MSC_VER > 1200
			if ((uint64_t)self->m_readPosWraps * 
				(uint64_t)self->_dsBufferSize >= 0x100000000LL)
			{
				self->m_readPosWraps = 0;
				// PsycleGlobal::midi().ReSync();	// MIDI IMPLEMENTATION
			}
#else
			if ((uint64_t)self->m_readPosWraps * 
				(uint64_t)self->_dsBufferSize >= 0x100000000L)
			{
				self->m_readPosWraps = 0;
				// PsycleGlobal::midi().ReSync();	// MIDI IMPLEMENTATION
			}
#endif
		}
		self->_highMark = self->_lowMark + self->_bufferSize;
		if (SUCCEEDED(hr) && !self->_playing)
		{
			IDirectSoundBuffer8_SetCurrentPosition(self->_pBuffer, self->_highMark);						
			IDirectSoundBuffer_Play(self->_pBuffer, 0, 0, DSBPLAY_LOOPING);

			if (SUCCEEDED(hr)) {
				self->_playing = TRUE;
				// PsycleGlobal::midi().ReSync(); // MIDI IMPLEMENTATION
			}
		}
	}
}

int driver_close(psy_AudioDriver* driver)
{
	DXDriver* self = (DXDriver*) driver;	

	if (!self->_running) return TRUE;
	self->_threadRun = FALSE;
	WaitForSingleObject(self->hEvent, INFINITE);
	// Once we get here, the PollerThread should have stopped
	if (self->_playing) {
		IDirectSoundBuffer_Stop(self->_pBuffer);		
		self->_playing = FALSE;
	}
	IDirectSoundBuffer_Release(self->_pBuffer);	
	self->_pBuffer = NULL;
	IDirectSound_Release(self->_pDs);
	self->_pDs = NULL;		
	self->_running = FALSE;
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

BOOL isvistaorlater(void)
{
#if defined _MSC_VER > 1200
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	int op = VER_GREATER_EQUAL;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 0;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
	// Perform the test.
	return VerifyVersionInfo(
		&osvi, VER_MAJORVERSION | VER_MINORVERSION,
		dwlConditionMask);
#else
	return 0;
#endif
}
