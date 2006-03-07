
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
	int _globalLine; // global line count durign song play
	int trackPatternID[MAX_PATTERNS]; // actual play time track ID , Sequence EDIT can change actual track pattern ID 
	int linePatternCounter[MAX_PATTERNS]; // every track pattern has it own line count!!!

	Player();
	~Player();

	void Start(int pos,int line);
	void Stop(void);
#if defined(_WINAMP_PLUGIN_)
	float * Work(void* context, int& nsamples);
#else
	static float * Work(void* context, int& nsamples);
#endif // _WINAMP_PLUGIN_

	void StartRecording(char* psFilename,int bitdepth=-1,int samplerate =-1, int channelmode =-1);
	void StopRecording(bool bOk = TRUE);
protected:
	int prevMachines[MAX_TRACKS];

	int backup_rate;
	int backup_bits;
	int backup_channelmode;

	float _pBuffer[MAX_DELAY_BUFFER];
#if !defined(_WINAMP_PLUGIN_)
	WaveFile _outputWaveFile;
#endif // ndef _WINAMP_PLUGIN_
	void ExecuteLine(void);
};

#endif