#ifndef _AUDIODRIVER_H
#define _AUDIODRIVER_H

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

#define ADF_STEREO 1

class AudioDriverInfo
{
public:
	char const *_psName;
};

class AudioDriver
{
public:
	virtual ~AudioDriver() {};
	virtual void Initialize(HWND hwnd, float *(*pCallback)(int &numSamples)) {};
	virtual void Reset() {};
	virtual bool Enable(bool e) { return false; };	
	virtual int GetWritePos() { return 0; };
	virtual int GetPlayPos() { return 0; };
	virtual void Configure() {};
	virtual bool Initialized() { return true; };
	virtual bool Configured() { return true; };
	virtual AudioDriverInfo* GetInfo() { return NULL; };

public:
	int _samplesPerSec;
	int _flags;

	void (*ReadProfileString)(char const *key, char *value);
	void (*WriteProfileString)(char const *key, char const *value);
	int (*ReadProfileInt)(char const *key, int const value);
	void (*WriteProfileInt)(char const *key, int const value);

protected:
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