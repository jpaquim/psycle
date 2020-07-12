// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// linking
#include "../../detail/prefix.h"
#include "../audiodriversettings.h"
#include "avrt.h"
#include <quantize.h>
#include <operations.h>

#include "../driver.h"
#include "../../detail/psydef.h"

#include <windows.h>
#include <stdio.h>
#include <hashtbl.h>

#include <ks.h>
#include <ksmedia.h>

#undef KSDATAFORMAT_SUBTYPE_PCM
const GUID KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

#undef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 00000003, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

// WASAPI
#include <mmreg.h>  // must be before other Wasapi headers
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define COBJMACROS
#include <Audioclient.h>
#include <endpointvolume.h>
#define INITGUID // Avoid additional linkage of static libs, excessive code will be optimized out by the compiler
#include <mmdeviceapi.h>
#include <functiondiscoverykeys.h>
#include <devicetopology.h>	// Used to get IKsJackDescription interface
#undef INITGUID
#endif

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { goto Exit; }


/* __uuidof is only available in C++, so we hard-code the GUID values for all
 * these. This is ok because these are ABI. */
const CLSID CLSID_MMDeviceEnumerator = { 0xbcde0395, 0xe52f, 0x467c,
  {0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e}
};

const IID IID_IMMDeviceEnumerator = { 0xa95664d2, 0x9614, 0x4f35,
  {0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6}
};

const IID IID_IMMEndpoint = { 0x1be09788, 0x6894, 0x4089,
  {0x85, 0x86, 0x9a, 0x2a, 0x6c, 0x26, 0x5a, 0xc5}
};

const IID IID_IAudioClient = { 0x1cb9ad4c, 0xdbfa, 0x4c32,
  {0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2}
};

const IID IID_IAudioClient3 = { 0x7ed4ee07, 0x8e67, 0x4cd4,
  {0x8c, 0x1a, 0x2b, 0x7a, 0x59, 0x87, 0xad, 0x42}
};

const IID IID_IAudioClock = { 0xcd63314f, 0x3fba, 0x4a1b,
  {0x81, 0x2c, 0xef, 0x96, 0x35, 0x87, 0x28, 0xe7}
};

const IID IID_IAudioCaptureClient = { 0xc8adbd64, 0xe71e, 0x48a0,
  {0xa4, 0xde, 0x18, 0x5c, 0x39, 0x5c, 0xd3, 0x17}
};

const IID IID_IAudioRenderClient = { 0xf294acfc, 0x3146, 0x4483,
  {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}
};


#define BYTES_PER_SAMPLE 4	// 2 * 16bits
#define SHORT_MIN	-32768
#define SHORT_MAX	32767

#define MAX_WAVEOUT_BLOCKS 8

#define MAX_STR_LEN 512

#define _CRT_SECURE_NO_WARNINGS

interface IMMDevice;
interface IMMDeviceEnumerator;
interface IMMDeviceCollection;

typedef struct {			
	HANDLE Handle;
	unsigned char *pData;
	WAVEHDR *pHeader;
	HANDLE HeaderHandle;
	int Prepared;
} CBlock;

static DWORD WINAPI EventAudioThread(void* pWasapi);

typedef struct {	
	// from GetId
	WCHAR szDeviceID[MAX_STR_LEN];
	// from PropVariant
	char portName[MAX_STR_LEN];

	REFERENCE_TIME DefaultDevicePeriod;
	REFERENCE_TIME MinimumDevicePeriod;

	WAVEFORMATEXTENSIBLE MixFormat;
	bool isDefault;
} PortEnum;

typedef struct PaWasapiSubStream
{
	// Device
	WCHAR szDeviceID[MAX_STR_LEN];
	IMMDevice* device;
	IAudioClient* client;
	WAVEFORMATEXTENSIBLE wavex;
	UINT32               bufferFrameCount;
	REFERENCE_TIME       device_latency;
	REFERENCE_TIME       period;
	AUDCLNT_SHAREMODE    shareMode;
	UINT32               streamFlags; // AUDCLNT_STREAMFLAGS_EVENTCALLBACK, ...
	UINT32               flags;
	float* pleft;
	float* pright;
	int _machinepos;
}
PaWasapiSubStream;

typedef struct {
	psy_AudioDriver driver;
	psy_AudioDriverSettings settings;
	HWAVEOUT _handle;
	int _deviceId;
	int _dither;

	size_t _currentBlock;
	int _running;
	int _stopPolling;
	unsigned int _writePos;
	/// number of "wraparounds" to compensate the WaveOutGetPosition() call.
	int m_readPosWraps;
	/// helper variable to detect the previous wraps.
	int m_lastPlayPos;		
	CBlock _blocks[MAX_WAVEOUT_BLOCKS];	
	int (*error)(int, const char*);
	HANDLE hEvent;

	bool shared;
	WCHAR szDeviceID[MAX_STR_LEN];
	bool _initialized;
	bool _configured;
	psy_List* _playEnums;
	psy_List* _capEnums;
	psy_List* _capPorts;
	psy_Table _portMapping; // <int, int>
	// output
	PaWasapiSubStream out;
	IAudioClock* pAudioClock;
	UINT64 audioClockFreq;
	uint32_t writeMark;

	// must be volatile to avoid race condition on user query while
	// thread is being started
	volatile bool running;

	DWORD dwThreadId;
} WasapiDriver;

static void RefreshPorts(WasapiDriver*, IMMDeviceEnumerator* pEnumerator);
static void FillPortList(WasapiDriver*, psy_List** portList, IMMDeviceCollection* pCollection, LPWSTR defaultID);
static const char* GetError(HRESULT hr);
static HRESULT DoBlock(WasapiDriver*, IAudioRenderClient* pRenderClient, int numFramesAvailable);
static HRESULT DoBlockRecording(WasapiDriver*, PaWasapiSubStream* port, IAudioCaptureClient* pCaptureClient, int numFramesAvailable);
static void GetPlaybackPorts(WasapiDriver*, psy_List** ports);
static void GetCapturePorts(WasapiDriver*, psy_List** ports);
static void GetReadBuffers(WasapiDriver*, int idx, float** pleft, float** pright, int numsamples);
static uint32_t GetWritePosInSamples(WasapiDriver*);
static uint32_t GetPlayPosInSamples(WasapiDriver*);
static HRESULT GetStreamFormat(WasapiDriver*, PaWasapiSubStream* stream, WAVEFORMATEXTENSIBLE* wfOut);
static uint32_t GetOutputLatencyMs(WasapiDriver*);
static uint32_t GetInputLatencySamples(WasapiDriver*);
static uint32_t GetOutputLatencySamples(WasapiDriver*);
static uint32_t GetIdxFromDevice(WasapiDriver*, WCHAR* szDeviceID);
static bool AddCapturePort(WasapiDriver*, int idx);
static bool RemoveCapturePort(WasapiDriver*, int idx);
static HRESULT CreateCapturePort(WasapiDriver*, IMMDeviceEnumerator* pEnumerator, PaWasapiSubStream* port);
static void FreePorts(psy_List* ports);
static bool start(WasapiDriver*);
static bool stop(WasapiDriver*);
static unsigned int GetBufferSamples(WasapiDriver*);

