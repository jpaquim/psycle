// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "fileoutdriver.h"

#include "../../detail/os.h"
#include "../../driver/driver.h"

#include <stdlib.h>
#include <string.h>

#include "waveio.h"
#include <fileio.h>

#if defined(DIVERSALIS__OS__MICROSOFT)	
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#endif

typedef struct {
	PsyFile file;
	char* path;
	uint32_t begin;
	uint32_t numsamplesbegin;
	uint32_t numsamples;
} FileContext;

typedef struct {
	psy_AudioDriver driver;
	int pollsleep;
	int stoppolling;
#if defined(DIVERSALIS__OS__MICROSOFT)	
	HANDLE hEvent;
#endif	
	FileContext filecontext;
} FileOutDriver;

static void driver_deallocate(psy_AudioDriver*);
static int fileoutdriver_init(FileOutDriver*);
static void driver_connect(psy_AudioDriver*, void* context, AUDIODRIVERWORKFN callback,
	void* handle);
static int driver_open(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, psy_Properties*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static unsigned int samplerate(psy_AudioDriver*);
static void PollerThread(void *fileoutdriver);
static void fileoutdriver_createfile(FileOutDriver*);
static void fileoutdriver_writebuffer(FileOutDriver*, float* pBuf,
	unsigned int numsamples);
static void fileoutdriver_closefile(FileOutDriver*);

static void init_properties(psy_AudioDriver* driver);

psy_AudioDriver* psy_audio_create_fileout_driver(void)
{
	FileOutDriver* out = malloc(sizeof(FileOutDriver));
	fileoutdriver_init(out);
	return &out->driver;
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver->dispose(driver);
	free(driver);
}

int fileoutdriver_init(FileOutDriver* self)
{
	memset(&self->driver, 0, sizeof(psy_AudioDriver));
	self->driver.open = driver_open;
	self->driver.deallocate = driver_deallocate;	
	self->driver.connect = driver_connect;
	self->driver.open = driver_open;
	self->driver.close = driver_close;
	self->driver.dispose = driver_dispose;
	self->driver.configure = driver_configure;
	self->driver.samplerate = samplerate;
	psy_signal_init(&self->driver.signal_stop);
	init_properties(&self->driver);
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
	properties_free(driver->properties);
	driver->properties = 0;
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

void driver_configure(psy_AudioDriver* driver, psy_Properties* config)
{
}

unsigned int samplerate(psy_AudioDriver* self)
{
	return 44100;
}

void init_properties(psy_AudioDriver* self)
{		
	self->properties = psy_properties_create();
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "name", "File psy_AudioDriver"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "vendor", "Psycedelics"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "version", "1.0"),
		PSY_PROPERTY_HINT_READONLY);	
	psy_properties_append_string(self->properties, "outputpath", "Untitled.wav");
}

void PollerThread(void* driver)
{	
	uintptr_t n;	
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
	psy_audio_WaveFormatChunk format;
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
	if (!psyfile_create(file, "untitled.wav", 1)) {
		return;
	}
	psyfile_write(file, "RIFF", 4);
	begin = psyfile_getpos(file);
	self->filecontext.begin = begin;
	temp32 = 0;
	psyfile_write(file, &temp32, sizeof(temp32));
	// Write Format Chunk
	format.wFormatTag = psy_audio_WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 2 * 44100 * bitspersample / 8;	
	format.nBlockAlign = 2 * bitspersample / 8;
	format.wBitsPerSample = bitspersample;	
	format.cbSize = 0;
	psyfile_write(file, "WAVEfmt ", 8);
	temp32 = 0;
	pcmbegin = psyfile_getpos(file);
	psyfile_write(file, &temp32, sizeof(temp32));	
	temp16 = format.wFormatTag;
	psyfile_write(file, &temp16, sizeof(temp16));
	temp16 = format.nChannels;
	psyfile_write(file, &temp16, sizeof(temp16));
	temp32 = format.nSamplesPerSec;
	psyfile_write(file, &temp32, sizeof(temp32));
	temp32 = format.nAvgBytesPerSec;
	psyfile_write(file, &temp32, sizeof(temp32));
	temp16 = format.nBlockAlign;
	psyfile_write(file, &temp16, sizeof(temp16));
	temp16 = format.wBitsPerSample;
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

void fileoutdriver_writebuffer(FileOutDriver* self, float* pBuf, unsigned int numsamples)
{
	uintptr_t i;
	uint16_t temp16;

	for (i = 0; i < numsamples;	++i) {		
		temp16 = (uint16_t) pBuf[i];
		psyfile_write(&self->filecontext.file, &temp16, sizeof(temp16));								
	}
	self->filecontext.numsamples += numsamples;
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
