#pragma once

#include "AudioDriver.h"
#include "PortAudio.h"

class PortAudioASIO : public AudioDriver
{

public:
	PortAudioASIO(void);
	virtual ~PortAudioASIO(void);
	virtual void Initialize(
		HWND hwnd,
		AUDIODRIVERWORKFN pCallback,
		void* context);
	virtual void Configure();
	virtual bool Initialized() { return _initialized; };
	virtual bool Configured() { return _configured; };
	virtual int GetNumBuffers( void ){ return _numBlocks; };
	virtual int GetBufferSize( void ){ return _blockSize; };
	virtual AudioDriverInfo* GetInfo() { return &_info; };
	virtual void Reset();
	virtual bool Enable(bool e);
	virtual int GetWritePos();
	virtual int GetPlayPos();

private:
	void Error(char const *msg);
	void ReadConfig();
	void WriteConfig();
	bool Start();
	bool Stop();
	static int paCallback(void *inputBuffer, void *outputBuffer,
					unsigned long framesPerBuffer,
					PaTimestamp outTime, void *userData );
	

	static AudioDriverInfo		_info;
	static CCriticalSection		_lock;

	bool	_initialized;
	bool	_configured;
	bool	_running;

	int		_deviceID;
	int		_numBlocks;
	int		_blockSize;
	int		_dither;

	PaError				err;
	PortAudioStream		*stream;

	void*				_callbackContext;
	AUDIODRIVERWORKFN	_pCallback;
};
