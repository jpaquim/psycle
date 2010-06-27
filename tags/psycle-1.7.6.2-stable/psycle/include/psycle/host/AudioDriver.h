#ifndef _AUDIODRIVER_H
#define _AUDIODRIVER_H

typedef float* (*AUDIODRIVERWORKFN)(void* context, int& numSamples);

class AudioDriverInfo
{
public:
	char const *_psName;
};

class AudioDriverEvent : public CEvent
{
public:
	AudioDriverEvent() : CEvent(FALSE, TRUE) {}
};

class AudioDriver
{
public:
	AudioDriver() { _samplesPerSec = 44100; _bitDepth = 16; _channelmode = 3;}
	virtual ~AudioDriver() {};

	virtual void Initialize(
		HWND hwnd,
		AUDIODRIVERWORKFN pCallback,
		void* context) {};
	virtual void Reset(
		void) {};
	virtual bool Enable(
		bool e) { return false; };	
	virtual int GetWritePos(
		void) { return 0; };
	virtual int GetPlayPos(
		void) { return 0; };
	virtual int GetNumBuffers(
		void) { return 0; };
	virtual int GetBufferSize(
		void) { return 0; };
	virtual void Configure(
		void) {};
	virtual bool Initialized(
		void) { return true; };
	virtual bool Configured(
		void) { return true; };
	virtual AudioDriverInfo* GetInfo(
		void) { return &_info; };
	
	static void QuantizeWithDither(
		float *pin,
		int *piout,
		int c);
	static void Quantize(
		float *pin,
		int *piout,
		int c);

public:
	int _samplesPerSec;
	int _channelmode;
	int _bitDepth;

protected:
	static AudioDriverInfo _info;


};

#endif