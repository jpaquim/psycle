#ifndef __BUZZ_AUDIO_DRIVER_H
#define __BUZZ_AUDIO_DRIVER_H

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

#define ADF_STEREO		1		// this is currently required by buzz

class CAudioDriver
{
public:
	virtual ~CAudioDriver() {};
	virtual void Initialize(dword hwnd, float *(*pcallback)(int &numsamples)) {};
	virtual void Reset() {};
	virtual bool Enable(bool e) { return false; };	
	virtual int GetWritePos() { return 0; };
	virtual int GetPlayPos() { return 0; };
	virtual void Configure() {};

public:
	int SamplesPerSec;
	int Flags;

	void (*ReadProfileString)(char const *key, char *value);
	void (*WriteProfileString)(char const *key, char const *value);
	int (*ReadProfileInt)(char const *key, int const value);
	void (*WriteProfileInt)(char const *key, int const value);
};

class CAudioDriverInfo
{
public:
	char const *Name;
};


#endif