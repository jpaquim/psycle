// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "fileoutdriver.h"

#include "../../detail/os.h"
#include "../../driver/audiodriver.h"
#include "../../driver/audiodriversettings.h"

#include <stdlib.h>
#include <string.h>

#include "waveio.h"
#include <fileio.h>

#if defined(DIVERSALIS__OS__MICROSOFT)	
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#endif

typedef enum {
	DDC_SUCCESS,           ///< operation succeded
	DDC_FAILURE,           ///< operation failed for unspecified reasons
	DDC_OUT_OF_MEMORY,     ///< operation failed due to running out of memory
	DDC_FILE_ERROR,        ///< operation encountered file I/O error
	DDC_INVALID_CALL,      ///< operation was called with invalid parameters
	DDC_USER_ABORT,        ///< operation was aborted by the user
	DDC_INVALID_FILE       ///< file format does not match
} DDCRET;


DDCRET WriteMonoSample(psy_audio_WaveFormatChunk*, float SampleData,
	PsyFile*);
DDCRET WriteStereoSample(psy_audio_WaveFormatChunk*, float LeftSample, float RightSample,
	PsyFile*);

typedef struct {
	PsyFile file;
	char* path;
	uint32_t begin;
	uint32_t numsamplesbegin;
	uint32_t numsamples;
} FileContext;

typedef struct {
	psy_AudioDriver driver;
	psy_AudioDriverSettings settings;
	psy_Property* configuration;
	int pollsleep;
	int stoppolling;
#if defined(DIVERSALIS__OS__MICROSOFT)	
	HANDLE hEvent;
#endif	
	FileContext filecontext;
	psy_audio_WaveFormatChunk format;
} FileOutDriver;

static void driver_deallocate(psy_AudioDriver*);
static int fileoutdriver_init(FileOutDriver*);
static void driver_connect(psy_AudioDriver*, void* context, AUDIODRIVERWORKFN callback,
	void* handle);
static int driver_open(psy_AudioDriver*);
static void driver_configure(FileOutDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static unsigned int samplerate(psy_AudioDriver*);
static void PollerThread(void *fileoutdriver);
static void fileoutdriver_createfile(FileOutDriver*);
static void fileoutdriver_writebuffer(FileOutDriver*, float* pBuf,
	uintptr_t amount);
static void fileoutdriver_closefile(FileOutDriver*);

static void init_properties(FileOutDriver* driver);

static psy_AudioDriverVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_deallocate;
		vtable.connect = driver_connect;
		vtable.open = driver_open;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.configure = (psy_audiodriver_fp_configure)driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = (psy_audiodriver_fp_samplerate)samplerate;
		vtable_initialized = 1;
	}
}

psy_AudioDriver* psy_audio_create_fileout_driver(void)
{
	FileOutDriver* out = malloc(sizeof(FileOutDriver));
	if (out) {		
		fileoutdriver_init(out);
		return &out->driver;
	}
	return NULL;
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver_dispose(driver);
	free(driver);
}

