// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "../driver.h"
#include <windows.h>
#include <mmsystem.h>
#include <process.h>

#define BYTES_PER_SAMPLE 4	// 2 * 16bits
#define SHORT_MIN	-32768
#define SHORT_MAX	32767

#define MAX_WAVEOUT_BLOCKS 8

typedef struct {			
	HANDLE Handle;
	unsigned char *pData;
	WAVEHDR *pHeader;
	HANDLE HeaderHandle;
	int Prepared;
} CBlock;

typedef struct {
	Driver driver;
	HWAVEOUT _handle;
	int _deviceId;
	int _dither;
	int _bitDepth;
	unsigned int _samplesPerSec;
	unsigned int _numBlocks;
	int _blockSizeBytes;
	int _blockSize;
	int _currentBlock;
	int _running;
	int _stopPolling;	
	unsigned int pollSleep_;
	unsigned int _writePos;
	/// number of "wraparounds" to compensate the WaveOutGetPosition() call.
	int m_readPosWraps;
	/// helper variable to detect the previous wraps.
	int m_lastPlayPos;		
	CBlock _blocks[MAX_WAVEOUT_BLOCKS];		
	void* _callbackContext;	
	int (*error)(int, const char*);
} MmeDriver;

static void driver_free(Driver*);
static int driver_init(Driver*);
static void driver_connect(Driver*, void* context, AUDIODRIVERWORKFN callback, void* handle);
static int driver_open(Driver*);
static int driver_close(Driver*);
static int driver_dispose(Driver*);
static void updateconfiguration(Driver*);
static unsigned int samplerate(Driver*);

static void PrepareWaveFormat(WAVEFORMATEX* wf, int channels, int sampleRate, int bits, int validBits);
static void PollerThread(void *pWaveOut);
static void DoBlocks(MmeDriver* self);
static void init_properties(Driver* self);
static void apply_properties(MmeDriver* self);
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
	info.Name = "Windows MME Driver";
	info.ShortName = "MME";
	info.Version = 0;
	return &info;
}

EXPORT Driver* __cdecl driver_create(void)
{
	MmeDriver* mme = (MmeDriver*) malloc(sizeof(MmeDriver));
	memset(mme, 0, sizeof(MmeDriver));
	mme->driver.open = driver_open;
	mme->driver.free = driver_free;
	mme->driver.init = driver_init;
	mme->driver.connect = driver_connect;
	mme->driver.open = driver_open;
	mme->driver.close = driver_close;
	mme->driver.dispose = driver_dispose;
	mme->driver.updateconfiguration = updateconfiguration;
	mme->driver.samplerate = samplerate;

	return &mme->driver;
}

void driver_free(Driver* driver)
{
	free(driver);
}

int driver_init(Driver* driver)
{
	MmeDriver* self = (MmeDriver*) driver;	
	/*self->_deviceId = 0; // WAVE_MAPPER;
	self->_bitDepth = 16;
	self->_samplesPerSec = 44100;
	self->_numBlocks = 6;
	self->_blockSizeBytes = 4096;
	self->pollSleep_ = 10;*/
	self->error = on_error;

	self->_samplesPerSec=44100;
	self->_deviceId=0;
	self->_numBlocks = 4;
	self->_blockSize = 4096;
	self->_blockSizeBytes = 4096;
	self->pollSleep_ = 20;
	self->_dither = 0;
//	self->_channelmode = 3;
	self->_bitDepth = 16;


	init_properties(&self->driver);
//	apply_properties(self);
	return 0;
}

int driver_dispose(Driver* driver)
{
	MmeDriver* self = (MmeDriver*) driver;
	properties_free(self->driver.properties);
	self->driver.properties = 0;
	return 0;
}

