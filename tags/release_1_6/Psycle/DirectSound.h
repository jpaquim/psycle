#ifndef _DIRECTSOUND_H
#define _DIRECTSOUND_H

#include <mmsystem.h>
#include <dsound.h>
#include "AudioDriver.h"

class DirectSound : public AudioDriver
{
public:
	DirectSound();
	virtual ~DirectSound();
	virtual void Initialize(
		HWND hwnd,
		AUDIODRIVERWORKFN pCallback,
		void* context);
	virtual void Reset();
	virtual bool Enable(
		bool e);
	virtual int GetWritePos();
	virtual int GetPlayPos();
	virtual void Configure();
	virtual bool Initialized() { return _initialized; };
	virtual bool Configured() { return _configured; };
	virtual AudioDriverInfo* GetInfo() { return &_info; };

private:
	bool _initialized;
	bool _configured;
	static AudioDriverInfo _info;

	HWND _hwnd;
	MMRESULT _timer;
	bool _running;
	bool _playing;
	bool _timerActive;
	static AudioDriverEvent _event;
	static CCriticalSection _lock;

	GUID _dsGuid;
	LPCGUID _pDsGuid;
	LPDIRECTSOUND _pDs;
	LPDIRECTSOUNDBUFFER _pBuffer;
	void* _callbackContext;
	AUDIODRIVERWORKFN _pCallback;

	int _deviceIndex;
	int _bytesPerSample;
	int _numBuffers;
	int _bufferSize;
	int _dsBufferSize;
	int _currentOffset;
	int _lowMark;
	int _highMark;
	int _buffersToDo;
	bool _exclusive;
	bool _dither;

	static void PollerThread(
		void* pDirectSound);
//	static void TimerCallback(UINT uTimerID, UINT uMsg, DWORD pDirectSound, DWORD dw1, DWORD dw2);
	void ReadConfig();
	void WriteConfig();
	void Error(
		char const *msg);
	void DoBlocks();
	bool Start();
	bool Stop();

};

#endif