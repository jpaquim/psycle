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
	virtual void Initialize(HWND hwnd, float *(*pCallback)(int &numSamples));
	virtual void Reset();
	virtual bool Enable(bool e);	
	virtual int GetWritePos();
	virtual int GetPlayPos();
	virtual void Configure();
	virtual bool Initialized() { return _initialized; };
	virtual bool Configured() { return _configured; };
	virtual AudioDriverInfo* GetInfo() { return &_info; };

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
	float *(*_pCallback)(int &numSamples);

	static void PollerThread(void *pWaveOut);
	void ReadConfig();
	void WriteConfig();
	void Error(char const *msg);
	void DoBlocks();
	bool Start();
	bool Stop();
};

#endif