static void driver_deallocate(psy_AudioDriver*);
static int driver_init(psy_AudioDriver*);
static void driver_connect(psy_AudioDriver*, void* context, AUDIODRIVERWORKFN callback, void* handle);
static int driver_open(psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, psy_Properties*);
static uintptr_t samplerate(psy_AudioDriver*);
static const char* capturename(psy_AudioDriver*, int index);
static int numcaptures(psy_AudioDriver*);
static const char* playbackname(psy_AudioDriver*, int index);
static int numplaybacks(psy_AudioDriver*);
static int addcaptureport(WasapiDriver*, int idx);
static int removecaptureport(WasapiDriver*, int idx);

static void PrepareWaveFormat(WAVEFORMATEXTENSIBLE* wf, int channels, int sampleRate, int bits, int validBits);
static void init_properties(psy_AudioDriver* self);
static int on_error(int err, const char* msg);

static void refreshavailableports(WasapiDriver*);
static void clearplayenums(WasapiDriver*);
static void clearcapenums(WasapiDriver*);

// ------------------------------------------------------------------------------------------
// Aligns v backwards
static INLINE UINT32 ALIGN_BWD(UINT32 v, UINT32 align)
{
	return ((v - (align ? v % align : 0)));
}
// ------------------------------------------------------------------------------------------
static INLINE double nano100ToMillis(const REFERENCE_TIME* ref)
{
	// 1 nano = 0.000000001 seconds
	//100 nano = 0.0000001 seconds
	//100 nano = 0.0001 milliseconds
	return ((double)*ref) * 0.0001;
}
// ------------------------------------------------------------------------------------------
static INLINE REFERENCE_TIME MillisTonano100(const double* ref)
{
	// 1 nano = 0.000000001 seconds
	//100 nano = 0.0000001 seconds
	//100 nano = 0.0001 milliseconds
	return (REFERENCE_TIME)(*ref / 0.0001);
}
// ------------------------------------------------------------------------------------------
// Makes Hns period from frames and sample rate
static INLINE REFERENCE_TIME MakeHnsPeriod(UINT32 nFrames, DWORD nSamplesPerSec)
{
	return (REFERENCE_TIME)((10000.0 * 1000 / nSamplesPerSec * nFrames) + 0.5);
}
// ------------------------------------------------------------------------------------------
// Aligns WASAPI buffer to 128 byte packet boundary. HD Audio will fail to play if buffer
// is misaligned. This problem was solved in Windows 7 were AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED
// is thrown although we must align for Vista anyway.
static INLINE UINT32 AlignFramesPerBuffer(UINT32 nFrames, UINT32 nSamplesPerSec, UINT32 nBlockAlign)
{
#define HDA_PACKET_SIZE 128

	long packets_total = 10000 * (nSamplesPerSec * nBlockAlign / HDA_PACKET_SIZE);
	long frame_bytes = nFrames * nBlockAlign;

	if (frame_bytes % HDA_PACKET_SIZE != 0) {
		//Ensure we round up, not down
		frame_bytes = ALIGN_BWD(frame_bytes + HDA_PACKET_SIZE, HDA_PACKET_SIZE);
	}

	// align to packet size
	nFrames = frame_bytes / nBlockAlign;
	long packets = frame_bytes / HDA_PACKET_SIZE;

	// align to packets count
	while (packets && ((packets_total % packets) != 0))
	{
		//round up
		++packets;
	}

	frame_bytes = packets * HDA_PACKET_SIZE;
	nFrames = frame_bytes / nBlockAlign;

	return nFrames;
}
// ------------------------------------------------------------------------------------------
// Converts Hns period into number of frames
static INLINE UINT32 MakeFramesFromHns(REFERENCE_TIME hnsPeriod, UINT32 nSamplesPerSec)
{
	UINT32 nFrames = (UINT32)( // frames =
		1.0 * hnsPeriod * // hns *
		nSamplesPerSec / // (frames / s) /
		1000 / // (ms / s) /
		10000 // (hns / s) /
		+ 0.5 // rounding
		);
	return nFrames;
}

bool IsFormatSupported(PortEnum* self, WAVEFORMATEXTENSIBLE* pwfx, AUDCLNT_SHAREMODE sharemode)
{
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* device = NULL;
	IAudioClient* client = NULL;
	bool issuccess = FALSE;
	HRESULT hr;
	WAVEFORMATEXTENSIBLE* bla = NULL;

	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
		&IID_IMMDeviceEnumerator, (void**)&pEnumerator);

	EXIT_ON_ERROR(hr)
		hr = IMMDeviceEnumerator_GetDevice(pEnumerator, self->szDeviceID, &device);
	EXIT_ON_ERROR(hr)
		hr = IMMDevice_Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL,
		(void**)&client);
	EXIT_ON_ERROR(hr)
		hr = IAudioClient_IsFormatSupported(client, sharemode, (WAVEFORMATEXTENSIBLE*)pwfx,
		(sharemode == AUDCLNT_SHAREMODE_SHARED) ? &bla : NULL);
	if (bla != NULL) { CoTaskMemFree(bla); }
	if (hr == S_OK) issuccess = TRUE;
Exit:
	if (client) {
		IAudioClient_Release(client);
	}
	client = 0;
	if (device) {
		IMMDevice_Release(device);
	}
	device = 0;
	if (pEnumerator) {
		IMMDeviceEnumerator_Release(pEnumerator);
	}
	pEnumerator = 0;
	return issuccess;
}