static void init_properties(Driver* self)
{		
	Properties* devices;
	int i;
	int n;

	self->properties = properties_create();
		
	properties_append_string(self->properties, "name", "winmme");
	properties_append_string(self->properties, "version", "1.0");
	devices = properties_append_choice(self->properties, "device", -1);		 
	properties_append_int(devices, "WAVE_MAPPER", -1, 0, 0);	
	n = waveOutGetNumDevs();	
	for (i = 0; i < n; i++)
	{
		WAVEOUTCAPS caps;
		waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS));
		properties_append_int(devices, caps.szPname, i, 0, 0);
	}
	properties_append_int(self->properties, "bitdepth", 16, 0, 32);
	properties_append_int(self->properties, "samplerate", 44100, 0, 0);
	properties_append_int(self->properties, "dither", 0, 0, 1);
	properties_append_int(self->properties, "numbuf", 8, 6, 8);
	properties_append_int(self->properties, "numsamples", 4096, 128, 8193);	
}

static void apply_properties(MmeDriver* self)
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
		self->_numBlocks = property->item.value.i;
	}
	property = properties_read(self->driver.properties, "numsamples");
	if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
		self->_blockSizeBytes = property->item.value.i;
	}
	property = properties_read(self->driver.properties, "numsamples");
	if (property && property->item.typ == PROPERTY_TYP_INTEGER) {
		self->_blockSizeBytes = property->item.value.i;
	}
}

void updateconfiguration(Driver* self)
{
	apply_properties((MmeDriver*)self);
}

unsigned int samplerate(Driver* self)
{
	return ((MmeDriver*)self)->_samplesPerSec;
}

void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback, void* handle)
{
	MmeDriver* self = (MmeDriver*) driver;
	self->_callbackContext = context;
	self->driver._pCallback = callback;
}

int driver_open(Driver* driver)
{
	MmeDriver* self = (MmeDriver*) driver;
	CBlock *pBlock;
	WAVEFORMATEX format;

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = 16;
	format.nSamplesPerSec = self->_samplesPerSec;
	format.cbSize = 0;
	format.nChannels = 2;

	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	if (waveOutOpen(&self->_handle, self->_deviceId, &format, 0UL, 0, 
		CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		self->error(1, "waveOutOpen() failed");
		return FALSE;
	}

	self->_currentBlock = 0;
	self->_writePos = 0;

	// allocate blocks
	for (pBlock = self->_blocks; pBlock < self->_blocks + self->_numBlocks; 
		pBlock++)
	{
		pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, self->_blockSize);
		pBlock->pData = (byte *)GlobalLock(pBlock->Handle);
	}

	// allocate block headers
	for (pBlock = self->_blocks; pBlock < self->_blocks + self->_numBlocks; pBlock++)
	{
		WAVEHDR *ph;

		pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
		pBlock->pHeader = (WAVEHDR *)GlobalLock(pBlock->HeaderHandle);

		ph = pBlock->pHeader;
		ph->lpData = (char *)pBlock->pData;
		ph->dwBufferLength = self->_blockSize;
		ph->dwFlags = WHDR_DONE;
		ph->dwLoops = 0;

		pBlock->Prepared = FALSE;
	}

	self->_stopPolling = FALSE;
	//_event.ResetEvent();
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
	MmeDriver* pThis = (MmeDriver*)self;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	while (!pThis->_stopPolling)
	{
		DoBlocks(pThis);
		Sleep(pThis->pollSleep_);
	}
	//_event.SetEvent();
	_endthread();
}

