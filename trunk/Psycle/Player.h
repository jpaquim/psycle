
#ifndef _PLAYER_H
#define _PLAYER_H
#include "constants.h"

#if !defined(_WINAMP_PLUGIN_)
	#include "Riff.h"
#endif

#include "Constants.h"

class Machine;

class Player
{
public:
	void SetSampleRate(int samprate);
	void AdvancePosition();
	bool _playBlock;
	bool _playing;
	bool _recording;
	bool _lineChanged;
	int _lineCounter;
	int _playPosition;
	int _playPattern;
	float _playTime;
	int _playTimem;
	int bpm;
	int tpb;
	int _ticksRemaining;
	bool _loopSong;

	Player();
	~Player();

	void Start(int pos,int line);
	void Stop(void);
#if defined(_WINAMP_PLUGIN_)
	float * Work(void* context, int& nsamples);
#else
	static float * Work(void* context, int& nsamples);
#endif // _WINAMP_PLUGIN_

	void StartRecording(char* psFilename,int bitdepth=16);
	void StopRecording(void);
protected:
	int prevMachines[MAX_TRACKS];

	float _pBuffer[MAX_DELAY_BUFFER];
#if !defined(_WINAMP_PLUGIN_)
	WaveFile _outputWaveFile;
#endif // ndef _WINAMP_PLUGIN_
	void ExecuteLine(void);
};

#endif