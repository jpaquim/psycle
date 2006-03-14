/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

#ifndef _PLAYER_H
#define _PLAYER_H


#include "constants.h"
#include "iPlayer.h"

#if !defined(_WINAMP_PLUGIN_)
	#include "Riff.h"
#endif

#include "Constants.h"

class Machine;

class Player : public iPlayer
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

	void Start(const int pos,const int line);
	void Stop();

	const bool IsPlaying(){return _playing;};
	void IsPlaying(const bool value){_playing = value;};

	const int Bpm(){return bpm;};
	void Bpm(const int value){bpm = value;};

	const int Tpb(){return tpb;};
	void Tpb(const int value){tpb = value;};

	static float * Work(void* context, int& nsamples);

	void StartRecording(const TCHAR* psFilename,int bitdepth=-1,int samplerate =-1, int channelmode =-1);
	void StopRecording(bool bOk = TRUE);
protected:
	int prevMachines[MAX_TRACKS];

	int backup_rate;
	int backup_bits;
	int backup_channelmode;

	float _pBuffer[MAX_DELAY_BUFFER];

	WaveFile _outputWaveFile;

	void ExecuteLine(void);
};

#endif