int on_error(int err, const char* msg)
{
	MessageBox(0, (LPCWSTR)msg, (LPCWSTR)"Windows Wasapi driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void)
{
	static AudioDriverInfo info;
	info.Flags = 0;
	info.Name = "Windows Wasapi AudioDriver";
	info.ShortName = "wasapi";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	WasapiDriver* wasapi;
	
	wasapi = (WasapiDriver*) malloc(sizeof(WasapiDriver));
	if (wasapi) {
		memset(wasapi, 0, sizeof(WasapiDriver));
		wasapi->driver.open = driver_open;
		wasapi->driver.deallocate = driver_deallocate;
		wasapi->driver.connect = driver_connect;
		wasapi->driver.open = driver_open;
		wasapi->driver.close = driver_close;
		wasapi->driver.dispose = driver_dispose;
		wasapi->driver.configure = driver_configure;
		wasapi->driver.samplerate = samplerate;
		wasapi->driver.addcapture = (psy_audiodriver_fp_addcapture) addcaptureport;
		wasapi->driver.removecapture = (psy_audiodriver_fp_removecapture) removecaptureport;
		wasapi->driver.capturename = (psy_audiodriver_fp_capturename) capturename;
		wasapi->driver.numcaptures = (psy_audiodriver_fp_numcaptures) numcaptures;
		wasapi->driver.playbackname = (psy_audiodriver_fp_playbackname) playbackname;
		wasapi->driver.numplaybacks = (psy_audiodriver_fp_numplaybacks) numplaybacks;
		wasapi->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		driver_init(&wasapi->driver);
		return &wasapi->driver;
	} else {
		return 0;
	}
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver->dispose(driver);
	free(driver);
}

int driver_init(psy_AudioDriver* driver)
{
	WasapiDriver* self = (WasapiDriver*) driver;
#ifdef PSYCLE_USE_SSE
	psy_dsp_sse2_init(&dsp);
#else
	psy_dsp_noopt_init(&dsp);
#endif
	SetupAVRT();
	psy_audiodriversettings_init(&self->settings);
	psy_audiodriversettings_setblockcount(&self->settings, 2);
	psy_table_init(&self->_portMapping);
	ZeroMemory(&self->out, sizeof(PaWasapiSubStream));
	refreshavailableports(self);
	self->error = on_error;
	self->_deviceId = 0;
	self->_dither = 0;	
	self->shared = TRUE;
	wcscpy_s(self->szDeviceID, MAX_STR_LEN - 1, L"");
	self->_capPorts = 0;
	init_properties(&self->driver);
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	WasapiDriver* self = (WasapiDriver*) driver;
	properties_free(self->driver.properties);
	self->driver.properties = 0;
	CloseHandle(self->hEvent);
	clearplayenums(self);
	clearcapenums(self);
	psy_list_free(self->_capPorts);	
	psy_table_dispose(&self->_portMapping);
	CloseAVRT();
	return 0;
}

static void init_properties(psy_AudioDriver* driver)
{		
	WasapiDriver* self = (WasapiDriver*) driver;
	psy_Properties* devices;
	psy_Properties* indevices;
	psy_List* p;
	int i;

	driver->properties = psy_properties_create();
		
	psy_properties_sethint(
		psy_properties_append_string(driver->properties, "name", "wasapi"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(driver->properties, "vendor", "Psycledelics"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(driver->properties, "version", "1.0"),
		PSY_PROPERTY_HINT_READONLY);
	devices = psy_properties_append_choice(driver->properties, "device", -1);
	
	psy_properties_append_int(driver->properties, "bitdepth", 16, 0, 32);
	psy_properties_append_int(driver->properties, "samplerate", 44100, 0, 0);
	psy_properties_append_int(driver->properties, "dither", 0, 0, 1);
	psy_properties_append_int(driver->properties, "numbuf", 8, 6, 8);
	psy_properties_append_int(driver->properties, "numsamples", 4096, 128, 8193);
	devices = psy_properties_append_choice(driver->properties, "device", 0);
	for (p = self->_playEnums, i = 0; p != NULL; p = p->next, ++i) {
		PortEnum* port = (PortEnum*)p->entry;
		psy_properties_append_int(devices, port->portName, i, 0, 0);
	}
	indevices = psy_properties_append_choice(driver->properties, "indevice", 0);
	for (p = self->_capEnums, i = 0; p != NULL; p = p->next, ++i) {
		PortEnum* port = (PortEnum*)p->entry;
		psy_properties_append_int(indevices, port->portName, i, 0, 0);
	}
}

void driver_configure(psy_AudioDriver* driver, psy_Properties* config)
{
	WasapiDriver* self;
	psy_Properties* property;

	self = (WasapiDriver*) driver;

	if (config) {
		properties_free(self->driver.properties);
		self->driver.properties = psy_properties_clone(config, 1);
	} else {
	property = psy_properties_read(self->driver.properties, "device");
	if (property && property->item.typ == PSY_PROPERTY_TYP_CHOICE) {
		psy_Properties* device;

		device = psy_properties_read_choice(property);
		if (device) {
			self->_deviceId =  psy_properties_value(device);
		}
	}
	// property = psy_properties_read(self->driver.properties, "bitdepth");
	// if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
	// 	self->_bitDepth = property->item.value.i;
	// }
	property = psy_properties_read(self->driver.properties, "samplerate");
	if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
		psy_audiodriversettings_setsamplespersec(&self->settings,
			property->item.value.i);
	}
	//property = psy_properties_read(self->driver.properties, "numbuf");
	//if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
	//	self->_numBlocks = property->item.value.i;
	//}
	//property = psy_properties_read(self->driver.properties, "numsamples");
	//if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
	//	self->_blockSizeBytes = property->item.value.i;
	//}
	//property = psy_properties_read(self->driver.properties, "numsamples");
	//if (property && property->item.typ == PSY_PROPERTY_TYP_INTEGER) {
	//	self->_blockSizeBytes = property->item.value.i;
	//}
	}
}

uintptr_t samplerate(psy_AudioDriver* self)
{
	return psy_audiodriversettings_samplespersec(&((WasapiDriver*)self)->settings);
}

void driver_connect(psy_AudioDriver* driver, void* context, AUDIODRIVERWORKFN callback, void* handle)
{
	driver->_callbackContext = context;
	driver->_pCallback = callback;
}

int driver_open(psy_AudioDriver* driver)
{
	WasapiDriver* self = (WasapiDriver*) driver;
	return start(self);	
}

bool start(WasapiDriver* self)
{
	HRESULT hr;
	IMMDeviceEnumerator* pEnumerator = NULL;
	wcscpy_s(self->out.szDeviceID, MAX_STR_LEN - 1, self->szDeviceID);
	self->out.streamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
	self->out.shareMode = self->shared ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE;
	if (self->running) return TRUE;
	if (!self->driver._pCallback) return FALSE;

	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
		&IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

		if (wcscmp(self->out.szDeviceID, L"") == 0) {
			hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnumerator, eRender,
				eMultimedia, &self->out.device);			
		}
		else {			
			hr = IMMDeviceEnumerator_GetDevice(pEnumerator, self->szDeviceID, &self->out.device);
		}
	EXIT_ON_ERROR(hr)
	hr = IMMDevice_Activate(self->out.device, &IID_IAudioClient, CLSCTX_ALL, NULL,
		(void**)&self->out.client);		
	EXIT_ON_ERROR(hr)

	hr = GetStreamFormat(self, &self->out, &self->out.wavex);
	EXIT_ON_ERROR(hr)

	UINT32 framesPerLatency;
	if (self->out.shareMode == AUDCLNT_SHAREMODE_SHARED)
	{		
		hr = IAudioClient_GetDevicePeriod(self->out.client,
			&self->out.period, NULL);
		EXIT_ON_ERROR(hr)

		framesPerLatency = MakeFramesFromHns(self->out.period, self->out.wavex.Format.nSamplesPerSec);
		psy_audiodriversettings_setblockcount(&self->settings, 2);
	}
	else {
		framesPerLatency = GetBufferSamples(self);
		if (framesPerLatency == 0) {
			hr = IAudioClient_GetDevicePeriod(self->out.client,
				NULL, &self->out.period);			
			EXIT_ON_ERROR(hr)
			// Add latency frames
			framesPerLatency = MakeFramesFromHns(self->out.period, self->out.wavex.Format.nSamplesPerSec);
		}
		psy_audiodriversettings_setblockcount(&self->settings, 2);
	}
	// Align frames to HD Audio packet size of 128 bytes 
	framesPerLatency = AlignFramesPerBuffer(framesPerLatency,
		self->out.wavex.Format.nSamplesPerSec, self->out.wavex.Format.nBlockAlign);

	// Calculate period
	self->out.period = MakeHnsPeriod(framesPerLatency, self->out.wavex.Format.nSamplesPerSec);
	psy_audiodriversettings_setblockframes(&self->settings, framesPerLatency);	

	hr = IAudioClient_Initialize(self->out.client, 	
		self->out.shareMode,
		self->out.streamFlags,
		self->out.period,
		(self->out.shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? self->out.period : 0),
		&self->out.wavex,
		NULL);
	
	EXIT_ON_ERROR(hr)
		
	hr = IAudioClient_GetBufferSize(self->out.client, &self->out.bufferFrameCount);
	EXIT_ON_ERROR(hr)
	psy_audiodriversettings_setblockframes(&self->settings, self->out.bufferFrameCount);
	
	hr = IAudioClient_GetService(self->out.client, &IID_IAudioClock,
		(void**)&self->pAudioClock);
	EXIT_ON_ERROR(hr)
	hr = IAudioClock_GetFrequency(self->pAudioClock, &self->audioClockFreq);
	EXIT_ON_ERROR(hr)


		//for (unsigned int i = 0; i < _capPorts.size(); i++) {
			//Ignore errors creating the Capture port to allow the playback to work.
			//CreateCapturePort(pEnumerator, _capPorts[i]);
		//}

	self->out.flags = 0;
	ResetEvent(self->hEvent);
	CreateThread(NULL, 0, EventAudioThread, self, 0, &self->dwThreadId);

	self->running = TRUE;
	if (pEnumerator) {
		IMMDeviceEnumerator_Release(pEnumerator);
	}
	pEnumerator = 0;
	return TRUE;
Exit:
	//For debugging purposes
	if (FAILED(hr)) {
		//Error(GetError(hr));
	}
	if (self->pAudioClock) {
		IAudioClock_Release(self->pAudioClock);
	}
	self->pAudioClock = 0;
	if (self->out.client) {
		IAudioClient_Release(self->out.client);
	}
	self->out.client = 0;
	if (self->out.device) {
		IMMDevice_Release(self->out.device);
	}
	self->out.device = 0;
	if (pEnumerator) {
		IMMDeviceEnumerator_Release(pEnumerator);
	}
	pEnumerator = 0;
	return FALSE;
}

unsigned int GetBufferSamples(WasapiDriver* self)
{
	return psy_audiodriversettings_blockframes(&self->settings);
}

void PrepareWaveFormat(WAVEFORMATEXTENSIBLE* wf, int channels, int sampleRate, int bits, int validBits)
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

int driver_close(psy_AudioDriver* driver)
{
	psy_List* pCapPort;
	int status;
	WasapiDriver* self;

	self = (WasapiDriver*)driver;
	status = stop((WasapiDriver*)driver);
	for (pCapPort = self->_capPorts; pCapPort != 0; pCapPort = pCapPort->next) {
		PaWasapiSubStream* portcap;

		portcap = (PaWasapiSubStream*)pCapPort->entry;
		free(portcap);
	}
	psy_list_free(self->_capPorts);
	self->_capPorts = 0;
	return status;
}

void refreshavailableports(WasapiDriver* self)
{
	IMMDeviceEnumerator* pEnumerator = NULL;
	HRESULT hr = S_OK;

	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
		&IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	EXIT_ON_ERROR(hr)
		RefreshPorts(self, pEnumerator);
Exit:
	if (pEnumerator) {
		IMMDeviceEnumerator_Release(pEnumerator);
	}
	pEnumerator = 0;
}

void RefreshPorts(WasapiDriver* self, IMMDeviceEnumerator* pEnumerator)
{
	HRESULT hr = S_OK;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice* pEndpoint = NULL;
	LPWSTR defaultID = NULL;	
	
	clearplayenums(self);
	clearcapenums(self);

	hr = IMMDeviceEnumerator_EnumAudioEndpoints(pEnumerator, eRender,
		DEVICE_STATE_ACTIVE, &pCollection);	
	EXIT_ON_ERROR(hr)
	hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnumerator, eRender,
		eMultimedia, &pEndpoint);		
	EXIT_ON_ERROR(hr)
	hr = IMMDevice_GetId(pEndpoint, &defaultID);		
	EXIT_ON_ERROR(hr)
	FillPortList(self, &self->_playEnums, pCollection, defaultID);
	CoTaskMemFree(defaultID);
	defaultID = NULL;
	if (pCollection) {
		IMMDeviceCollection_Release(pCollection);
	}
	pCollection = 0;
	hr = IMMDeviceEnumerator_EnumAudioEndpoints(pEnumerator, eCapture,
		DEVICE_STATE_ACTIVE, &pCollection);
	EXIT_ON_ERROR(hr)		
	hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnumerator, eCapture,
		eMultimedia, &pEndpoint);
	EXIT_ON_ERROR(hr)
	hr = IMMDevice_GetId(pEndpoint, &defaultID);		
	EXIT_ON_ERROR(hr)
	FillPortList(self, &self->_capEnums, pCollection, defaultID);
Exit:
	CoTaskMemFree(defaultID);
	if (pEndpoint) {
		IMMDevice_Release(pEndpoint);
	}
	pEndpoint = 0;
	if (pCollection) {
		IMMDeviceCollection_Release(pCollection);
	}
	pCollection = 0;
}