int fileoutdriver_init(FileOutDriver* self)
{
	memset(&self->driver, 0, sizeof(psy_AudioDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	psy_audiodriversettings_init(&self->settings);	
	psy_signal_init(&self->driver.signal_stop);
	init_properties(self);
#if defined(DIVERSALIS__OS__MICROSOFT)	
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif	
	self->pollsleep = 1;
	self->stoppolling = 0;
	self->filecontext.path = strdup("Untitled.wav");	
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	FileOutDriver* self;

	self = (FileOutDriver*) driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	psy_signal_dispose(&driver->signal_stop);
	free(self->filecontext.path);
	self->filecontext.path = 0;
	return 0;
}

void driver_connect(psy_AudioDriver* driver, void* context, AUDIODRIVERWORKFN callback,
	void* handle)
{
	driver->_pCallback = callback;
	driver->_callbackContext = context;
}

int driver_open(psy_AudioDriver* driver)
{	
	FileOutDriver* self;

	self = (FileOutDriver*) driver;
	self->stoppolling = 0;
#if defined(DIVERSALIS__OS__MICROSOFT)	
	ResetEvent(self->hEvent);
	_beginthread(PollerThread, 0, self);
#endif	
	return 0;
}

int driver_close(psy_AudioDriver* driver)
{
	FileOutDriver* self = (FileOutDriver*) driver;

	self->stoppolling = 1;
#if defined(DIVERSALIS__OS__MICROSOFT)		
	WaitForSingleObject(self->hEvent, INFINITE);
#endif	
	return 0;
}

void driver_configure(FileOutDriver* self, psy_Property* config)
{
	psy_AudioDriver* driver = &self->driver;

	if (config) {
		psy_property_sync(self->configuration, config);
	}
	free(self->filecontext.path);
	self->filecontext.path = strdup(
		psy_property_at_str(self->configuration,
			"outputpath", "Untitled.wav"));
	psy_audiodriversettings_setvalidbitdepth(&self->settings,
		psy_property_at_int(self->configuration, "bitdepth", 16));
	psy_audiodriversettings_setsamplespersec(&self->settings,
		psy_property_at_int(self->configuration, "samplerate", 44100));
	psy_audiodriversettings_setchannelmode(&self->settings,
		psy_property_at_int(self->configuration, "channels",
			psy_AUDIODRIVERCHANNELMODE_STEREO));
}

unsigned int samplerate(psy_AudioDriver* self)
{
	return 44100;
}

void init_properties(FileOutDriver* self)
{
	psy_AudioDriver* driver = &self->driver;

	self->configuration = psy_property_allocinit_key("FileOut Driver");
	psy_property_settext(
		psy_property_setreadonly(
			psy_property_append_string(self->configuration, "name", "FileOut Driver"),
			TRUE),
		"Name");
	psy_property_setreadonly(
		psy_property_append_string(self->configuration, "vendor", "Psycledelics"),
		TRUE);
	psy_property_setreadonly(
		psy_property_append_string(self->configuration, "version", "1.0"),
		TRUE);
	psy_property_append_string(self->configuration, "outputpath", "Untitled.wav");
	psy_property_append_int(self->configuration, "bitdepth",
		psy_audiodriversettings_bitdepth(&self->settings), 0, 32);
	psy_property_append_int(self->configuration, "samplerate",
		psy_audiodriversettings_samplespersec(&self->settings), 0, 0);
	psy_property_append_choice(self->configuration, "channels",
		(int)psy_audiodriversettings_channelmode(&self->settings));
}

void PollerThread(void* driver)
{	
	int n;	
	uintptr_t blocksize = 4096;
	int hostisplaying = 1;

	FileOutDriver* self = (FileOutDriver*) driver;
#if defined(DIVERSALIS__OS__MICROSOFT)	
	SetThreadPriority(GetCurrentThread(),
		THREAD_PRIORITY_ABOVE_NORMAL);
#endif		
	fileoutdriver_createfile(self);	
	while (!self->stoppolling && hostisplaying)
	{
		float *pBuf;
		n = blocksize / 2;
		
		pBuf = self->driver._pCallback(self->driver._callbackContext, &n,
			&hostisplaying);		
		fileoutdriver_writebuffer(self, pBuf, blocksize);
		self->filecontext.numsamples += blocksize;
#if defined(DIVERSALIS__OS__MICROSOFT)			
		Sleep(self->pollsleep);
#endif		
	}
	fileoutdriver_closefile(self);
#if defined(DIVERSALIS__OS__MICROSOFT)			
	SetEvent(self->hEvent);
#endif	
	psy_signal_emit(&self->driver.signal_stop, self, 0);
#if defined(DIVERSALIS__OS__MICROSOFT)	
	_endthread();
#endif	
}

void fileoutdriver_createfile(FileOutDriver* self)
{		
	int bitspersample = 16;
	uint8_t temp8 = 0;
	uint16_t temp16 = 0;
	uint32_t temp32 = 0;	
	uint32_t begin = 0;
	uint32_t pcmbegin = 0;
	uint32_t channel = 0;
	uint32_t frame = 0;
	uint32_t numsamples = 0;
	PsyFile* file;

	file = &self->filecontext.file;
	self->filecontext.numsamples = 0;
	if (!psyfile_create(file, self->filecontext.path, 1)) {
		return;
	}
	psyfile_write(file, "RIFF", 4);
	begin = psyfile_getpos(file);
	self->filecontext.begin = begin;
	temp32 = 0;
	psyfile_write(file, &temp32, sizeof(temp32));
	// Write Format Chunk
	psy_audio_waveformatchunk_config(&self->format,
		psy_audiodriversettings_samplespersec(&self->settings),
		psy_audiodriversettings_validbitdepth(&self->settings),
		psy_audiodriversettings_numchannels(&self->settings),
		FALSE /* isfloat */);
	psyfile_write(file, "WAVEfmt ", 8);
	temp32 = 0;
	pcmbegin = psyfile_getpos(file);
	psyfile_write(file, &temp32, sizeof(temp32));	
	temp16 = self->format.wFormatTag;
	psyfile_write(file, &temp16, sizeof(temp16));
	temp16 = self->format.nChannels;
	psyfile_write(file, &temp16, sizeof(temp16));
	temp32 = self->format.nSamplesPerSec;
	psyfile_write(file, &temp32, sizeof(temp32));
	temp32 = self->format.nAvgBytesPerSec;
	psyfile_write(file, &temp32, sizeof(temp32));
	temp16 = self->format.nBlockAlign;
	psyfile_write(file, &temp16, sizeof(temp16));
	temp16 = self->format.wBitsPerSample;
	psyfile_write(file, &temp16, sizeof(temp16));
	{
		uint32_t pos2;
		uint32_t size;
	
		pos2 = psyfile_getpos(file); 
		size = (pos2 - pcmbegin - 4);
		psyfile_seek(file, pcmbegin);
		psyfile_write(file, &size, sizeof(size));
		psyfile_seek(file, pos2);
	}
	// Write Data Chunk
	psyfile_write(file, "data", 4);
	numsamples = 0;
	self->filecontext.numsamplesbegin = psyfile_getpos(file);
	self->filecontext.numsamples = 0;
	temp32 = 0xEDED;
	psyfile_write(file, &temp32, sizeof(temp32));
}

void fileoutdriver_writebuffer(FileOutDriver* self, float* pBuf, uintptr_t amount)
{
	uintptr_t i;
	float* currbuf = pBuf;	
	
	amount /= 2;
	switch (psy_audiodriversettings_channelmode(&self->settings)) {
	case psy_AUDIODRIVERCHANNELMODE_MONO_MIX: // mono mix				
		for (i = 0; i < amount; i++) {
			float l, r;

			l = *currbuf;
			++currbuf;
			r = *currbuf;
			++currbuf;
			//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
			WriteMonoSample(&self->format, ((l + r) * 0.5f), &self->filecontext.file);
		}
	break;
	case psy_AUDIODRIVERCHANNELMODE_MONO_LEFT: // mono L
		for (i = 0; i < amount; ++i) {
			WriteMonoSample(&self->format, *currbuf, &self->filecontext.file);
			currbuf++;
			currbuf++; // skip right channel
		}	
	break;
	case psy_AUDIODRIVERCHANNELMODE_MONO_RIGHT: // mono R
		for (i = 0; i < amount; ++i) {
			currbuf++; // skip left channel
			WriteMonoSample(&self->format, *currbuf, &self->filecontext.file);
			currbuf++;
		}
		break;
	default: // psy_AUDIODRIVERCHANNELMODE_STEREO
		for (i = 0; i < amount; ++i) {
			float l;
			float r;

			l = *currbuf;
			++currbuf;
			r = *currbuf;
			++currbuf;
			WriteStereoSample(&self->format, l, r, &self->filecontext.file);
		}
		break;
	}
	self->filecontext.numsamples += amount;
}

void fileoutdriver_closefile(FileOutDriver* self)
{
	uint32_t pos2;
	uint32_t size;
	PsyFile* file;

	file = &self->filecontext.file;
	{
		uint32_t temp32;

		pos2 = psyfile_getpos(file);
		psyfile_seek(file, self->filecontext.numsamplesbegin);
		temp32 = self->filecontext.numsamples;
		psyfile_write(file, &temp32, sizeof(temp32));
		psyfile_seek(file, pos2);	
	}	
	pos2 = psyfile_getpos(file); 
	size = (pos2 - self->filecontext.begin - 4);
	psyfile_seek(file, self->filecontext.begin);
	psyfile_write(file, &size, sizeof(size));
	psyfile_seek(file, pos2);	
	psyfile_close(file);
}

DDCRET WriteMonoSample(psy_audio_WaveFormatChunk* wave_format, float SampleData,
	PsyFile* fp)
{
	int32_t d;

	switch (wave_format->wFormatTag)
	{
	case 1: // Integer PCM
		if (SampleData > 32767.0f) SampleData = 32767.0f;
		else if (SampleData < -32768.0f) SampleData = -32768.0f;
		switch (wave_format->wBitsPerSample) {
		case 8:
			// pcm_data.ckSize += 1;
			d = (int32_t)(SampleData / 256.0f);
			d += 128;
			return psyfile_write(fp, &d, 1);
		case 16:
			// pcm_data.ckSize += 2;
			d = (int32_t)(SampleData);
			return psyfile_write(fp, &d, 2);
		case 24:
			//pcm_data.ckSize += 3;
			d = (int32_t)(SampleData * 256.0f);
			return psyfile_write(fp, &d, 3);
		case 32:
			// pcm_data.ckSize += 4;
			// d = (int32_t)(SampleData * 65536.0f);
			return psyfile_write(fp, &SampleData, 4);
		default:
			break;
		}
		break;
	case 3: // IEEE float PCM
		if (wave_format->wBitsPerSample == 32)
		{
			// pcm_data.ckSize += 4;
			const float f = SampleData * 0.000030517578125f;
			return psyfile_write(fp, &f, 4);
		}
	default:
		break;
	}
	return DDC_INVALID_CALL;
}

DDCRET WriteStereoSample(psy_audio_WaveFormatChunk* wave_format, float LeftSample, float RightSample,
	PsyFile* fp)
{
	DDCRET retcode = DDC_SUCCESS;
	int32_t l, r;
	float f;
	switch (wave_format->wFormatTag)
	{
	case 1: // Integer PCM
		if (LeftSample > 32767.0f) LeftSample = 32767.0f;
		else if (LeftSample < -32768.0f) LeftSample = -32768.0f;
		if (RightSample > 32767.0f) RightSample = 32767.0f;
		else if (RightSample < -32768.0f) RightSample = -32768.0f;
		switch (wave_format->wBitsPerSample) {
		case 8:
			l = (int32_t)(LeftSample / 256.0f);
			r = (int32_t)(RightSample / 256.0f);
			l += 128;
			r += 128;
			retcode = psyfile_write(fp, &l, 1);
			if (retcode == DDC_SUCCESS) {
				retcode = psyfile_write(fp, &r, 1);
				// if (retcode == DDC_SUCCESS) pcm_data.ckSize += 2;
			}
			break;
		case 16:
			l = (int32_t)(LeftSample);
			r = (int32_t)(RightSample);
			retcode = psyfile_write(fp, &l, 2);
			if (retcode == DDC_SUCCESS) {
				retcode = psyfile_write(fp, &r, 2);
				// if (retcode == DDC_SUCCESS) pcm_data.ckSize += 4;
			}
			break;
		case 24:
			l = (int32_t)(LeftSample * 256.0f);
			r = (int32_t)(RightSample * 256.0f);
			retcode = psyfile_write(fp, &l, 3);
			if (retcode == DDC_SUCCESS) {
				retcode = psyfile_write(fp, &r, 3);
				// if (retcode == DDC_SUCCESS) pcm_data.ckSize += 6;
			}
			break;
		case 32:
			l = (int32_t)(LeftSample * 65536.0f);
			r = (int32_t)(RightSample * 65536.0f);
			retcode = psyfile_write(fp, &l, 4);
			if (retcode == DDC_SUCCESS) {
				retcode = psyfile_write(fp, &r, 4);
				//if (retcode == DDC_SUCCESS) pcm_data.ckSize += 8;
			}
			break;
		default:
			retcode = DDC_INVALID_CALL;
		}
		break;
	case 3: // IEEE float PCM
		if (wave_format->wBitsPerSample == 32)
		{
			f = LeftSample * 0.000030517578125f;
			retcode = psyfile_write(fp, &f, 4);
			if (retcode == DDC_SUCCESS)
			{
				f = RightSample * 0.000030517578125f;
				retcode = psyfile_write(fp, &f, 4);
				//if (retcode == DDC_SUCCESS) pcm_data.ckSize += 8;
			}
		}
	default:
		break;
	}
	return retcode;
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	FileOutDriver* self = (FileOutDriver*)driver;

	return self->configuration;
}
