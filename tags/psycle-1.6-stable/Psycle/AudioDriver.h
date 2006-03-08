#ifndef _AUDIODRIVER_H
#define _AUDIODRIVER_H

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

#define ADF_STEREO 1

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
	AudioDriver() { _samplesPerSec = 44100; }
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

public:
	int _samplesPerSec;
	int _flags;

protected:
	static AudioDriverInfo _info;

	static void QuantizeWithDither(
		float *pin,
		int *piout,
		int c);
	static void Quantize(
		float *pin,
		int *piout,
		int c);
};

#endif