void FillPortList(WasapiDriver* self, psy_List** portList, IMMDeviceCollection* pCollection, LPWSTR defaultID)
{
	IMMDevice* pEndpoint = NULL;
	IPropertyStore* pProps = NULL;
	IAudioClient* client = NULL;
	WCHAR* pszDeviceId = NULL;
	HRESULT hr = S_OK;
	UINT count;
	ULONG i;
	
	hr = IMMDeviceCollection_GetCount(pCollection, &count);
	EXIT_ON_ERROR(hr)
	// Each loop prints the name of an endpoint device.
	for (i = 0; i < count; ++i)
	{
		// Get pointer to endpoint number i.		
		hr = IMMDeviceCollection_Item(pCollection, i, &pEndpoint);
		EXIT_ON_ERROR(hr)

		// Get the endpoint ID string.		
		hr = IMMDevice_GetId(pEndpoint, &pszDeviceId);
		EXIT_ON_ERROR(hr)

		hr = IMMDevice_OpenPropertyStore(pEndpoint, STGM_READ, &pProps);
		EXIT_ON_ERROR(hr)

		PortEnum* pEnum;

		pEnum = malloc(sizeof(PortEnum));
		// "Friendly" Name
		{
			PROPVARIANT vars;
			PropVariantInit(&vars);

			hr = IPropertyStore_GetValue(pProps, &PKEY_Device_FriendlyName, &vars);			
			EXIT_ON_ERROR(hr)

			wcscpy_s(pEnum->szDeviceID, MAX_STR_LEN - 1, pszDeviceId);
			// wcstombs(pEnum->portName, vars.pwszVal, MAX_STR_LEN - 1);

			PropVariantClear(&vars);
			if (wcscmp(pEnum->szDeviceID, defaultID) == 0) {
				pEnum->isDefault = TRUE;
			}
		}
		// Default format
		{
			PROPVARIANT vars;
			PropVariantInit(&vars);

			hr = IPropertyStore_GetValue(pProps, &PKEY_AudioEngine_DeviceFormat, &vars);			
			EXIT_ON_ERROR(hr);

			ZeroMemory(&pEnum->MixFormat, sizeof(WAVEFORMATEXTENSIBLE));
			memcpy(&pEnum->MixFormat, vars.blob.pBlobData, 
				min(sizeof(WAVEFORMATEXTENSIBLE), vars.blob.cbSize));
			PropVariantClear(&vars);
		}
		//lantency
		{			
			hr = IMMDevice_Activate(pEndpoint, &IID_IAudioClient, CLSCTX_ALL, NULL,
				(void**)&client);
			EXIT_ON_ERROR(hr)
			hr = IAudioClient_GetDevicePeriod(client,
				&pEnum->DefaultDevicePeriod, &pEnum->MinimumDevicePeriod);
			EXIT_ON_ERROR(hr)
		}
		{
			WAVEFORMATEX* pwft;
			
			hr = IAudioClient_GetMixFormat(client, &pwft);			
			EXIT_ON_ERROR(hr);
			memcpy(&pEnum->MixFormat, pwft, min(sizeof(WAVEFORMATEX) + pwft->cbSize,
				sizeof(WAVEFORMATEXTENSIBLE)));
			if (pwft->cbSize == 0) pEnum->MixFormat.Samples.wValidBitsPerSample = pwft->wBitsPerSample;
			CoTaskMemFree(pwft);
		}
		// Align frames to HD Audio packet size of 128 bytes 
		int samples = MakeFramesFromHns(pEnum->DefaultDevicePeriod,
				pEnum->MixFormat.Format.nSamplesPerSec);
			samples = AlignFramesPerBuffer(samples,
				pEnum->MixFormat.Format.nSamplesPerSec, pEnum->MixFormat.Format.nBlockAlign);
			pEnum->DefaultDevicePeriod = MakeHnsPeriod(samples, pEnum->MixFormat.Format.nSamplesPerSec);			
			psy_list_append(portList, pEnum);
			CoTaskMemFree(pszDeviceId);
			pszDeviceId = NULL;
			if (client) {
				IAudioClient_Release(client);
			}
			client = 0;
			if (pProps) {
				IPropertyStore_Release(pProps);
			}
			pProps = 0;
			if (pEndpoint) {
				IMMDevice_Release(pEndpoint);
			}
			pEndpoint = 0;
		}
Exit:
	CoTaskMemFree(pszDeviceId);
	if (client) {
		IAudioClient_Release(client);
	}
	client = 0;
	if (pProps) {
		IPropertyStore_Release(pProps);
	}
	pProps = 0;
	if (pEndpoint) {
		IMMDevice_Release(pEndpoint);
	}
	pEndpoint = 0;
}

