// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../audiodriversettings.h"
#include "avrt.h"

#include <MMReg.h>
#define DIRECTSOUND_VERSION 0x8000
#include <ks.h>
#include <ksmedia.h>

#include <dsound.h>

// linking
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

// includes
#include <string.h>
#include "../audiodriver.h"
#include <stdio.h>
#include <quantize.h>
#include <operations.h>
#include <hashtbl.h>
#include "../../detail/portable.h"

static psy_dsp_Operations dsp;

#undef KSDATAFORMAT_SUBTYPE_PCM
const GUID KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

#undef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 00000003, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

#define PSY_AUDIODRIVER_DIRECTX_GUID 0x0002

#define BYTES_PER_SAMPLE 4	// 2 * 16bits
#define SHORT_MIN	-32768
#define SHORT_MAX	32767

static BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
static BOOL CALLBACK DSCaptureEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);

typedef struct {	
	char* portname;
	LPGUID guid;
} PortEnums;

INLINE void portenums_init(PortEnums* self)
{
	self->guid = 0;
	self->portname = strdup("");
}

INLINE void portenums_initall(PortEnums* self, LPGUID guid, const char* portname)
{
	self->guid = (guid == NULL) ? (GUID*) &DSDEVID_DefaultPlayback : guid;
	self->portname = portname ? strdup(portname) : strdup("");
}

INLINE void portenums_dispose(PortEnums* self)
{
	self->guid = 0;
	free(self->portname);
}

INLINE bool portenums_isformatsupported(WAVEFORMATEXTENSIBLE* pwfx, bool isInput)
{
	///\todo: Implement
	return TRUE;
}

typedef struct {
	LPGUID _pGuid;
	LPDIRECTSOUNDCAPTURE8 _pDs;
	LPDIRECTSOUNDCAPTUREBUFFER8  _pBuffer;
	int _lowMark;
	float* pleft;
	float* pright;
	int _machinepos;
} PortCapt;

INLINE portcapt_init(PortCapt* self)
{
	self->pleft = 0;
	self->pright = 0;
	self->_pGuid = 0;
	self->_pDs = 0;
	self->_pBuffer = 0;
	self->_lowMark = 0;
	self->_machinepos = 0;
}

typedef struct {		
	psy_AudioDriver driver;	
	psy_AudioDriverSettings settings;
	psy_Property* configuration;
	HWND m_hWnd;
	int _dither;	
	unsigned int pollSleep_;	
	
	int _initialized;
	int _configured;	
	// controls if the driver is supposed to be running or not
	int _running;
	// informs the real state of the DSound buffer (see the control of buffer play in DoBlocks())
	int _playing;
	// Controls if we want the thread to be running or not
	int _threadRun;	
	LPGUID device_guid_;
	int _deviceIndex;		
	int _currentOffset;
	uint32_t _dsBufferSize;
	uint32_t _lowMark;
	uint32_t _highMark;	
	int _buffersToDo;	
	/// number of "wraparounds" to compensate the GetCurrentPosition() call.
	int m_readPosWraps;
	psy_List* _playEnums;
	psy_List*  _capEnums;
	psy_List* _capPorts;
	psy_Table _portMapping; // <int, int>
	LPDIRECTSOUND8 _pDs;
	LPDIRECTSOUNDBUFFER8 _pBuffer;
	HANDLE hEvent;
	int (*error)(int, const char*);
} DXDriver;

static int driver_init(psy_AudioDriver*);
static int driver_open(psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_AudioDriver*);
static psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver*);
static const char* capturename(psy_AudioDriver*, int index);
static int numcaptures(psy_AudioDriver*);
static const char* playbackname(psy_AudioDriver*, int index);
static int numplaybacks(psy_AudioDriver*);
static int addcaptureport(DXDriver* self, int idx);
static int removecaptureport(DXDriver* self, int idx);
static bool start(DXDriver*);
static bool stop(DXDriver*);
static void driver_deallocate(psy_AudioDriver*);
static const psy_AudioDriverInfo* driver_info(psy_AudioDriver*);

