#ifndef _WAVEOUT_H
#define _WAVEOUT_H

// waveout stuff based on buzz code
#include <mmsystem.h>
#include "AudioDriver.h"

#define MAX_WAVEOUT_BLOCKS		8

class WaveOut : public AudioDriver
{
public:
	WaveOut();
	virtual ~WaveOut();
	virtual void Initialize(
		HWND hwnd,
		AUDIODRIVERWORKFN pCallback,
		void* context);
	virtual void Reset();
	virtual bool Enable(bool e);	
	virtual int GetWritePos();
	virtual int GetPlayPos();
	virtual void Configure();
	virtual bool Initialized() { return _initialized; };
	virtual bool Configured() { return _configured; };
	virtual AudioDriverInfo* GetInfo() { return &_info; };
	virtual int GetNumBuffers( void ){ return _numBlocks; };
	virtual int GetBufferSize( void ){ return _blockSize; };

private:
	class CBlock
	{
	public:
		HANDLE Handle;
		byte *pData;
		WAVEHDR *pHeader;
		HANDLE HeaderHandle;
		bool Prepared;
	};

	bool _initialized;
	bool _configured;
	static AudioDriverInfo _info;
	static AudioDriverEvent _event;
	static CCriticalSection _lock;

	HWAVEOUT _handle;
	int _deviceID;
	int _numBlocks;
	int _blockSize;
	int _currentBlock;
	int _writePos;
	int _pollSleep;
	int _dither;
	bool _running;
	bool _stopPolling;
	CBlock _blocks[MAX_WAVEOUT_BLOCKS];
	void* _callbackContext;
	AUDIODRIVERWORKFN _pCallback;

	static void PollerThread(void *pWaveOut);
	void ReadConfig();
	void WriteConfig();
	void Error(char const *msg);
	void DoBlocks();
	bool Start();
	bool Stop();
};

#endif