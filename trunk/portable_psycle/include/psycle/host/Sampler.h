#pragma once

#include "Machine.h"
#include "Filter.h"

#define SAMPLER_MAX_POLYPHONY		16
#define SAMPLER_DEFAULT_POLYPHONY	8

#define SAMPLER_CMD_NONE			0x00
#define SAMPLER_CMD_PORTAUP			0x01
#define SAMPLER_CMD_PORTADOWN		0x02
#define SAMPLER_CMD_PORTA2NOTE		0x03
#define SAMPLER_CMD_PANNING			0x08
#define SAMPLER_CMD_OFFSET			0x09
#define SAMPLER_CMD_VOLUME			0x0c
#define SAMPLER_CMD_RETRIG			0x15
#define SAMPLER_CMD_EXTENDED		0x0e
#define SAMPLER_CMD_EXT_NOTEOFF		0xc0
#define SAMPLER_CMD_EXT_NOTEDELAY	0xd0


typedef enum
{
	ENV_OFF = 0,
	ENV_ATTACK = 1,
	ENV_DECAY = 2,
	ENV_SUSTAIN = 3,
	ENV_RELEASE = 4,
	ENV_FASTRELEASE = 5
}
EnvelopeStage;

typedef enum
{
	INTERPOL_NONE = 0,
	INTERPOL_LINEAR = 1,
	INTERPOL_SPLINE = 2
}
InterpolationType;

typedef struct
{
	short* _pL;
	short* _pR;
	bool _stereo;
	ULARGE_INTEGER _pos;
	signed __int64 _speed;
	bool _loop;
	ULONG _loopStart;
	ULONG _loopEnd;
	ULONG _length;
	float _vol;
	float _lVolDest;
	float _rVolDest;
	float _lVolCurr;
	float _rVolCurr;
} WaveData;

typedef struct
{
	EnvelopeStage _stage;
	float _value;
	float _step;
	float _attack;
	float _decay;
	float _sustain;
	float _release;
}
Envelope;

typedef struct
{
	Envelope _filterEnv;
	Envelope _envelope;
	int _tickCounter;
	int _triggerNoteOff;
	int _triggerNoteDelay;
	int _instrument;
	WaveData _wave;
	Filter _filter;
	int _cutoff;
	float _coModify;
	int _channel;
	int effVal;
//	int effPortaNote;
	int effCmd;
	int effretMode;
	int effretTicks;
	float effretVol;
	int effOld;
}
Voice;

#if !defined(_WINAMP_PLUGIN_)
	class CGearTracker;
#endif

class Sampler : public Machine
{
public:
	void Tick();
	Sampler(int index);

	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual void Stop(void);
	virtual void Tick(int channel, PatternEntry* pData);
	virtual char* GetName(void) { return _psName; };
	virtual bool Load(RiffFile* pFile);
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
	{
		UINT size;
		pFile->Read(&size,sizeof(size));
		if (size)
		{
			if (version > CURRENT_FILE_VERSION_MACD)
			{
				// data is from a newer format of psycle, it might be unsafe to load.
				pFile->Skip(size);
				return FALSE;
			}
			else
			{
				int temp;
				pFile->Read(&temp, sizeof(temp)); // numSubtracks
				_numVoices=temp;
				pFile->Read(&temp, sizeof(temp)); // quality

				switch (temp)
				{
				case 2:
					_resampler.SetQuality(RESAMPLE_SPLINE);
					break;
				case 0:
					_resampler.SetQuality(RESAMPLE_NONE);
					break;
				default:
				case 1:
					_resampler.SetQuality(RESAMPLE_LINEAR);
					break;
				}
			}
		}
		return TRUE;
	};

#if !defined(_WINAMP_PLUGIN_)
	virtual void SaveSpecificChunk(RiffFile* pFile) 
	{
		int temp;
		UINT size = 2*sizeof(temp);
		pFile->Write(&size,sizeof(size));
		temp = _numVoices;
		pFile->Write(&temp, sizeof(temp)); // numSubtracks
		switch (_resampler._quality)
		{
		case RESAMPLE_NONE:
			temp = 0;
			break;
		case RESAMPLE_LINEAR:
			temp = 1;
			break;
		case RESAMPLE_SPLINE:
			temp = 2;
			break;
		}
		pFile->Write(&temp, sizeof(temp)); // quality
	};
#endif // ndef _WINAMP_PLUGIN_

	void Update(void);

protected:
	#if !defined(_WINAMP_PLUGIN_)
		friend CGearTracker;
	#endif

	static char* _psName;
	int _numVoices;
	Voice _voices[SAMPLER_MAX_POLYPHONY];
	Cubic _resampler;

	void PerformFx(int voice);
	void VoiceWork(int numsamples, int voice);
	void NoteOff(int voice);
	void NoteOffFast(int voice);
	int VoiceTick(int channel, PatternEntry* pData);
	inline void TickEnvelope(int voice);
	inline void TickFilterEnvelope(int voice);
	unsigned char lastInstrument[MAX_TRACKS];
	static inline int alteRand(int x)
	{
		return (x*rand())/32768;
	};
};