void DoBlocks(MmeDriver* self)
{
	CBlock *pb = self->_blocks + self->_currentBlock;

	while(pb->pHeader->dwFlags & WHDR_DONE)
	{
		int *pOut;
		int bs;
		if (pb->Prepared)
		{
			if (waveOutUnprepareHeader(self->_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				self->error(1, "waveOutUnprepareHeader() failed");
			}
			pb->Prepared = FALSE;
		}
		pOut = (int *)pb->pData;
		bs = self->_blockSizeBytes / BYTES_PER_SAMPLE;

		do
		{
			int n = bs;
			float *pBuf = self->driver._pCallback(self->_callbackContext, &n);
			if (self->_stopPolling == TRUE) {
				break;
			}
			if (self->_dither)
			{
				// QuantizeWithDither(pBuf, pOut, n);
			}
			else
			{
				Quantize(pBuf, pOut, n);
			}
			pOut += n;
			bs -= n;
		}
		while (bs > 0);

		if (self->_stopPolling == TRUE) {
			break;
		}

		self->_writePos += self->_blockSizeBytes/BYTES_PER_SAMPLE;

		pb->pHeader->dwFlags = 0;
		pb->pHeader->lpData = (char *)pb->pData;
		pb->pHeader->dwBufferLength = self->_blockSizeBytes;
		pb->pHeader->dwFlags = 0;
		pb->pHeader->dwLoops = 0;

		if (waveOutPrepareHeader(self->_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			self->error(1, "waveOutPrepareHeader() failed");
		}
		pb->Prepared = TRUE;

		if (waveOutWrite(self->_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			self->error(1, "waveOutWrite() failed");
		}
		pb++;
		if (pb == self->_blocks + self->_numBlocks)
		{
			pb = self->_blocks;
		}
	}
	self->_currentBlock = pb - self->_blocks;
} 

int driver_close(Driver* driver)
{
	MmeDriver* self = (MmeDriver*) driver;
	unsigned int _numBlocks;
	CBlock *pBlock;
	// SingleLock lock(&_lock, TRUE);
	if(!self->_running) return TRUE;
	self->_stopPolling = TRUE;
	// CSingleLock event(&_event, TRUE);
	_numBlocks = self->_numBlocks;
	// Once we get here, the PollerThread should have stopped
	if(waveOutReset(self->_handle) != MMSYSERR_NOERROR)
	{
		self->error(1, "waveOutReset() failed");
		return FALSE;
	}
	for(;;)
	{
		BOOL alldone = TRUE;
		CBlock *pBlock;
		for(pBlock = self->_blocks; pBlock < self->_blocks + self->_numBlocks; pBlock++)
		{
			if((pBlock->pHeader->dwFlags & WHDR_DONE) == 0) alldone = FALSE;
		}
		if(alldone) break;
		Sleep(10);
	}
	for(pBlock = self->_blocks; pBlock < self->_blocks + self->_numBlocks; pBlock++)
	{
		if(pBlock->Prepared)
		{
			if(waveOutUnprepareHeader(self->_handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				self->error(1, "waveOutUnprepareHeader() failed");
			}
		}
	}
	if(waveOutClose(self->_handle) != MMSYSERR_NOERROR)
	{
		self->error(1, "waveOutClose() failed");
		return FALSE;
	}
	for(pBlock = self->_blocks; pBlock < self->_blocks + self->_numBlocks; pBlock++)
	{
		GlobalUnlock(pBlock->Handle);
		GlobalFree(pBlock->Handle);
		GlobalUnlock(pBlock->HeaderHandle);
		GlobalFree(pBlock->HeaderHandle);
	}
	/*
	for(unsigned int i=0; i<_capPorts.size(); i++)
	{
		if(_capPorts[i]._handle == NULL)
			continue;
		if(::waveInReset(_capPorts[i]._handle) != MMSYSERR_NOERROR)
		{
			Error("waveInReset() failed");
			return false;
		}
		///\todo: wait until WHDR_DONE like with waveout?
		for(CBlock *pBlock = _capPorts[i]._blocks; pBlock < _capPorts[i]._blocks + _numBlocks; pBlock++)
		{
			if(pBlock->Prepared)
			{
				if(::waveInUnprepareHeader(_capPorts[i]._handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				{
					Error("waveInUnprepareHeader() failed");
				}
			}
		}
		waveInClose(_capPorts[i]._handle);
		_capPorts[i]._handle = NULL;
		universalis::os::aligned_memory_dealloc(_capPorts[i].pleft);
		universalis::os::aligned_memory_dealloc(_capPorts[i].pright);
		for(CBlock *pBlock = _capPorts[i]._blocks; pBlock < _capPorts[i]._blocks + _numBlocks; pBlock++)
		{
			::GlobalUnlock(pBlock->Handle);
			::GlobalFree(pBlock->Handle);
			::GlobalUnlock(pBlock->HeaderHandle);
			::GlobalFree(pBlock->HeaderHandle);
		}
	}*/

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

