// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../audiodriver.h"
#include "../audiodriversettings.h"


#include <windows.h>
#include <mmsystem.h>
#include <ks.h>
#include <ksmedia.h>
#include <process.h>
#include <stdio.h>
#include <operations.h>
#include <quantize.h>
#include <hashtbl.h>

#include "../../detail/portable.h"

#undef KSDATAFORMAT_SUBTYPE_PCM
const GUID KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

#undef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 00000003, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

#define PSY_AUDIODRIVER_MME_GUID 0x0003

#define BYTES_PER_SAMPLE 4	// 2 * 16bits
#define SHORT_MIN	-32768
#define SHORT_MAX	32767

#define MAX_WAVEOUT_BLOCKS 8

typedef struct {
	HANDLE Handle;
	unsigned char* pData;
	WAVEHDR* pHeader;
	HANDLE HeaderHandle;
	int Prepared;
} CBlock;

typedef struct {
	char* portname;
	int idx;
} PortEnums;

INLINE void portenums_init(PortEnums* self)
{
	self->idx = 0;
	self->portname = strdup("");
}

INLINE void portenums_initall(PortEnums* self, int idx, const char* portname)
{
	self->idx = idx;
	self->portname = portname ? strdup(portname) : strdup("");
}

INLINE void portenums_dispose(PortEnums* self)
{
	self->idx = 0;
	free(self->portname);
}

INLINE bool portenums_isformatsupported(PortEnums* self, WAVEFORMATEXTENSIBLE* pwfx, bool isInput)
{
	if (isInput) {
		return MMSYSERR_NOERROR == waveInOpen(
			NULL,                 // ptr can be NULL for query 
			self->idx,            // the device identifier 
			(WAVEFORMATEX*) (pwfx),                 // defines requested format 
			(DWORD_PTR) NULL,                 // no callback 
			(DWORD_PTR) NULL,                 // no instance data 
			WAVE_FORMAT_QUERY);  // query only, do not open device 
	}
	else {
		return MMSYSERR_NOERROR == waveOutOpen(
			NULL,                 // ptr can be NULL for query 
			self->idx,            // the device identifier 
			(WAVEFORMATEX*) (pwfx),                 // defines requested format 
			(DWORD_PTR) NULL,                 // no callback 
			(DWORD_PTR) NULL,                 // no instance data 
			WAVE_FORMAT_QUERY);  // query only, do not open device 
	}
}

typedef struct {
	HWAVEIN _handle;
	int _idx;
	byte* pData;
	WAVEHDR* pHeader;
	HANDLE HeaderHandle;
	CBlock _blocks[MAX_WAVEOUT_BLOCKS];
	bool Prepared;
	float* pleft;
	float* pright;
	int _machinepos;
} PortCapt;

INLINE portcapt_init(PortCapt* self)
{
	self->Prepared = FALSE;
	self->pleft = 0;
	self->pright = 0;
	self->_handle = 0;
	self->_idx = 0;
	self->_machinepos = 0;
}

typedef struct {
	psy_AudioDriver driver;
	psy_AudioDriverSettings settings;
	psy_Property* configuration;
	HWAVEOUT _handle;
	int _deviceId;
	int _dither;
	size_t _currentBlock;
	int _running;
	int _stopPolling;	
	unsigned int pollSleep_;
	unsigned int _writePos;
	/// number of "wraparounds" to compensate the WaveOutGetPosition() call.
	int m_readPosWraps;
	/// helper variable to detect the previous wraps.
	int m_lastPlayPos;		
	CBlock _blocks[MAX_WAVEOUT_BLOCKS];	
	int (*error)(int, const char*);
	HANDLE hEvent;
	psy_List* _playEnums;	// PortEnums
	psy_List* _capEnums;	// PortEnums
	psy_List* _capPorts;	// PortCapt
	psy_Table _portMapping; // <int, int>
} MmeDriver;

static void driver_deallocate(psy_AudioDriver*);
static int driver_init(psy_AudioDriver*);
static int driver_open(psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_AudioDriver*);
static psy_dsp_big_hz_t samplerate(psy_AudioDriver*);
static const char* capturename(psy_AudioDriver*, int index);
static int numcaptures(psy_AudioDriver*);
static const char* playbackname(psy_AudioDriver*, int index);
static int numplaybacks(psy_AudioDriver*);
static bool start(MmeDriver*);
static bool stop(MmeDriver*);
static const psy_AudioDriverInfo* driver_info(psy_AudioDriver*);
static uintptr_t playposinsamples(psy_AudioDriver*);