static DWORD WINAPI EventAudioThread(void* pWasapi)
{
	HRESULT hr;
	HANDLE hEvent = NULL;
	HANDLE hTask = NULL;
	UINT32 bufferFrameCount;
	IAudioRenderClient* pRenderClient = NULL;
	psy_List* capture = 0; // IAudioCaptureClient*

	// universalis::os::thread_name thread_name("wasapi sound output");
	// universalis::cpu::exceptions::install_handler_in_thread();
	WasapiDriver* pThis = (WasapiDriver*)(pWasapi);
	// Ask MMCSS to temporarily boost the thread priority
	// to reduce glitches while the low-latency stream plays.
	DWORD taskIndex = 0;
	hTask = pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
	if (hTask == NULL)
	{
		// hr = E_FAIL;
		// EXIT_ON_ERROR(hr)
	}
	// Create an event handle and register it for buffer-event notifications.
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL)
	{
		hr = E_FAIL;
		goto Exit;
	}
	hr = IAudioClient_SetEventHandle(pThis->out.client, hEvent);	
	EXIT_ON_ERROR(hr);

	// Get the actual size of the two allocated buffers.
	hr = IAudioClient_GetBufferSize(pThis->out.client, &bufferFrameCount);
	EXIT_ON_ERROR(hr)
		pThis->out.bufferFrameCount = bufferFrameCount;

	// Get the lantency.
	hr = IAudioClient_GetBufferSize(pThis->out.client, &pThis->out.device_latency);
	EXIT_ON_ERROR(hr)

	hr = IAudioClient_GetService(pThis->out.client, &IID_IAudioRenderClient,
		(void**)&pRenderClient);
	EXIT_ON_ERROR(hr)

		// To reduce latency, load the first buffer with data
		// from the audio source before starting the stream.
		//hr = pThis->DoBlock(pRenderClient, bufferFrameCount);
	EXIT_ON_ERROR(hr)

	pThis->writeMark = 0;
	hr = IAudioClient_Start(pThis->out.client); // Start playing.
	EXIT_ON_ERROR(hr)

	psy_List* p;
	for (p = pThis->_capPorts; p != NULL; p = p->next) {	
		PaWasapiSubStream* capport = (PaWasapiSubStream*) p->entry;
			
		if (capport->client == NULL)
			continue;
		IAudioCaptureClient* captClient = NULL;
		hr = IAudioClient_GetBufferSize(capport->client, capport->bufferFrameCount);
		EXIT_ON_ERROR(hr)
		hr = IAudioClient_GetStreamLatency(capport->client, capport->device_latency);
		EXIT_ON_ERROR(hr)
		hr = IAudioClient_GetService(capport->client, &IID_IAudioCaptureClient,
			(void**)&captClient);
		EXIT_ON_ERROR(hr)
		hr = IAudioClient_Start(capport->client); // Start playing.
		EXIT_ON_ERROR(hr)
		psy_list_append(&capture, captClient);
	}

	// Each loop fills one of the two buffers.
	while (pThis->out.flags != AUDCLNT_BUFFERFLAGS_SILENT)
	{
		// Wait for next buffer event to be signaled.
		DWORD retval = WaitForSingleObject(hEvent, 2000);
		if (retval != WAIT_OBJECT_0)
		{
			// Event handle timed out after a 2-second wait.
			IAudioClient_Stop(pThis->out.client);			
			for (p = pThis->_capPorts; p != NULL; p = p->next) {			
				PaWasapiSubStream* capport = (PaWasapiSubStream*)p->entry;
				if (capport->client == NULL)
					continue; 
				hr = IAudioClient_Stop(capport->client);				
			}
			hr = ERROR_TIMEOUT;
			goto Exit;
		}

		if (pThis->out.shareMode == AUDCLNT_SHAREMODE_SHARED)
		{
			// See how much buffer space is available.
			UINT32 numFramesPadding, numFramesAvailable;
			hr = IAudioClient_GetCurrentPadding(pThis->out.client, &numFramesPadding);
			EXIT_ON_ERROR(hr)
				numFramesAvailable = bufferFrameCount - numFramesPadding;
			if (numFramesAvailable > 0) {
				psy_List* q;
				// First, run the capture buffers so that audio is available to wavein machines.
				for (p = pThis->_capPorts, q = capture; p != NULL && q != 0; p = p->next, q = q->next) {
					PaWasapiSubStream* capport = (PaWasapiSubStream*)p->entry;
					IAudioCaptureClient* captureClient = (IAudioCaptureClient*) q->entry;

					if (capport->client == NULL)
						continue;
					hr = DoBlockRecording(pThis, capport, captureClient, numFramesAvailable);
				}
				hr = DoBlock(pThis, pRenderClient, numFramesAvailable);
				EXIT_ON_ERROR(hr)
			}
		}
		else {
			psy_List* q;
			// First, run the capture buffers so that audio is available to wavein machines.
			for (p = pThis->_capPorts, q = capture; p != NULL && q != 0; p = p->next, q = q->next) {
				PaWasapiSubStream* capport = (PaWasapiSubStream*)p->entry;
				IAudioCaptureClient* captureClient = (IAudioCaptureClient*)q->entry;
				if (capport->client == NULL)
					continue;
				hr = DoBlockRecording(pThis, capport, captureClient, bufferFrameCount);
			}
			hr = DoBlock(pThis, pRenderClient, bufferFrameCount);
			EXIT_ON_ERROR(hr)
		}
	}
	{
		unsigned int i = 0;

		for (p = pThis->_capPorts; p != NULL; p = p->next) {
			PaWasapiSubStream* capport = (PaWasapiSubStream*)p->entry;
			if (capport->client == NULL)
				continue;
			hr = IAudioClient_Stop(capport->client);
		}
	}
	hr = IAudioClient_Stop(pThis->out.client); // Stop playing.
