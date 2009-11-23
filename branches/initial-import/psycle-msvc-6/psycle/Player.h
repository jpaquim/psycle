
#ifndef _PLAYER_H
#define _PLAYER_H

#include "Riff.h"

class Machine;

class Player
{
public:
	bool _playBlock;
	bool _playing;
	bool _recording;
	bool _lineChanged;
	int _lineCounter;
	int _playPosition;
	int _playPattern;
	int bpm;
	int tpb;

	Player();
	~Player();

	void Start(int line);
	void Stop(void);
#if defined(_WINAMP_PLUGIN_)
	float * Work(void* context, int& nsamples);
#else
	static float * Work(void* context, int& nsamples);
#endif // _WINAMP_PLUGIN_

	void StartRecording(char* psFilename);
	void StopRecording(void);
protected:
	int _ticksRemaining;
	int prevMachines[/*MAX_TRACKS*/32];

	float _pBuffer[65536];
#if !defined(_WINAMP_PLUGIN_)
	WaveFile _outputWaveFile;
#endif // ndef _WINAMP_PLUGIN_
	void ExecuteLine(void);
};

#endif