static void preparewaveformat(WAVEFORMATEXTENSIBLE* wf, int channels, int sampleRate, int bits, int validBits);
static void PollerThread(void *pWaveOut);
static void DoBlocks(MmeDriver* self);
static void init_properties(psy_AudioDriver* self);
static int onerror(int err, const char* msg);

int onerror(int err, const char* msg)
{
	MessageBox(0, msg, "Windows Wave MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

static void readbuffers(MmeDriver*, int idx, float** pleft, float** pright,
	uintptr_t numsamples);
static void refreshavailableports(MmeDriver*);
static void enumerateplaybackports(MmeDriver*);
static void enumeratecaptureports(MmeDriver*);
static void clearplayenums(MmeDriver*);
static void clearcapenums(MmeDriver*);
static bool createcaptureport(MmeDriver*, PortCapt*);
static bool addcaptureport(MmeDriver*, int idx);
static bool removecaptureport(MmeDriver*, int idx);

static psy_AudioDriverVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_deallocate;		
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = samplerate;
		vtable.addcapture = (psy_audiodriver_fp_addcapture)addcaptureport;
		vtable.removecapture = (psy_audiodriver_fp_removecapture)removecaptureport;
		vtable.readbuffers = (psy_audiodriver_fp_readbuffers)readbuffers;
		vtable.capturename = (psy_audiodriver_fp_capturename)capturename;
		vtable.numcaptures = (psy_audiodriver_fp_numcaptures)numcaptures;
		vtable.playbackname = (psy_audiodriver_fp_playbackname)playbackname;
		vtable.numplaybacks = (psy_audiodriver_fp_numplaybacks)numplaybacks;
		vtable.playposinsamples = playposinsamples;
		vtable.info = (psy_audiodriver_fp_info)driver_info;
		vtable_initialized = 1;
	}
}

EXPORT psy_AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void)
{
	static psy_AudioDriverInfo info;

	info.guid = PSY_AUDIODRIVER_MME_GUID;
	info.Flags = 0;
	info.Name = "Windows MME AudioDriver";
	info.ShortName = "MME";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	MmeDriver* mme = (MmeDriver*) malloc(sizeof(MmeDriver));
	if (mme) {		
		driver_init(&mme->driver);
		return &mme->driver;
	} else {
		return 0;
	}
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver_dispose(driver);
	free(driver);
}