Exit:
	//For debugging purposes
	if (FAILED(hr)) {
		// Error(GetError(hr));
	}
	if (hEvent != NULL) { CloseHandle(hEvent); }
	if (hTask != NULL) { pAvRevertMmThreadCharacteristics(hTask); }
	{
		unsigned int i = 0;

		psy_List* q;
		// First, run the capture buffers so that audio is available to wavein machines.
		for (q = capture; q != 0; q = q->next) {			
			IAudioCaptureClient* captureClient = (IAudioCaptureClient*)q->entry;		
			IAudioCaptureClient_Release(captureClient);
		}
	}
	if (pRenderClient) {
		IAudioRenderClient_Release(pRenderClient);
	}
	pRenderClient = 0;
	pThis->running = FALSE;
	SetEvent(pThis->hEvent);
	return 0;
}

HRESULT DoBlockRecording(WasapiDriver* self, PaWasapiSubStream* port, IAudioCaptureClient* pCaptureClient, int numFramesAvailable)
{
	BYTE* pData;
	DWORD flags;
	UINT32 packetLength = 0;
	UINT32 numf;
	
	HRESULT hr = IAudioCaptureClient_GetNextPacketSize(pCaptureClient, &packetLength);
	EXIT_ON_ERROR(hr)

	while (packetLength != 0 && packetLength <= numFramesAvailable)
	{
		///\todo: finish this implementation.
		HRESULT hr = IAudioCaptureClient_GetBuffer(pCaptureClient, &pData, &numf, &flags, NULL, NULL);
		EXIT_ON_ERROR(hr)
			if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
			{
				unsigned int _sampleValidBits = psy_audiodriversettings_validbitdepth(&self->settings);
				if (_sampleValidBits == 32) {
					// DeinterlaceFloat(reinterpret_cast<float*>(pData), port.pleft, port.pright, numf);
				}
				else if (_sampleValidBits == 24) {
					// DeQuantize32AndDeinterlace(reinterpret_cast<int*>(pData), port.pleft, port.pright, numf);
				}
				else {
					// DeQuantize16AndDeinterlace(reinterpret_cast<short int*>(pData), port.pleft, port.pright, numf);
					// Quantize(pFloatBlock, pFloatBlock, numFramesAvailable);
				}
			}
		hr = IAudioCaptureClient_ReleaseBuffer(pCaptureClient, numf);
		EXIT_ON_ERROR(hr)
			numFramesAvailable -= numf;
		hr = IAudioCaptureClient_GetNextPacketSize(pCaptureClient, &packetLength);
		EXIT_ON_ERROR(hr)
	}
Exit:
	port->_machinepos = 0;
	return hr;
}

HRESULT DoBlock(WasapiDriver* self, IAudioRenderClient* pRenderClient, int numFramesAvailable)
{
	BYTE* pData;
	DWORD flags;	
	UINT32 numf;
	// Grab the next empty buffer from the audio device.	
	HRESULT hr = IAudioRenderClient_GetBuffer(pRenderClient, numFramesAvailable, &pData);
	EXIT_ON_ERROR(hr)
	unsigned int _sampleValidBits = psy_audiodriversettings_validbitdepth(&self->settings);
	int hostisplaying;
	float* pFloatBlock =
		self->driver._pCallback(
			self->driver._callbackContext, &numFramesAvailable, &hostisplaying);
	if (_sampleValidBits == 32) {
		dsp.movmul(pFloatBlock, (float*)pData, numFramesAvailable * 2, 1.f / 32768.f);
	}
	else if (_sampleValidBits == 24) {
		// Quantize24in32Bit(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
	}
	else if (_sampleValidBits == 16) {
		//if (settings_->dither()) Quantize16WithDither(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
		// else Quantize16(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
		psy_dsp_quantize16(pFloatBlock, (int*)pFloatBlock, numFramesAvailable);
	}

	hr = IAudioRenderClient_ReleaseBuffer(pRenderClient, numFramesAvailable, self->out.flags);
	self->writeMark += numFramesAvailable;
Exit:
	return hr;
}

void GetPlaybackPorts(WasapiDriver* self, psy_List** ports)
{
	FreePorts(*ports);
	*ports = 0;
	//for (unsigned int i = 0; i < _playEnums.size(); i++) ports.push_back(_playEnums[i].portName);
}

void GetCapturePorts(WasapiDriver* self, psy_List** ports)
{
	FreePorts(*ports);
	*ports = 0;
	//for (unsigned int i = 0; i < _capEnums.size(); i++) ports.push_back(_capEnums[i].portName);
}

void FreePorts(psy_List* ports)
{
	psy_List* p;

	for (p = ports; p != NULL; p = p->next) {
		free(p->entry);
	}
	psy_list_free(ports);
}

void GetReadBuffers(WasapiDriver* self, int idx, float** pleft, float** pright, int numsamples)
{
	/*if (!self->running || idx >= self->_portMapping.size() || _portMapping[idx] == -1
		|| _capPorts[_portMapping[idx]].client == NULL)
	{
		*pleft = 0;
		*pright = 0;
		return;
	}
	int mpos = _capPorts[_portMapping[idx]]._machinepos;
	*pleft = _capPorts[_portMapping[idx]].pleft + mpos;
	*pright = _capPorts[_portMapping[idx]].pright + mpos;
	_capPorts[_portMapping[idx]]._machinepos += numsamples;*/
}

HRESULT GetStreamFormat(WasapiDriver* self, PaWasapiSubStream* stream, WAVEFORMATEXTENSIBLE* wfOut)
{
	HRESULT hr = 0;	

	if (stream->shareMode == AUDCLNT_SHAREMODE_SHARED)
	{
		WAVEFORMATEX* pwft;
		hr = IAudioClient_GetMixFormat(stream->client, &pwft);
		EXIT_ON_ERROR(hr)
		
		psy_audiodriversettings_setsamplespersec(&self->settings,
			pwft->nSamplesPerSec);
		psy_audiodriversettings_setvalidbitdepth(&self->settings,
			pwft->wBitsPerSample);
		if (pwft->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
			WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)pwft;
			psy_audiodriversettings_setvalidbitdepth(&self->settings,
				wfex->Samples.wValidBitsPerSample);
		}
		memcpy(wfOut, pwft, min(sizeof(WAVEFORMATEX) + pwft->cbSize, sizeof(WAVEFORMATEXTENSIBLE)));
		CoTaskMemFree(pwft);
	}
	else
	{
		WAVEFORMATPCMEX format;
		
		PrepareWaveFormat((WAVEFORMATEXTENSIBLE*)&format,
			psy_audiodriversettings_numchannels(&self->settings),
			psy_audiodriversettings_samplespersec(&self->settings),
			psy_audiodriversettings_bitdepth(&self->settings),
			psy_audiodriversettings_validbitdepth(&self->settings));
		memcpy(wfOut, &format, sizeof(WAVEFORMATPCMEX));
	}
	WAVEFORMATEX* bla = NULL;	
	hr = IAudioClient_IsFormatSupported(stream->client, stream->shareMode, wfOut,
		(stream->shareMode == AUDCLNT_SHAREMODE_SHARED) ? &bla : NULL);
	if (bla != NULL) { CoTaskMemFree(bla); }
	return hr;
