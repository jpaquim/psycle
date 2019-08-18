// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(DRIVER_H)
#define DRIVER_H

#include <windows.h>
#include <mmsystem.h>
#include <properties.h>

typedef float* (*AUDIODRIVERWORKFN)(void* context, int* numSamples);

#define MAX_WAVEOUT_BLOCKS 8

typedef struct {			
	HANDLE Handle;
	unsigned char *pData;
	WAVEHDR *pHeader;
	HANDLE HeaderHandle;
	int Prepared;
} CBlock;


typedef struct {
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
	Properties* properties;
	void* _callbackContext;
	AUDIODRIVERWORKFN _pCallback;
	int (*error)(int, const char*);
} Driver;

int driver_init(Driver*);
int driver_dispose(Driver*);
int driver_open(Driver*);
int driver_close(Driver*);
void driver_connect(Driver*, void* context, AUDIODRIVERWORKFN callback);



#endif