static void preparewaveformat(WAVEFORMATEXTENSIBLE* wf, int channels, int sampleRate, int bits, int validBits);
static DWORD WINAPI notifythread(void* pDirectSound);
static DWORD WINAPI pollerthread(void* pDirectSound);
static void doblocksrecording(DXDriver*, PortCapt*);
static void doblocks(DXDriver*);
static void init_properties(psy_AudioDriver* self);
static int on_error(int err, const char* msg);
static BOOL isvistaorlater(void);
static BOOL wantsmoreblocks(DXDriver*);
static void refreshavailableports(DXDriver*);
static void clearplayenums(DXDriver*);
static void clearcapenums(DXDriver*);
static void clearcapports(DXDriver*);
static bool createcaptureport(DXDriver*, PortCapt* port);
static void readbuffers(DXDriver* self, int index,
	float** pleft, float** pright, uintptr_t numsamples);
static uintptr_t playposinsamples(psy_AudioDriver*);

static psy_AudioDriverVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_deallocate;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = driver_samplerate;
		vtable.addcapture = (psy_audiodriver_fp_addcapture)addcaptureport;
		vtable.removecapture = (psy_audiodriver_fp_removecapture)removecaptureport;
		vtable.readbuffers = (psy_audiodriver_fp_readbuffers)readbuffers;
		vtable.capturename = (psy_audiodriver_fp_capturename)capturename;
		vtable.numcaptures = (psy_audiodriver_fp_numcaptures)numcaptures;
		vtable.playbackname = (psy_audiodriver_fp_playbackname)playbackname;
		vtable.numplaybacks = (psy_audiodriver_fp_numplaybacks)numplaybacks;
		vtable.playposinsamples = playposinsamples;
		vtable.info = (psy_audiodriver_fp_info)driver_info;
		vtable_initialized = TRUE;
	}
}

