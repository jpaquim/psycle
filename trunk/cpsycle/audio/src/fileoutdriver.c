// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "fileoutdriver.h"
#include <stdlib.h>
#include <string.h>

#include "waveio.h"
#include <fileio.h>

#include <windows.h>
#include <mmsystem.h>
#include <process.h>


typedef struct {
	PsyFile file;
	char* path;
	uint32_t begin;
	uint32_t numsamplesbegin;
	uint32_t numsamples;
} FileContext;

typedef struct {
	Driver driver;
	int pollsleep;
	int stoppolling;
	HANDLE hEvent;
	FileContext filecontext;
} FileOutDriver;

static void driver_free(Driver*);
static int fileoutdriver_init(FileOutDriver*);
static void driver_connect(Driver*, void* context, AUDIODRIVERWORKFN callback,
	void* handle);
static int driver_open(Driver*);
static void driver_configure(Driver*, Properties*);
static int driver_close(Driver*);
static int driver_dispose(Driver*);
static unsigned int samplerate(Driver*);
static void PollerThread(void *fileoutdriver);
static void fileoutdriver_createfile(FileOutDriver*);
static void fileoutdriver_writebuffer(FileOutDriver*, float* pBuf,
	unsigned int numsamples);
static void fileoutdriver_closefile(FileOutDriver*);

static void init_properties(Driver* driver);

Driver* create_fileout_driver(void)
{
	FileOutDriver* out = malloc(sizeof(FileOutDriver));
	fileoutdriver_init(out);
	return &out->driver;
}

void driver_free(Driver* driver)
{
	free(driver);
}

int fileoutdriver_init(FileOutDriver* self)
{
	memset(&self->driver, 0, sizeof(Driver));
	self->driver.open = driver_open;
	self->driver.free = driver_free;	
	self->driver.connect = driver_connect;
	self->driver.open = driver_open;
	self->driver.close = driver_close;
	self->driver.dispose = driver_dispose;
	self->driver.configure = driver_configure;
	self->driver.samplerate = samplerate;
	signal_init(&self->driver.signal_stop);
	init_properties(&self->driver);
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	self->pollsleep = 1;
	self->stoppolling = 0;
	self->filecontext.path = strdup("Untitled.wav");
	return 0;
}

int driver_dispose(Driver* driver)
{
	FileOutDriver* self;

	self = (FileOutDriver*) driver;
	properties_free(driver->properties);
	driver->properties = 0;
	signal_dispose(&driver->signal_stop);
	free(self->filecontext.path);
	self->filecontext.path = 0;
	return 0;
}

void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback,
	void* handle)
{
	driver->_pCallback = callback;
	driver->_callbackContext = context;
}

int driver_open(Driver* driver)
{	
	FileOutDriver* self;

	self = (FileOutDriver*) driver;
	self->stoppolling = 0;
	ResetEvent(self->hEvent);
	_beginthread(PollerThread, 0, self);
	return 0;
}

int driver_close(Driver* driver)
{
	FileOutDriver* self = (FileOutDriver*) driver;

	self->stoppolling = 1;	
	WaitForSingleObject(self->hEvent, INFINITE);
	return 0;
}

void driver_configure(Driver* driver, Properties* config)
{
}

unsigned int samplerate(Driver* self)
{
	return 44100;
}

void init_properties(Driver* self)
{		
	self->properties = properties_create();
	properties_sethint(
		properties_append_string(self->properties, "name", "File Driver"),
		PROPERTY_HINT_READONLY);
	properties_sethint(
		properties_append_string(self->properties, "vendor", "Psycedelics"),
		PROPERTY_HINT_READONLY);
	properties_sethint(
		properties_append_string(self->properties, "version", "1.0"),
		PROPERTY_HINT_READONLY);	
	properties_append_string(self->properties, "outputpath", "Untitled.wav");
}

void PollerThread(void* driver)
{	
	uintptr_t n;	
	uintptr_t blocksize = 4096;
	int hostisplaying = 1;

	FileOutDriver* self = (FileOutDriver*) driver;
	SetThreadPriority(GetCurrentThread(),
		THREAD_PRIORITY_ABOVE_NORMAL);
	fileoutdriver_createfile(self);	
	while (!self->stoppolling && hostisplaying)
	{
		float *pBuf;
		n = blocksize / 2;
		
		pBuf = self->driver._pCallback(self->driver._callbackContext, &n,
			&hostisplaying);		
		fileoutdriver_writebuffer(self, pBuf, blocksize);
		self->filecontext.numsamples += blocksize;
		Sleep(self->pollsleep);
	}
	fileoutdriver_closefile(self);	
	SetEvent(self->hEvent);	
	signal_emit(&self->driver.signal_stop, self, 0);
	_endthread();
}

void fileoutdriver_createfile(FileOutDriver* self)
{	
	WaveFormatChunk format;
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
	format.wFormatTag = WAVE_FORMAT_PCM;
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