Exit:
	return hr;
}

uint32_t GetWritePosInSamples(WasapiDriver* self)
{
	return self->writeMark;
}

uint32_t GetPlayPosInSamples(WasapiDriver* self)
{
	UINT64 pos;
	uint32_t retVal = 0;
	if (self->running) {
		HRESULT hr = IAudioClock_GetPosition(self->pAudioClock, &pos, NULL);		
		EXIT_ON_ERROR(hr)
		if (self->audioClockFreq == psy_audiodriversettings_samplespersec(&self->settings)) {
			retVal = pos;
		} else {
			// Thus, the stream-relative offset in seconds can always be calculated as p/f.
			retVal = (pos * psy_audiodriversettings_samplespersec(&self->settings) / self->audioClockFreq);
		}
	}
Exit:
	return retVal;
}

uint32_t GetInputLatencyMs(WasapiDriver* self)
{
	if (self->_capPorts) {
		///\todo: The documentation suggests that the period has to be added to the latency. verify it.
		PaWasapiSubStream* port;
		REFERENCE_TIME rf;

		port = (PaWasapiSubStream*) self->_capPorts->entry;
		rf = port->device_latency + port->period;
		return nano100ToMillis(&rf);
	} else
	return 0;
}

uint32_t GetOutputLatencyMs(WasapiDriver* self)
{
	REFERENCE_TIME rf;
	///\todo: The documentation suggests that the period has to be added to the latency. verify it.
	rf = self->out.device_latency + self->out.period;
	return nano100ToMillis(&rf);
}

uint32_t GetInputLatencySamples(WasapiDriver* self)
{ 
	return GetInputLatencyMs(self) * psy_audiodriversettings_samplespersec(&self->settings) * 0.001f;
}

uint32_t GetOutputLatencySamples(WasapiDriver* self)
{ 
	return GetOutputLatencyMs(self) * psy_audiodriversettings_samplespersec(&self->settings) * 0.001f;
}

uint32_t GetIdxFromDevice(WasapiDriver* self, WCHAR* szDeviceID)
{
	psy_List* pPort;
	uint32_t i;

	for (pPort = self->_capEnums, i = 0; pPort != 0; pPort = pPort->next, ++i) {
		PortEnum* port;

		port = (PortEnum*) pPort->entry;
		if (wcscmp(port->szDeviceID, szDeviceID) == 0) {
			return i;
		}
	}
	return 0;
}

const char* GetError(HRESULT hr)
{
	const char* text = NULL;
	switch (hr)
	{
	case S_OK:									return "";
	case E_POINTER:text = "E_POINTER"; break;
	case E_INVALIDARG:text = "E_INVALIDARG"; break;

	case AUDCLNT_E_NOT_INITIALIZED:text = "AUDCLNT_E_NOT_INITIALIZED"; break;
	case AUDCLNT_E_ALREADY_INITIALIZED:text = "AUDCLNT_E_ALREADY_INITIALIZED"; break;
	case AUDCLNT_E_WRONG_ENDPOINT_TYPE:text = "AUDCLNT_E_WRONG_ENDPOINT_TYPE"; break;
	case AUDCLNT_E_DEVICE_INVALIDATED:text = "AUDCLNT_E_DEVICE_INVALIDATED"; break;
	case AUDCLNT_E_NOT_STOPPED:text = "AUDCLNT_E_NOT_STOPPED"; break;
	case AUDCLNT_E_BUFFER_TOO_LARGE:text = "AUDCLNT_E_BUFFER_TOO_LARGE"; break;
	case AUDCLNT_E_OUT_OF_ORDER:text = "AUDCLNT_E_OUT_OF_ORDER"; break;
	case AUDCLNT_E_UNSUPPORTED_FORMAT:text = "AUDCLNT_E_UNSUPPORTED_FORMAT"; break;
	case AUDCLNT_E_INVALID_SIZE:text = "AUDCLNT_E_INVALID_SIZE"; break;
	case AUDCLNT_E_DEVICE_IN_USE:text = "AUDCLNT_E_DEVICE_IN_USE"; break;
	case AUDCLNT_E_BUFFER_OPERATION_PENDING:text = "AUDCLNT_E_BUFFER_OPERATION_PENDING"; break;
	case AUDCLNT_E_THREAD_NOT_REGISTERED:text = "AUDCLNT_E_THREAD_NOT_REGISTERED"; break;
	case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:text = "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"; break;
	case AUDCLNT_E_ENDPOINT_CREATE_FAILED:text = "AUDCLNT_E_ENDPOINT_CREATE_FAILED"; break;
	case AUDCLNT_E_SERVICE_NOT_RUNNING:text = "AUDCLNT_E_SERVICE_NOT_RUNNING"; break;
	case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED:text = "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"; break;
	case AUDCLNT_E_EXCLUSIVE_MODE_ONLY:text = "AUDCLNT_E_EXCLUSIVE_MODE_ONLY"; break;
	case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:text = "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"; break;
	case AUDCLNT_E_EVENTHANDLE_NOT_SET:text = "AUDCLNT_E_EVENTHANDLE_NOT_SET"; break;
	case AUDCLNT_E_INCORRECT_BUFFER_SIZE:text = "AUDCLNT_E_INCORRECT_BUFFER_SIZE"; break;
	case AUDCLNT_E_BUFFER_SIZE_ERROR:text = "AUDCLNT_E_BUFFER_SIZE_ERROR"; break;
	case AUDCLNT_E_CPUUSAGE_EXCEEDED:text = "AUDCLNT_E_CPUUSAGE_EXCEEDED"; break;

#ifdef AUDCLNT_E_BUFFER_ERROR
	case AUDCLNT_E_BUFFER_ERROR:text = "AUDCLNT_E_BUFFER_ERROR"; break;
#endif

#ifdef AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED
	case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:text = "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"; break;
#endif

#ifdef AUDCLNT_E_INVALID_DEVICE_PERIOD
	case AUDCLNT_E_INVALID_DEVICE_PERIOD:text = "AUDCLNT_E_INVALID_DEVICE_PERIOD"; break;
#endif

	case AUDCLNT_S_BUFFER_EMPTY:text = "AUDCLNT_S_BUFFER_EMPTY"; break;
	case AUDCLNT_S_THREAD_ALREADY_REGISTERED:text = "AUDCLNT_S_THREAD_ALREADY_REGISTERED"; break;
	case AUDCLNT_S_POSITION_STALLED:text = "AUDCLNT_S_POSITION_STALLED"; break;

		// other windows common errors:
	case CO_E_NOTINITIALIZED:text = "CO_E_NOTINITIALIZED"; break;

	default:
		text = "UNKNOWN ERROR";
	}
	return text;
}