int driver_init(psy_AudioDriver* driver)
{
	MmeDriver* self = (MmeDriver*)driver;	

	memset(self, 0, sizeof(MmeDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	/*self->_deviceId = 0; // WAVE_MAPPER;
	self->_blockSizeBytes = 4096;
	self->pollSleep_ = 10;*/
	self->error = onerror;
	self->_deviceId = 0;
	self->pollSleep_ = 20;
	self->_dither = 0;
	self->_running = FALSE;	
//	self->_channelmode = 3;

#ifdef PSYCLE_USE_SSE
	psy_dsp_sse2_init(&dsp);
#else
	psy_dsp_noopt_init(&dsp);
#endif
	psy_audiodriversettings_init(&self->settings);
	psy_table_init(&self->_portMapping);
	refreshavailableports(self);
	init_properties(&self->driver);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	MmeDriver* self = (MmeDriver*) driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	CloseHandle(self->hEvent);
	psy_table_dispose(&self->_portMapping);
	clearplayenums(self);
	clearcapenums(self);
	return 0;
}

static void init_properties(psy_AudioDriver* driver)
{			
	MmeDriver* self;
	char key[256];
	psy_Property* devices;
	psy_Property* indevices;
	psy_List* p;
	int i;

	self = (MmeDriver*)driver;
	psy_snprintf(key, 256, "mme-guid-%d", PSY_AUDIODRIVER_MME_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_settext(
		psy_property_allocinit_key(key), "Windows Wave MME"));
	psy_property_sethint(psy_property_append_int(self->configuration,
		"guid", PSY_AUDIODRIVER_MME_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_settext(psy_property_setreadonly(
		psy_property_append_string(self->configuration, "name", "Windows Wave MME"),
		TRUE),
		"Driver");
	psy_property_settext(psy_property_setreadonly(
		psy_property_append_string(self->configuration, "vendor", "Psycledelics"),
		TRUE),
		"Vendor");
	psy_property_settext(psy_property_setreadonly(
		psy_property_append_string(self->configuration, "version", "1.0"),
		TRUE),
		"Version");
	psy_property_settext(
		psy_property_append_int(self->configuration, "bitdepth",
			psy_audiodriversettings_bitdepth(&self->settings), 0, 32),
		"Bitrate");
	psy_property_settext(
		psy_property_append_int(self->configuration, "samplerate",
			(intptr_t)psy_audiodriversettings_samplespersec(&self->settings),
				0, 0),
		"Samplerate"
		);
	psy_property_settext(
		psy_property_append_int(self->configuration, "dither", 0, 0, 1),
		"Dither");
	psy_property_settext(
		psy_property_append_int(self->configuration, "numbuf",
			psy_audiodriversettings_blockcount(&self->settings), 1, 8),
		"Buffer Number");
	psy_property_settext(
		psy_property_append_int(self->configuration, "numsamples",
			psy_audiodriversettings_blockframes(&self->settings),
			64, 8193),
		"Buffer Samples");
	devices = psy_property_settext(
		psy_property_append_choice(self->configuration, "device", 0),
		"Output Device");
	psy_property_append_int(devices, "WAVE_MAPPER", -1, 0, 0);
	for (p = self->_playEnums, i = 0; p != NULL; p = p->next, ++i) {
		PortEnums* port = (PortEnums*)p->entry;
		psy_property_append_int(devices, port->portname, i, 0, 0);
	}
	indevices = psy_property_settext(
		psy_property_append_choice(self->configuration, "indevice", 0),
		"Standard Input Device (Select different in Recorder)");
	for (p = self->_capEnums, i = 0; p != NULL; p = p->next, ++i) {
		PortEnums* port = (PortEnums*)p->entry;
		psy_property_append_int(indevices, port->portname, i, 0, 0);
	}
}

void driver_configure(psy_AudioDriver* driver, psy_Property* config)
{
	MmeDriver* self;
	psy_Property* property;

	self = (MmeDriver*)driver;
	if (self->configuration && config) {
		psy_property_sync(self->configuration, config);
	}
	property = psy_property_at(self->configuration, "device", PSY_PROPERTY_TYPE_NONE);
	if (property && property->item.typ == PSY_PROPERTY_TYPE_CHOICE) {
		psy_Property* device;

		device = psy_property_at_choice(property);
		if (device) {
			self->_deviceId =  psy_property_item_int(device);
		}
	}	
	psy_audiodriversettings_setvalidbitdepth(&self->settings,
		psy_property_at_int(self->configuration, "bitdepth",
			psy_audiodriversettings_validbitdepth(&self->settings)));
	psy_audiodriversettings_setsamplespersec(&self->settings,
		psy_property_at_int(self->configuration, "samplerate",
			(intptr_t)psy_audiodriversettings_samplespersec(&self->settings)));
	psy_audiodriversettings_setblockcount(&self->settings,
		psy_property_at_int(self->configuration, "numbuf",
			psy_audiodriversettings_blockcount(&self->settings)));
	psy_audiodriversettings_setblockframes(&self->settings,
		psy_property_at_int(self->configuration, "numsamples",
			psy_audiodriversettings_blockframes(&self->settings)));
}

psy_dsp_big_hz_t samplerate(psy_AudioDriver* self)
{
	return psy_audiodriversettings_samplespersec(&((MmeDriver*)self)->settings);
}

int driver_open(psy_AudioDriver* driver)
{
	return start((MmeDriver*)driver);
}

bool start(MmeDriver* self)
{
	WAVEFORMATEX format;
	CBlock* pBlock;
	unsigned int _blockSizeBytes;
	unsigned int _numBlocks;
	psy_List* pCapPort;

	//			CSingleLock lock(&_lock, TRUE);
	if (self->_running) return TRUE;
	if (self->driver.callback == NULL) {
		return FALSE;
	}
	preparewaveformat((WAVEFORMATEXTENSIBLE*)&format,
		psy_audiodriversettings_numchannels(&self->settings),
		(int)psy_audiodriversettings_samplespersec(&self->settings),
		psy_audiodriversettings_bitdepth(&self->settings),
		psy_audiodriversettings_validbitdepth(&self->settings));
	if (waveOutOpen(&self->_handle, self->_deviceId, &format, 0UL, 0,
		CALLBACK_NULL) != MMSYSERR_NOERROR) {
		self->error(0, "waveOutOpen() failed");
		return FALSE;
	}
	self->_currentBlock = 0;
	self->_writePos = 0;
	self->m_readPosWraps = 0;
	self->m_lastPlayPos = 0;

	// allocate blocks
	_blockSizeBytes = psy_audiodriversettings_blockbytes(&self->settings);
	_numBlocks = psy_audiodriversettings_blockcount(&self->settings);
	for (pBlock = self->_blocks; pBlock < self->_blocks + _numBlocks;
		++pBlock) {
		pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, _blockSizeBytes);
		if (pBlock->Handle) {
			pBlock->pData = (byte*)GlobalLock(pBlock->Handle);
		}
		else {
			pBlock->pData = 0;
		}
	}
	// allocate block headers
	for (pBlock = self->_blocks; pBlock < self->_blocks + _numBlocks; ++pBlock) {
		WAVEHDR* ph;

		pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
		if (pBlock->HeaderHandle) {
			pBlock->pHeader = (WAVEHDR*)GlobalLock(pBlock->HeaderHandle);

			ph = pBlock->pHeader;
			if (ph) {
				ph->lpData = (char*)pBlock->pData;
				ph->dwBufferLength = _blockSizeBytes;
				ph->dwFlags = WHDR_DONE;
				ph->dwLoops = 0;
			}
		}
		pBlock->Prepared = FALSE;
	}

	for (pCapPort = self->_capPorts; pCapPort != 0; pCapPort = pCapPort->next) {
		createcaptureport(self, (PortCapt*)pCapPort->entry);
	}
	self->_stopPolling = FALSE;
	ResetEvent(self->hEvent);
	_beginthread(PollerThread, 0, self);
	self->_running = TRUE;
	// PsycleGlobal::midi().ReSync();	// MIDI IMPLEMENTATION
	return TRUE;
}

void preparewaveformat(WAVEFORMATEXTENSIBLE* wf, int channels, int sampleRate, int bits, int validBits)
{
	// Set up wave format structure. 
	ZeroMemory(wf, sizeof(WAVEFORMATEX));
	wf->Format.nChannels = channels;
	wf->Format.wBitsPerSample = bits;
	wf->Format.nSamplesPerSec = sampleRate;
	wf->Format.nBlockAlign = wf->Format.nChannels * wf->Format.wBitsPerSample / 8;
	wf->Format.nAvgBytesPerSec = wf->Format.nSamplesPerSec * wf->Format.nBlockAlign;
	if (bits <= 16) {
		wf->Format.wFormatTag = WAVE_FORMAT_PCM;
		wf->Format.cbSize = 0;
	}
	else {
		wf->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wf->Format.cbSize = 0x16;
		wf->Samples.wValidBitsPerSample = validBits;
		if (channels == 2) {
			wf->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		}
		if (validBits == 32) {
			wf->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
		}
		else {
			wf->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		}
	}
}

void PollerThread(void* self)
{
	unsigned int _numBlocks;

	// universalis::os::thread_name thread_name("mme wave out");
	// universalis::cpu::exceptions::install_handler_in_thread();
	MmeDriver* pThis = (MmeDriver*)self;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	_numBlocks = psy_audiodriversettings_blockcount(&pThis->settings);
	while (!pThis->_stopPolling)
	{
		CBlock* pb = pThis->_blocks + pThis->_currentBlock;
		unsigned int underruns = 0;
		while (pb->pHeader->dwFlags & WHDR_DONE)
		{
			// for (unsigned int i = 0; i < pThis->_capPorts.size(); i++)
			// {
			//	if (pThis->_capPorts[i]._handle == NULL)
			//		continue;
			//	pThis->DoBlocksRecording(pThis->_capPorts[i]);
			// }
			DoBlocks(pThis);
			++pb;
			if (pb == pThis->_blocks + _numBlocks) pb = pThis->_blocks;
			if (pb->pHeader->dwFlags & WHDR_DONE) {
				++underruns;
				if (underruns > _numBlocks)
				{
					// Audio dropout most likely happened
					// (There's a possibility a dropout didn't happen, but the cpu usage
					// is almost at 100%, so we force an exit of the loop for a "Sleep()" call,
					// preventing psycle from being frozen.
					break;
				}
			}
			pThis->_currentBlock = pb - pThis->_blocks;
		}
		Sleep(pThis->pollSleep_);
	}
	SetEvent(pThis->hEvent);
	_endthread();
}

void DoBlocks(MmeDriver* self)
{
	CBlock* pb = self->_blocks + self->_currentBlock;
	int* pBlock;
	int numSamples;
	int n;
	unsigned int _sampleValidBits;
	int hostisplaying;
	float* pFloatBlock;

	if (pb->Prepared)
	{
		while (waveOutUnprepareHeader(self->_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
		}
		pb->Prepared = FALSE;
	}
	pBlock = (int*)pb->pData;
	numSamples = psy_audiodriversettings_blockframes(&self->settings);
	_sampleValidBits = psy_audiodriversettings_validbitdepth(&self->settings);
	n = numSamples;
	pFloatBlock = self->driver.callback(self->driver.callbackcontext, &n, &hostisplaying);
	if (_sampleValidBits == 32) {
		//dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pBlock), numSamples * 2, 1.f / 32768.f);
	}
	else if (_sampleValidBits == 24) {
		//Quantize24in32Bit(pFloatBlock, pBlock, numSamples);
	}
	else if (_sampleValidBits == 16) {
		//if (settings_->dither()) Quantize16WithDither(pFloatBlock, pBlock, numSamples);
		//else Quantize16(pFloatBlock, pBlock, numSamples);
		psy_dsp_quantize16(pFloatBlock, pBlock, numSamples);
	}

	self->_writePos += numSamples;

	pb->pHeader->lpData = (char*)pb->pData;
	pb->pHeader->dwBufferLength = psy_audiodriversettings_blockbytes(&self->settings);
	pb->pHeader->dwFlags = 0;
	pb->pHeader->dwLoops = 0;

	if (waveOutPrepareHeader(self->_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		self->error(0, "waveOutPrepareHeader() failed");
	}
	pb->Prepared = TRUE;
	if (waveOutWrite(self->_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		self->error(0, "waveOutWrite() failed");
	}
}

int driver_close(psy_AudioDriver* driver)
{
	int status;
	psy_List* pCapPort;
	MmeDriver* self = (MmeDriver*)driver;
	status = stop(self);
	for (pCapPort = self->_capPorts; pCapPort != 0; pCapPort = pCapPort->next) {
		PortCapt* portcap;

		portcap = (PortCapt*)pCapPort->entry;
		free(portcap);
	}
	psy_list_free(self->_capPorts);
	self->_capPorts = 0;
	return status;
}

bool stop(MmeDriver* self)
{
	size_t _numBlocks;
	CBlock* pBlock;
	psy_List* pCapPort;

	if (!self->_running) {
		return TRUE;
	}
	self->_stopPolling = TRUE;
	_numBlocks = psy_audiodriversettings_blockcount(&self->settings);
	WaitForSingleObject(self->hEvent, INFINITE);
	// Once we get here, the PollerThread should have stopped
	// or we hang in deadlock	
	if (waveOutReset(self->_handle) != MMSYSERR_NOERROR)
	{
		self->error(1, "waveOutReset() failed");
		return FALSE;
	}
	for (;;)
	{
		BOOL alldone = TRUE;
		CBlock* pBlock;
		for (pBlock = self->_blocks; pBlock < self->_blocks + _numBlocks; pBlock++)
		{
			if ((pBlock->pHeader->dwFlags & WHDR_DONE) == 0) alldone = FALSE;
		}
		if (alldone) break;
		Sleep(10);
	}
	for (pBlock = self->_blocks; pBlock < self->_blocks + _numBlocks; pBlock++)
	{
		if (pBlock->Prepared)
		{
			if (waveOutUnprepareHeader(self->_handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				self->error(1, "waveOutUnprepareHeader() failed");
			}
		}
	}
	if (waveOutClose(self->_handle) != MMSYSERR_NOERROR)
	{
		self->error(1, "waveOutClose() failed");
		return FALSE;
	}
	for (pBlock = self->_blocks; pBlock < self->_blocks + _numBlocks; pBlock++)
	{
		GlobalUnlock(pBlock->Handle);
		GlobalFree(pBlock->Handle);
		GlobalUnlock(pBlock->HeaderHandle);
		GlobalFree(pBlock->HeaderHandle);
	}
	for (pCapPort = self->_capPorts; pCapPort != 0; pCapPort = pCapPort->next) {
		PortCapt* portcap;

		portcap = (PortCapt*)pCapPort->entry;
		if (portcap->_handle == NULL)
			continue;
		if (waveInReset(portcap->_handle) != MMSYSERR_NOERROR)
		{
			self->error(0, "waveInReset() failed");
			return FALSE;
		}
		///\todo: wait until WHDR_DONE like with waveout?
		for (pBlock = portcap->_blocks; pBlock < portcap->_blocks + _numBlocks; pBlock++)
		{
			if (pBlock->Prepared)
			{
				if (waveInUnprepareHeader(portcap->_handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				{
					self->error(0, "waveInUnprepareHeader() failed");
				}
			}
		}
		waveInClose(portcap->_handle);
		portcap->_handle = NULL;
		dsp.memory_dealloc(portcap->pleft);
		dsp.memory_dealloc(portcap->pright);
		for (pBlock = portcap->_blocks; pBlock < portcap->_blocks + _numBlocks; pBlock++)
		{
			GlobalUnlock(pBlock->Handle);
			GlobalFree(pBlock->Handle);
			GlobalUnlock(pBlock->HeaderHandle);
			GlobalFree(pBlock->HeaderHandle);
		}
	}
	self->_running = FALSE;
	return TRUE;
}

void refreshavailableports(MmeDriver* self)
{
	enumerateplaybackports(self);
	enumeratecaptureports(self);
}

void enumerateplaybackports(MmeDriver* self)
{
	unsigned int i;

	clearplayenums(self);
	for (i = 0; i < waveOutGetNumDevs(); ++i) {
		PortEnums* port;
		WAVEOUTCAPS caps;
		waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS));
		port = (PortEnums*) malloc(sizeof(PortEnums));
		if (port) {
			portenums_initall(port, i, caps.szPname);
			psy_list_append(&self->_playEnums, port);
		}
	}
}

void enumeratecaptureports(MmeDriver* self)
{
	unsigned int i;

	clearcapenums(self);
	for (i = 0; i < waveInGetNumDevs(); ++i) {
		PortEnums* port;
		WAVEINCAPS caps;
		waveInGetDevCaps(i, &caps, sizeof(WAVEINCAPS));		
		port = (PortEnums*) malloc(sizeof(PortEnums));
		if (port) {
			portenums_initall(port, i, caps.szPname);
			psy_list_append(&self->_capEnums, port);
		}
	}
}

void clearplayenums(MmeDriver* self)
{
	psy_List* p;

	for (p = self->_playEnums; p != NULL; p = p->next) {
		PortEnums* port = (PortEnums*)p->entry;
		portenums_dispose(port);
		free(port);
	}
	psy_list_free(self->_playEnums);
	self->_playEnums = 0;
}

void clearcapenums(MmeDriver* self)
{
	psy_List* p;

	for (p = self->_capEnums; p != NULL; p = p->next) {
		PortEnums* port = (PortEnums*)p->entry;
		portenums_dispose(port);
		free(port);
	}
	psy_list_free(self->_capEnums);
	self->_capEnums = 0;
}

bool createcaptureport(MmeDriver* self, PortCapt* port)
{
	HRESULT hr;
	WAVEFORMATEX format;
	unsigned int _blockSizeBytes;
	unsigned int _numBlocks;
	CBlock* pBlock;

	//avoid opening a port twice
	if (port->_handle) {
		return TRUE;
	}
	preparewaveformat((WAVEFORMATEXTENSIBLE*)&format,
		psy_audiodriversettings_numchannels(&self->settings),
		(int)psy_audiodriversettings_samplespersec(&self->settings),
		psy_audiodriversettings_bitdepth(&self->settings),
		psy_audiodriversettings_validbitdepth(&self->settings));
	_blockSizeBytes = psy_audiodriversettings_blockbytes(&self->settings);
	_numBlocks = psy_audiodriversettings_blockcount(&self->settings);
	if ((hr = waveInOpen(&port->_handle, port->_idx, (LPWAVEFORMATEX)(&format),
		(DWORD_PTR)NULL, (DWORD_PTR)NULL, CALLBACK_NULL)) != MMSYSERR_NOERROR)
	{
		self->error(0, "waveInOpen() failed");
		return FALSE;
	}
	// allocate blocks
	for (pBlock = port->_blocks; pBlock < port->_blocks + _numBlocks; ++pBlock)
	{
		pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, _blockSizeBytes);
		if (pBlock->Handle) {
			pBlock->pData = (byte*) GlobalLock(pBlock->Handle);
		}
	}
	// allocate block headers
	for (pBlock = port->_blocks; pBlock < port->_blocks + _numBlocks; ++pBlock)
	{		
		pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
		if (pBlock->HeaderHandle) {
			pBlock->pHeader = (WAVEHDR*)GlobalLock(pBlock->HeaderHandle);
			if (pBlock->pHeader) {
				WAVEHDR* ph;
				ph = pBlock->pHeader;
				ph->lpData = (char*)pBlock->pData;
				ph->dwBufferLength = _blockSizeBytes;
				ph->dwFlags = WHDR_DONE;
				ph->dwLoops = 0;
			}
		}
		pBlock->Prepared = FALSE;
	}
	port->pleft = dsp.memory_alloc(_blockSizeBytes * sizeof(float), 1);
	port->pright = dsp.memory_alloc(_blockSizeBytes * sizeof(float), 1);

	ZeroMemory(port->pleft, 2 * _blockSizeBytes);
	ZeroMemory(port->pright, 2 * _blockSizeBytes);
	port->_machinepos = 0;

	waveInStart(port->_handle);

	return TRUE;
}

bool addcaptureport(MmeDriver* self, int idx)
{
	PortCapt* port;
	bool isplaying = self->_running;
	if (idx >= (int) psy_list_size(self->_capEnums)) return FALSE;
	if (idx < (int) psy_table_size(&self->_portMapping) && (intptr_t)psy_table_at(&self->_portMapping, idx) != -1) return TRUE;

	if (isplaying) stop(self);
	
	port = malloc(sizeof(PortCapt));
	if (port) {
		portcapt_init(port);
		port->_idx = idx;

		psy_list_append(&self->_capPorts, port);
		if ((int)psy_table_size(&self->_portMapping) <= idx) {
			int oldsize = psy_table_size(&self->_portMapping);
			int i;

			for (i = oldsize; i < idx + 1; i++) {
				psy_table_insert(&self->_portMapping, i, (void*)(intptr_t)i);
			}
		}
		psy_table_insert(&self->_portMapping, idx, (void*)(intptr_t)(psy_list_size(self->_capPorts) - 1));
		if (isplaying) return start(self);
	} else {
		return FALSE;
	}
	return TRUE;
}

bool removecaptureport(MmeDriver* self, int idx)
{
	bool isplaying = self->_running;
	int maxSize = 0;
	unsigned int i;
	psy_List* pCapPort;
	psy_List* newports;

	newports = 0;
	if (idx >= (int)psy_list_size(self->_capEnums) ||
			idx >= (int)psy_table_size(&self->_portMapping) ||
			(intptr_t)psy_table_at(&self->_portMapping, idx) == -1) {
		return FALSE;
	}
	if (isplaying) {
		stop(self);
	}
	for (i = 0; i < psy_table_size(&self->_portMapping); ++i) {
		if (i != idx && (intptr_t)psy_table_at(&self->_portMapping, i) != -1) {
			maxSize = i + 1;
			psy_list_append(&newports,
				psy_list_at(self->_capPorts,
				(intptr_t)psy_table_at(&self->_portMapping, i)));
			psy_table_insert(&self->_portMapping, i,
				(void*)(intptr_t)(psy_list_size(newports) - 1));
		}
	}
	psy_table_insert(&self->_portMapping, idx, (void*)(intptr_t) -1);
	// if (maxSize < (int) psy_table_size(&self->_portMapping)) {
	//	_portMapping.resize(maxSize);
	// }
	for (pCapPort = self->_capPorts; pCapPort != 0; pCapPort = pCapPort->next) {
		PortCapt* portcap;

		portcap = (PortCapt*)pCapPort->entry;
		free(portcap);
	}
	psy_list_free(self->_capPorts);
	self->_capPorts = newports;
	if (isplaying) {
		start(self);
	}
	return TRUE;
}

void readbuffers(MmeDriver* self, int idx, float** pleft, float** pright, uintptr_t numsamples)
{
	PortCapt* portcap;
	int mpos;

	if (!self->_running || idx >= (int)psy_table_size(&self->_portMapping) ||
		(intptr_t)psy_table_at(&self->_portMapping, idx) == -1 ||
			self->_capPorts == 0 ||
			((PortCapt*)psy_list_at(self->_capPorts,
			(intptr_t)psy_table_at(&self->_portMapping, idx))->entry)->_handle == NULL) {
		*pleft = 0;
		*pright = 0;
		return;
	}
	portcap = ((PortCapt*)psy_list_at(self->_capPorts,
		(intptr_t)psy_table_at(&self->_portMapping, idx))->entry);
	mpos = portcap->_machinepos;
	*pleft = portcap->pleft + mpos;
	*pright = portcap->pright + mpos;
	portcap->_machinepos += numsamples;
}

const char* capturename(psy_AudioDriver* driver, int index)
{
	MmeDriver* self = (MmeDriver*) driver;
	psy_List* pPort;

	if (self->_capEnums) {
		pPort = psy_list_at(self->_capEnums, index);
		if (pPort) {
			return ((PortEnums*)(pPort->entry))->portname;
		}
	}
	return "";
}

int numcaptures(psy_AudioDriver* driver)
{
	MmeDriver* self = (MmeDriver*)driver;

	return psy_list_size(self->_capEnums);
}

const char* playbackname(psy_AudioDriver* driver, int index)
{
	MmeDriver* self = (MmeDriver*)driver;
	psy_List* pPort;

	if (self->_playEnums) {
		pPort = psy_list_at(self->_playEnums, index);
		if (pPort) {
			return ((PortEnums*)(pPort->entry))->portname;
		}
	}
	return "";
}

int numplaybacks(psy_AudioDriver* driver)
{
	MmeDriver* self = (MmeDriver*)driver;
	
	return psy_list_size(self->_playEnums);
}

const psy_AudioDriverInfo* driver_info(psy_AudioDriver* self)
{
	return GetPsycleDriverInfo();
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	MmeDriver* self = (MmeDriver*)driver;

	return self->configuration;
}

uintptr_t playposinsamples(psy_AudioDriver* driver)
{
	uint32_t retval;
	MMTIME time;

	MmeDriver* self = (MmeDriver*)driver;
	// WARNING! waveOutGetPosition in TIME_SAMPLES has max of 0x7FFFFF for 16bit stereo signals.
	if (self->_stopPolling) {
		return 0;
	}
	time.wType = TIME_SAMPLES;
	if (waveOutGetPosition(self->_handle, &time, sizeof(MMTIME)) != MMSYSERR_NOERROR)
	{
		onerror(0, "waveOutGetPosition() failed");
	}
	if (time.wType != TIME_SAMPLES)
	{
		onerror(0, "waveOutGetPosition() doesn't support TIME_SAMPLES");
	}

	retval = time.u.sample;
	// sample counter wrap around?
	if (self->m_lastPlayPos > (int)retval)
	{
		self->m_readPosWraps++;
		if (self->m_lastPlayPos > (int)self->_writePos) {
			self->m_readPosWraps = 0;
			// PsycleGlobal::midi().ReSync();	// MIDI IMPLEMENTATION
		}
	}
	self->m_lastPlayPos = retval;
	return retval + (self->m_readPosWraps * 0x800000);
}
