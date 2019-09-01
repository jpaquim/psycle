// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

/*

#ifdef __cplusplus
  #define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif

#if !defined(MME_H)
#define MME_H

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


typedef EXPORT int (__cdecl *pfndriver_init)(Driver*);
typedef EXPORT int (__cdecl *pfndriver_dispose)(Driver*);
typedef EXPORT int (__cdecl *pfndriver_open)(Driver*);
typedef EXPORT int (__cdecl *pfndriver_close)(Driver*);
typedef EXPORT void (__cdecl *pfndriver_connect)(Driver*, void* context, AUDIODRIVERWORKFN callback);

EXPORT int __cdecl driver_init(Driver*);
EXPORT int __cdecl driver_dispose(Driver*);
EXPORT int __cdecl driver_open(Driver*);
EXPORT int __cdecl driver_close(Driver*);
EXPORT void __cdecl driver_connect(Driver*, void* context, AUDIODRIVERWORKFN callback);


#endif

*/