bool RemoveCapturePort(WasapiDriver* self, int idx)
{
	bool isplaying = self->running;
	int maxSize = 0;
	/*std::vector<PaWasapiSubStream> newports;
	if (idx >= _capEnums.size() ||
		idx >= _portMapping.size() || _portMapping[idx] == -1) return false;

	if (isplaying) Stop();
	for (unsigned int i = 0; i < _portMapping.size(); ++i)
	{
		if (i != idx && _portMapping[i] != -1) {
			maxSize = i + 1;
			newports.push_back(_capPorts[_portMapping[i]]);
			_portMapping[i] = (int)(newports.size() - 1);
		}
	}
	_portMapping[idx] = -1;
	if (maxSize < _portMapping.size()) _portMapping.resize(maxSize);
	_capPorts = newports;
	if (isplaying) Start();*/
	return TRUE;
}

bool AddCapturePort(WasapiDriver* self, int idx)
{
	bool isplaying = self->running;
/*	if (idx >= _capEnums.size()) return false;
	if (idx < _portMapping.size() && _portMapping[idx] != -1) return true;

	if (isplaying) Stop();
	PaWasapiSubStream port;
	ZeroMemory(&port, sizeof(PaWasapiSubStream));
	wcscpy_s(port.szDeviceID, MAX_STR_LEN - 1, _capEnums[idx].szDeviceID);
	_capPorts.push_back(port);
	if (_portMapping.size() <= idx) {
		int oldsize = _portMapping.size();
		_portMapping.resize(idx + 1);
		for (int i = oldsize; i < _portMapping.size(); i++) _portMapping[i] = -1;
	}
	_portMapping[idx] = (int)(_capPorts.size() - 1);
	if (isplaying) return Start();*/
	return TRUE;
};

HRESULT CreateCapturePort(WasapiDriver* self, IMMDeviceEnumerator* pEnumerator, PaWasapiSubStream* port)
{
	//avoid opening a port twice
	//if (port.client != NULL) return true;
	//port._machinepos = 0;
	HRESULT hr = 0;
	/*if (wcscmp(port.szDeviceID, L"") == 0) {
		hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &port.device);
	}
	else {
		hr = pEnumerator->GetDevice(port.szDeviceID, &port.device);
	}
	EXIT_ON_ERROR(hr)

		hr = port.device->Activate(IID_IAudioClient, CLSCTX_ALL,
			NULL, (void**)&port.client);
	EXIT_ON_ERROR(hr)

		port.shareMode = out.shareMode;
	hr = GetStreamFormat(port, &port.wavex);
	EXIT_ON_ERROR(hr)
		port.streamFlags = 0;
	port.period = out.period;

	hr = port.client->Initialize(port.shareMode, port.streamFlags, port.period,
		(out.shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? port.period : 0),
		reinterpret_cast<WAVEFORMATEX*>(&port.wavex), NULL);
	port.flags = 0;
	EXIT_ON_ERROR(hr)
		// 2* is a safety measure (Haven't been able to dig out why it crashes if it is exactly the size)
		universalis::os::aligned_memory_alloc(16, port.pleft, 2 * settings_->blockBytes());
	universalis::os::aligned_memory_alloc(16, port.pright, 2 * settings_->blockBytes());
	ZeroMemory(port.pleft, 2 * settings_->blockBytes());
	ZeroMemory(port.pright, 2 * settings_->blockBytes());
	return hr;
Exit:
	Error("Couldn't open the capture device. Possibly the format is not supported"); */
	//SAFE_RELEASE(port.client)
	//SAFE_RELEASE(port.device)
	return hr;
}

bool stop(WasapiDriver* self)
{
	if (!self->running) return FALSE;

	self->out.flags = AUDCLNT_BUFFERFLAGS_SILENT;
	WaitForSingleObject(self->hEvent, INFINITE);
	// for (unsigned int i = 0; i < _capPorts.size(); i++)
	// {
	//	SAFE_RELEASE(_capPorts[i].client)
	//		SAFE_RELEASE(_capPorts[i].device)
	//}
	if (self->pAudioClock) {
		IAudioClock_Release(self->pAudioClock);
	}
	self->pAudioClock = 0;
	if (self->out.client) {
		IAudioClient_Release(self->out.client);
	}
	self->out.client = 0;
	if (self->out.device) {
		IMMDevice_Release(self->out.device);
	}
	self->out.device = 0;
	return TRUE;
}

void clearplayenums(WasapiDriver* self)
{
	psy_List* p;

	for (p = self->_playEnums; p != NULL; p = p->next) {
		PortEnum* port = (PortEnum*)p->entry;
		// portenum_dispose(port);
		free(port);
	}
	psy_list_free(self->_playEnums);
	self->_playEnums = 0;
}

void clearcapenums(WasapiDriver* self)
{
	psy_List* p;

	for (p = self->_capEnums; p != NULL; p = p->next) {
		PortEnum* port = (PortEnum*)p->entry;
		// portenum_dispose(port);
		free(port);
	}
	psy_list_free(self->_capEnums);
	self->_capEnums = 0;
}

int addcaptureport(WasapiDriver* self, int idx)
{
	PaWasapiSubStream* port;
	bool isplaying;
	
	isplaying = self->_running;
	if (idx >= (int)psy_list_size(self->_capEnums)) {
		return FALSE;
	}
	if (idx < (int)psy_table_size(&self->_portMapping) &&
		(intptr_t)psy_table_at(&self->_portMapping, idx) != -1) {
		return TRUE;
	}
	if (isplaying) {
		stop(self);
	}
	port = malloc(sizeof(PaWasapiSubStream));
	if (port) {
		ZeroMemory(port, sizeof(PaWasapiSubStream));
		wcscpy_s(port->szDeviceID, MAX_STR_LEN - 1,
			((PortEnum*)psy_list_at(self->_capEnums, idx)->entry)->szDeviceID);
		psy_list_append(&self->_capPorts, port);
		if ((int) psy_table_size(&self->_portMapping) <= idx) {
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

int removecaptureport(WasapiDriver* self, int idx)
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
		PaWasapiSubStream* portcap;

		portcap = (PaWasapiSubStream*)pCapPort->entry;
		free(portcap);
	}
	psy_list_free(self->_capPorts);
	self->_capPorts = newports;
	if (isplaying) {
		start(self);
	}
	return TRUE;
}

const char* capturename(psy_AudioDriver* driver, int index)
{
	WasapiDriver* self = (WasapiDriver*)driver;
	psy_List* pPort;

	if (self->_capEnums) {
		pPort = psy_list_at(self->_capEnums, index);
		if (pPort) {
			return ((PortEnum*)(pPort->entry))->portName;
		}
	}
	return "";
}

int numcaptures(psy_AudioDriver* driver)
{
	WasapiDriver* self = (WasapiDriver*)driver;

	return psy_list_size(self->_capEnums);
}

const char* playbackname(psy_AudioDriver* driver, int index)
{
	WasapiDriver* self = (WasapiDriver*)driver;
	psy_List* pPort;

	if (self->_playEnums) {
		pPort = psy_list_at(self->_playEnums, index);
		if (pPort) {
			return ((PortEnum*)(pPort->entry))->portName;
		}
	}
	return "";
}

int numplaybacks(psy_AudioDriver* driver)
{
	WasapiDriver* self = (WasapiDriver*)driver;

	return psy_list_size(self->_playEnums);
}
