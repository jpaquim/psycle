//////////////////////////////////////////////////////////////////////
// CTrack.h include file
//
// Remarks
// -------
//
// envStage:
// 0 -> Off
// 1 -> Attack
// 2 -> Decay
// 3 -> Sustain
// 4 -> Release [Note Off]
// 5 -> Fast Release [Note Off Fast]

struct smpos
{
	unsigned __int32 last;
	unsigned __int32 first;
};

union s_access
{
	smpos half;
	unsigned __int64 full;
};

class CTrack
{
public:
	CTrack();
	void Generate(float *psamples, float *psamplesr,int numsamples, int interpol);
	void GenerateZero(float *psamples, int numsamples);	
	int Tick(int note, int inst, int cmd, int val);
	void NoteOff();
	void NoteOffFast();
	inline void GetEnvelopeLevel();
	inline void GetFilterEnvelopeLevel();

	int envStage;
	int d_inst;

private:

	int alteRand(int x);

	/* envelope Stage */
	int envCounter;
	
	float envCoef_FR;
	float envCoef;
	float envSustain;

	float fenvStep;
	float fenvValue;
	int fenvStage;
	float f_sl;
	float f_module;
	
	/* Filter */
	float tfcoef0;
	float tfcoef1;
	float tfcoef2;
	float tfcoef3;
	float tfcoef4;
	float trackVolume;
	int f_c;
	int f_q;
	int f_t;
	int f_e;
	int r_cutoff;
	float lfogr;
	int lfoval;
	float lfoamp;
	float lfosp;

	
	float x1,x2,y1,y2;
	float sx1,sx2,sy1,sy2;

	/* Tick Counter */
	int tickCounter;
	int triggerNoteoff;

	/* Wave Data */
	int d_wave;

	short *waveDataL;
	short *waveDataR;

	unsigned int waveLength;
	float waveVolumeCoef;
	float waveVolCoefL;
	float waveVolCoefR;
	bool waveStereo;
	bool waveLO;
	unsigned int waveLS;
	unsigned int waveLE;
	
	/* Resampling Stuff */
	s_access readPoint;
	signed __int64 readSpeed;

	inline float FilterL(float x)
	{
	float y;
	y= tfcoef0*x + tfcoef1*x1 + tfcoef2*x2 + tfcoef3*y1 + tfcoef4*y2;
	y2=y1;
	y1=y;
	x2=x1;
	x1=x;
	return y;
	}

	inline float FilterR(float x)
	{
	float y;
	y= tfcoef0*x + tfcoef1*sx1 + tfcoef2*sx2 + tfcoef3*sy1 + tfcoef4*sy2;
	sy2=sy1;
	sy1=y;
	sx2=sx1;
	sx1=x;
	return y;
	}
};