int on_error(int err, const char* msg)
{
	MessageBox(0, msg, "DirectSound Audio Driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT psy_AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void)
{
	static psy_AudioDriverInfo info;

	info.guid = PSY_AUDIODRIVER_DIRECTX_GUID;
	info.Flags = 0;
	info.Name = "DirectSound";
	info.ShortName = "DXSound";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	DXDriver* directx;
	
	directx = (DXDriver*)malloc(sizeof(DXDriver));
	if (directx != NULL) {		
		driver_init(&directx->driver);
		return &directx->driver;
	}
	return NULL;
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver_dispose(driver);
	free(driver);
}

int driver_init(psy_AudioDriver* driver)
{
	DXDriver* self = (DXDriver*) driver;	

	memset(self, 0, sizeof(DXDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->error = on_error;
	self->_initialized = FALSE;
	self->_configured = FALSE;
	self->_running = FALSE;
	self->_playing = FALSE;	
	self->_pDs = NULL;
	self->_pBuffer = NULL;
	self->driver.callback = NULL;
	self->driver.callbackcontext = NULL;
	self->driver.handle = NULL;
	self->m_hWnd = 0;
	self->device_guid_ = (LPGUID)&DSDEVID_DefaultPlayback;
	self->_dither = 0;
	self->_playEnums = 0;
	self->_capEnums = 0;
	self->_capPorts = 0;
#ifdef PSYCLE_USE_SSE
	psy_dsp_sse2_init(&dsp);
#else
	psy_dsp_noopt_init(&dsp);
#endif
	SetupAVRT();
	psy_audiodriversettings_init(&self->settings);
	psy_table_init(&self->_portMapping);
	refreshavailableports(self);
	init_properties(&self->driver);
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	DXDriver* self = (DXDriver*) driver;

	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	CloseHandle(self->hEvent);
	psy_table_dispose(&self->_portMapping);
	clearplayenums(self);
	clearcapenums(self);
	CloseAVRT();
	return 0;
}

void refreshavailableports(DXDriver* self)
{
	clearplayenums(self);
	clearcapenums(self);
	DirectSoundEnumerate(DSEnumCallback, &self->_playEnums);
	DirectSoundCaptureEnumerate(DSCaptureEnumCallback, &self->_capEnums);
}

BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{	
	PortEnums* port;
	
	port = (PortEnums*)malloc(sizeof(PortEnums));
	if (port) {
		psy_List** ports;

		ports = (psy_List**)lpContext;
		portenums_initall(port, lpGuid, lpcstrDescription);		
		psy_list_append(ports, port);
	}
	return TRUE;
}
BOOL CALLBACK DSCaptureEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{	
	PortEnums* port;

	port = (PortEnums*)malloc(sizeof(PortEnums));
	if (port) {
		psy_List** ports;

		ports = (psy_List**)lpContext;
		portenums_initall(port, lpGuid, lpcstrDescription);
		psy_list_append(ports, port);
	}
	return TRUE;	
}

void clearplayenums(DXDriver* self)
{
	psy_List* p;

	for (p = self->_playEnums; p != NULL; p = p->next) {
		PortEnums* port = (PortEnums*) p->entry;
		portenums_dispose(port);
		free(port);
	}
	psy_list_free(self->_playEnums);
	self->_playEnums = 0;	
}

void clearcapenums(DXDriver* self)
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

static void init_properties(psy_AudioDriver* driver)
{	
	DXDriver* self = (DXDriver*)driver;
	char key[256];	
	psy_Property* devices;
	psy_Property* indevices;
	psy_List* p;
	int i;		

	psy_snprintf(key, 256, "directx-guid-%d", PSY_AUDIODRIVER_DIRECTX_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_settext(
		psy_property_allocinit_key(key), "DirectSound"));
	psy_property_sethint(		
		psy_property_append_int(self->configuration,
		"guid", PSY_AUDIODRIVER_DIRECTX_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_settext(
		psy_property_setreadonly(
			psy_property_append_string(self->configuration, "name", "DirectSound"),
				TRUE),
			"Name");
	psy_property_settext(
		psy_property_setreadonly(
		psy_property_append_string(self->configuration, "vendor", "Psycledelics"),
			TRUE),
			"Vendor");
	psy_property_settext(
		psy_property_setreadonly(
			psy_property_append_string(self->configuration, "version", "1.0"),
			TRUE),
			"Version");
	psy_property_settext(
		psy_property_append_int(self->configuration, "bitdepth",
			psy_audiodriversettings_bitdepth(&self->settings), 0, 32),
		"Bitdepth");
	psy_property_settext(
		psy_property_append_int(self->configuration, "samplerate",
			(intptr_t)psy_audiodriversettings_samplespersec(&self->settings),
				0, 0),
		"Samplerate");
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
	for (p = self->_playEnums, i = 0; p != NULL; psy_list_next(&p), ++i) {
		PortEnums* port = (PortEnums*)psy_list_entry(p);
		psy_property_append_int(devices, port->portname, i, 0, 0);
	}
	indevices = psy_property_settext(
		psy_property_append_choice(self->configuration, "indevice", 0),
		"Standard Input Device (Select different in Recorder)");
	for (p = self->_capEnums, i = 0; p != NULL; psy_list_next(&p), ++i) {
		PortEnums* port = (PortEnums*)psy_list_entry(p);
		psy_property_append_int(indevices, port->portname, i, 0, 0);
	}
}

void driver_configure(psy_AudioDriver* driver, psy_Property* config)
{
	DXDriver* self;
	psy_Property* property;

	self = (DXDriver*)driver;
	if (self->configuration && config) {
		psy_property_sync(self->configuration, config);
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
	property = psy_property_at(self->configuration, "device",
		PSY_PROPERTY_TYPE_CHOICE);
	if (property) {
		int playenumindex;
		psy_List* portnode;
		
		portnode = NULL;
		playenumindex = psy_property_item_int(property);
		if (playenumindex >= 0) {
			portnode = psy_list_at(self->_playEnums, playenumindex);
			if (portnode) {
				PortEnums* port;

				port = (PortEnums*)psy_list_entry(portnode);
				if (port) {
					self->device_guid_ = port->guid;
				}
			}
		}
		if (!portnode) {
			self->device_guid_ = (LPGUID)&DSDEVID_DefaultPlayback;
		}
	}
}

psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver* self)
{
	return psy_audiodriversettings_samplespersec(&((DXDriver*)self)->settings);
}

int driver_open(psy_AudioDriver* driver)
{	
	return start((DXDriver*)driver);
}

bool start(DXDriver* self)
{	
	DSBUFFERDESC desc;
	// WAVEFORMATPCMEX format;
	WAVEFORMATEX format;
	LPDIRECTSOUNDBUFFER pBufferGen;
	HRESULT hr;
	DWORD dwThreadId;

	//CSingleLock lock(&_lock, TRUE);	
	if (self->_running) {
		return TRUE;
	}
	self->m_hWnd = self->driver.handle;
	if (self->m_hWnd == NULL) {
		return FALSE;
	}
	if (self->driver.callback == NULL) {
		return FALSE;
	}
	if (hr = FAILED(DirectSoundCreate8(self->device_guid_, &self->_pDs, NULL)))
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
	self->_dsBufferSize = psy_audiodriversettings_totalbufferbytes(&self->settings);
	preparewaveformat((WAVEFORMATEXTENSIBLE*)&format,
		psy_audiodriversettings_numchannels(&self->settings),
		(int)psy_audiodriversettings_samplespersec(&self->settings),
		psy_audiodriversettings_bitdepth(&self->settings),
		psy_audiodriversettings_validbitdepth(&self->settings));
	ZeroMemory(&desc, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	desc.dwBufferBytes = self->_dsBufferSize;
	desc.dwReserved = 0;
	desc.lpwfxFormat = (LPWAVEFORMATEX) &format;
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
	ResetEvent(self->hEvent);
	self->_threadRun = TRUE;
	self->_playing = FALSE;	
#define DIRECTSOUND_POLLING P
#ifdef DIRECTSOUND_POLLING
	CreateThread(NULL, 0, pollerthread, self, 0, &dwThreadId);
#else
	CreateThread(NULL, 0, notifythread, self, 0, &dwThreadId);
#endif
	self->_running = TRUE;
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
	if(bits <= 16) {
		wf->Format.wFormatTag = WAVE_FORMAT_PCM;
		wf->Format.cbSize = 0;
	} else {
		wf->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wf->Format.cbSize = 0x16;
		wf->Samples.wValidBitsPerSample = validBits;
		if (channels == 2) {
			wf->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		}
		if (validBits == 32) {
			wf->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
		} else {
			wf->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		}
	}
}

DWORD WINAPI pollerthread(void* self)
{	
	DXDriver* pThis = (DXDriver*)self;
	HANDLE hTask = NULL;
	unsigned int i;
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
	pThis->_highMark = psy_audiodriversettings_blockbytes(&pThis->settings);
	pThis->_currentOffset = 0;
	for (i = 0; i < psy_audiodriversettings_blockcount(&pThis->settings); ++i) {
		// Directsound playback buffer is started here.
		doblocks(pThis);
	}
	while (pThis->_threadRun)
	{
		unsigned int runs = 0;

		while (wantsmoreblocks(pThis))
		{
			// First, run the capture buffers so that audio is available to wavein machines.
			
			// Next, proceeed with the generation of audio
			doblocks(pThis);
			if (++runs > psy_audiodriversettings_blockcount(&pThis->settings))
				break;
		}
		Sleep(1);
	}	
	SetEvent(pThis->hEvent);	
	return 0;
}

DWORD WINAPI notifythread(void* self)
{
	return 0;
}

BOOL wantsmoreblocks(DXDriver* self)
{
	// [_lowMark,_highMark) is the next buffer to be filled.
	// if (play) pos is still inside, we have to wait.
	uint32_t pos = 0;
	HRESULT hr;
	
	hr = IDirectSoundBuffer8_GetCurrentPosition(self->_pBuffer, &pos, 0);
	if (FAILED(hr)) return FALSE;
	if (self->_lowMark <= pos && pos < self->_highMark)	return FALSE;
	return TRUE;
}

void doblocksrecording(DXDriver* self, PortCapt* port)
{
	int* pBlock1, * pBlock2;
	unsigned long blockSize1, blockSize2;
	HRESULT hr;

	//If directsound capture fails
	if (port->_pBuffer == NULL) {
		return;
	}		
	hr = IDirectSoundBuffer_Lock(
		self->_pBuffer,
		(DWORD)self->_lowMark,
		(DWORD)psy_audiodriversettings_blockbytes(&self->settings),
		(void**)&pBlock1, (DWORD*)&blockSize1,
		(void**)&pBlock2, (DWORD*)&blockSize2,
		0);
	if (SUCCEEDED(hr)) {
		// Put the audio in our float buffers.
		unsigned int _sampleValidBits = psy_audiodriversettings_validbitdepth(&self->settings);
		int numSamples = blockSize1 / psy_audiodriversettings_framebytes(&self->settings);				
		if (numSamples > 0) {
			if (_sampleValidBits == 32) {
				// DeinterlaceFloat(reinterpret_cast<float*>(pBlock1), port.pleft, port.pright, numSamples);
			}
			else if (_sampleValidBits == 24) {
				// DeQuantize32AndDeinterlace(pBlock1, port.pleft, port.pright, numSamples);
			}
			else {
				psy_dsp_dequantize16anddeinterlace((short int*)(pBlock1), port->pleft, port->pright, numSamples);				
			}
		}
		port->_lowMark += blockSize1;
		if (blockSize2 > 0)
		{
			numSamples = blockSize2 / psy_audiodriversettings_framebytes(&self->settings);
			if (_sampleValidBits == 32) {
				// DeinterlaceFloat(reinterpret_cast<float*>(pBlock2), port.pleft + numSamples, port.pright + numSamples, numSamples);
			}
			else if (_sampleValidBits == 24) {
				// DeQuantize32AndDeinterlace(pBlock2, port.pleft + numSamples, port.pright + numSamples, numSamples);
			}
			else {
				psy_dsp_dequantize16anddeinterlace((short int*)(pBlock2), port->pleft + numSamples, port->pright + numSamples, numSamples);
			}
			port->_lowMark += blockSize2;
		}
		// Release the data back to DirectSound. 
		// Release the data back to DirectSound. 		
		hr = IDirectSoundBuffer_Unlock(self->_pBuffer, pBlock1, blockSize1, pBlock2, blockSize2);
		if (port->_lowMark >= (int) self->_dsBufferSize) port->_lowMark = 0;
	}
	port->_machinepos = 0;
}

void doblocks(DXDriver* self)
{
	int* pBlock1, * pBlock2;
	unsigned long blockSize1, blockSize2;
	HRESULT hr;
	
	hr = IDirectSoundBuffer_Lock(
		self->_pBuffer,
		(DWORD) self->_lowMark,
		(DWORD) psy_audiodriversettings_blockbytes(&self->settings),
		(void**) &pBlock1, (DWORD*)&blockSize1,
		(void**) &pBlock2, (DWORD*)&blockSize2,
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
			(DWORD)self->_lowMark,
			(DWORD)psy_audiodriversettings_blockbytes(&self->settings),
			(void**)&pBlock1, (DWORD*)&blockSize1,
			(void**)&pBlock2, (DWORD*)&blockSize2,
			0);
	}
	if (SUCCEEDED(hr))
	{
		// Generate audio and put it into the buffer
		unsigned int _sampleValidBits = psy_audiodriversettings_validbitdepth(&self->settings);
		int numSamples = blockSize1 / psy_audiodriversettings_framebytes(&self->settings);
		int hostisplaying;
		float* pFloatBlock =
			self->driver.callback(
				self->driver.callbackcontext, &numSamples, &hostisplaying);			
		if (_sampleValidBits == 32) {			
			// dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pBlock1), numSamples * 2, 1.f / 32768.f);
		}
		else if (_sampleValidBits == 24) {
			// Quantize24in32Bit(pFloatBlock, pBlock1, numSamples);
		}
		else if (_sampleValidBits == 16) {
			psy_dsp_quantize16(pFloatBlock, pBlock1, numSamples);
			// if (settings_->dither()) Quantize16WithDither(pFloatBlock, pBlock1, numSamples);
			// else Quantize16(pFloatBlock, pBlock1, numSamples);
		}
		self->_lowMark += blockSize1;
		if (blockSize2 > 0)
		{
			float* pFloatBlock;
			numSamples = blockSize2 / psy_audiodriversettings_framebytes(&self->settings);
			pFloatBlock = self->driver.callback(
					self->driver.callbackcontext, &numSamples, &hostisplaying);
				
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
				psy_dsp_quantize16(pFloatBlock, pBlock2, numSamples);
			}
			self->_lowMark += blockSize2;
		}
		// Release the data back to DirectSound. 		
		hr = IDirectSoundBuffer_Unlock(self->_pBuffer, pBlock1, blockSize1, pBlock2, blockSize2);
		if (self->_lowMark >= self->_dsBufferSize) {
			self->_lowMark -= self->_dsBufferSize;
			self->m_readPosWraps++;
#if _MSC_VER > 1200
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
		self->_highMark = self->_lowMark + 
			psy_audiodriversettings_blockbytes(&self->settings);
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
	psy_List* pCapPort;
	int status;
	DXDriver* self;
	
	self = (DXDriver*)driver;
	status = stop((DXDriver*)driver);
	for (pCapPort = self->_capPorts; pCapPort != 0; pCapPort = pCapPort->next) {
		PortCapt* portcap;

		portcap = (PortCapt*)pCapPort->entry;
		free(portcap);
	}
	psy_list_free(self->_capPorts);
	self->_capPorts = 0;
	return status;
}

bool stop(DXDriver* self)
{
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

BOOL isvistaorlater(void)
{
#if _MSC_VER > 1200
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
bool createcaptureport(DXDriver* self, PortCapt* port)
{
	HRESULT hr;
//	WAVEFORMATPCMEX wf;
	WAVEFORMATEX wf;
	DSCBUFFERDESC dscbd;
	LPDIRECTSOUNDCAPTUREBUFFER cb;

	// avoid opening a port twice
	if (port->_pDs) return TRUE;
	port->_machinepos = 0;
	// Create IDirectSoundCapture using the selected capture device
	if (FAILED(hr = DirectSoundCaptureCreate8(port->_pGuid, &port->_pDs, NULL))) {
		self->error(0, "Failed to create Capture DirectSound Device");
		return FALSE;
	}
	// Create the capture buffer	
	preparewaveformat((WAVEFORMATEXTENSIBLE*)&wf,
		psy_audiodriversettings_numchannels(&self->settings),
		(int)psy_audiodriversettings_samplespersec(&self->settings),
		psy_audiodriversettings_bitdepth(&self->settings),
		psy_audiodriversettings_validbitdepth(&self->settings));	
	ZeroMemory(&dscbd, sizeof(DSCBUFFERDESC));
	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwBufferBytes = self->_dsBufferSize;
	dscbd.lpwfxFormat = (LPWAVEFORMATEX)(&wf);	
	if (FAILED(hr = IDirectSoundCapture_CreateCaptureBuffer(port->_pDs, &dscbd,
			&cb, NULL))) {
		self->error(0, "Failed to create Capture DirectSound Buffer");
		if (port->_pDs) {
			IDirectSoundCapture_Release(port->_pDs);
			port->_pDs = 0;
		}
		return FALSE;
	}
	if (FAILED(hr = IDirectSoundCapture_QueryInterface(cb,
			&IID_IDirectSoundCaptureBuffer8, (void**)&port->_pBuffer))) {
		self->error(0, "Failed to create Interface for Capture DirectSound Buffer(s)");
		if (cb) {
			IDirectSoundCaptureBuffer_Release(cb);
			cb = 0;
		}
		if (port->_pDs) {
			IDirectSoundCapture_Release(port->_pDs);
			port->_pDs = 0;
		}
		return FALSE;
	}
	// 2* is a safety measure (Haven't been able to dig out why it crashes if it is exactly the size)
	port->pleft = dsp.memory_alloc(2 * psy_audiodriversettings_blockbytes(&self->settings), 1);
	port->pright = dsp.memory_alloc(2 * psy_audiodriversettings_blockbytes(&self->settings), 1);
	dsp.clear(port->pleft, psy_audiodriversettings_blockbytes(&self->settings));
	dsp.clear(port->pright, psy_audiodriversettings_blockbytes(&self->settings));	
	return TRUE;
}

int addcaptureport(DXDriver* self, int idx)
{
	PortCapt* port;
	bool isplaying = self->_running;
	if (idx >= (int)psy_list_size(self->_capEnums)) return FALSE;
	if (idx < (int)psy_table_size(&self->_portMapping) && (intptr_t)psy_table_at(&self->_portMapping, idx) != -1) return TRUE;

	if (isplaying) {
		stop(self);
	}

	port = malloc(sizeof(PortCapt));
	if (port) {
		portcapt_init(port);
		port->_pGuid = ((PortEnums*) psy_list_at(self->_capEnums, idx)->entry)->guid;

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
	}
	else {
		return FALSE;
	}
	return TRUE;
}

int removecaptureport(DXDriver* self, int idx)
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
	psy_table_insert(&self->_portMapping, idx, (void*)(intptr_t)-1);
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

void readbuffers(DXDriver* self, int idx, float** pleft, float** pright,
	uintptr_t numsamples)
{
	PortCapt* portcap;
	int mpos;

	if (!self->_running || idx >= (int)psy_table_size(&self->_portMapping) ||
		(intptr_t)psy_table_at(&self->_portMapping, idx) == -1 ||
		self->_capPorts == 0 ||
		((PortCapt*)psy_list_at(self->_capPorts,
		(intptr_t)psy_table_at(&self->_portMapping, idx))->entry)->_pDs == NULL) {
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
	DXDriver* self = (DXDriver*)driver;
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
	DXDriver* self = (DXDriver*)driver;

	return psy_list_size(self->_capEnums);
}

const char* playbackname(psy_AudioDriver* driver, int index)
{
	DXDriver* self = (DXDriver*)driver;
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
	DXDriver* self = (DXDriver*)driver;

	return psy_list_size(self->_playEnums);
}

const psy_AudioDriverInfo* driver_info(psy_AudioDriver* self)
{
	return GetPsycleDriverInfo();
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	DXDriver* self = (DXDriver*)driver;

	return self->configuration;
}

uintptr_t playposinsamples(psy_AudioDriver* driver)
{
	DXDriver* self = (DXDriver*)driver;
	HRESULT hr;
	DWORD playPos, writePos;

	//http://msdn.microsoft.com/en-us/library/ee418744%28v=VS.85%29.aspx
	//When a buffer is created, the play cursor is set to 0. As the buffer is played,
	//the cursor moves and always points to the next byte of data to be played.
	//When the buffer is stopped, the cursor remains where it is.
	if (!self->_threadRun) return 0;
	
	hr = IDirectSoundBuffer8_GetCurrentPosition(self->_pBuffer, &playPos, &writePos);
	if (FAILED(hr))
	{
		on_error(0, "DirectSoundBuffer::GetCurrentPosition failed");
		return 0;
	}
	if (playPos < writePos && self->_lowMark > writePos) {
		return (playPos + self->m_readPosWraps * self->_dsBufferSize) /
			psy_audiodriversettings_framebytes(&self->settings);
	} else {
		return (playPos + (self->m_readPosWraps - 1) * self->_dsBufferSize) /
			psy_audiodriversettings_framebytes(&self->settings);
	}
}
