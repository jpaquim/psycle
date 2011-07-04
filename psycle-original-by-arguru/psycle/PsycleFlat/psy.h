//////////////////////////////////////////////////////////////////////
// Machine index assignment
//
// 0 - Master
// 1 - PsychOsc
// 2 - Distortion

//////////////////////////////////////////////////////////////////////
// Psycle Architecture Include 0.01
//
#include "io.h"
#include "direct.h"
#include "machineinterface.h"
#include "math.h"
#include "filter.h"
#include "..\vst\Vst.h"
#define undenormalise(sample) if(((*(unsigned int*)&sample)&0x7f800000)==0) sample=0.0f
#include "riff.h"
	
class fxCB : public CFxCallback
{
public:
	virtual void MessBox(char* ptxt,char *caption,unsigned int type)
	{
		MessageBox(hWnd,ptxt,caption,type);
	}

	virtual int GetTickLength()
	{
		return cbSamplesPerTick;
	}

	virtual int GetSamplingRate()
	{
		return cbSamplesPerSec;
	}

	virtual int GetBeatsPerMin()
	{
		return cbBeatsPerMin;
	}

	virtual int CallbackFunc(int cbkID,int par1,int par2,int par3){return 0;}
	virtual float *GetWaveLData(int inst,int wave){return NULL;}
	virtual float *GetWaveRData(int inst,int wave){return NULL;}
	virtual int GetBPM(){return cbBeatsPerMin;}

	int cbBeatsPerMin;
	int cbSamplesPerTick;
	int cbSamplesPerSec;
	
public:
	HWND hWnd;
};

// CUBIC SPLINE FUNCTION
//
// Arguru
//

struct PSYCONFIG
{
	char mv_skin_path[256];
	char pv_skin_path[256];
	bool mv_skin;
	bool pv_skin;
	COLORREF  mv_colour;
	COLORREF  pv_colour;
	COLORREF gen_colour;
	COLORREF eff_colour;
	COLORREF mas_colour;
	COLORREF plg_colour;
	COLORREF vu1;
	COLORREF vu2;
	COLORREF vu3;
	int KEYBOARDMODE; // 0 - Standard, 1 - French
};

class Cubic
{
public:
	Cubic();

	inline float WorkLinear(float y0,float y1,unsigned __int32 res, unsigned __int64 offset, unsigned __int64 length)
	{
	if(offset+2>length)y1=0;
	return y0+(y1-y0)*lt[res>>21];
	}

	inline float Work(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length)
	{
	res=res>>21;
	if(offset==0)yo=0;
	if(offset+2>length)y1=0;
	if(offset+3>length)y2=0;
	return at[res]*yo+bt[res]*y0+ct[res]*y1+dt[res]*y2;
	}

	// Work function. Where all is cooked :]
	// yo = y[-1] [sample at x-1]
	// y0 = y[0]  [sample at x (input)]
	// y1 = y[1]  [sample at x+1]
	// y2 = y[2]  [sample at x+2]
	
	// res= distance between two neighboughs sample points [y0 and y1] 
	//      ,so [0...1.0]. You have to multiply this distance * RESOLUTION used
	//      on the  spline conversion table. [256 by default]
    // If you are using 256 is asumed you are using 8 bit decimal
	// fixed point offsets for resampling.

	// offset = sample offset [info to avoid go out of bounds on sample reading ]
	// offset = sample length [info to avoid go out of bounds on sample reading ]

private:
	int RESOLUTION; // Currently is 256, that's enough...
    float at[2048];
	float bt[2048];
	float ct[2048];
	float dt[2048];
	float lt[2048];

};

#define OVERLAPTIME	128

#include "ctrack.h"
#define MAX_BUZZFX_PLUGS	256
#define MAX_BUSES			64
#define MAX_CONNECTIONS		12
#define MAX_MACHINES		128
#define MAX_TRACKS			32
#define MAX_SUBTRACKS		8
#define MAX_WAVES			16
#define MAX_LINES			128
#define MAX_INSTRUMENTS		255
#define PREV_WAV_INS		254
#define MAX_DELAY_BUFFER	65536
#define LOCK_LATENCY		256
#define MAX_PATTERNS 128				// MAX NUM OF PATTERNS
#define MULTIPLY				160		//MAXTRACKS * 5
#define MULTIPLY2				20480	//MULTIPLY * MAX_LINES
#define MAX_PATTERN_BUFFER_LEN	2621440	// MULTIPLY2 * MAXPATTERNS
#define MAX_SONG_POSITIONS		128

#define TR_TRIGGERED	1
#define DPI				6.28318530717958647692528676655901f

typedef CMachineInfo const* (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

//////////////////////////////////////////////////////////////////////
// Master Machine
void GetMachineName(char *buf,int mac);

class vstmi
{
public:
	vstmi();
	~vstmi();

	void Init();
	void SequencerTick();
	void Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks,bool iseffect);

	bool DescribeValue(char* txt,int const param, int const value);
	void ParameterTweak(int par, int val);
	void SeqTick(int track, int note, int ins, int cmd, int val);
	void Stop();
	void NoteOff(int track);
	// VST Stuff
	void SendVstMidi();
	void SetVstInstance(int i);
	
	VstMidiEvent m_midievent;
	//VstMidiEvent m_midievent2;
	
	VstEvents m_events;

    CVst *pVST;
	int instance;
	bool ANY;
	bool replace;
	char midichannel;

private:


	unsigned char ln[MAX_TRACKS];

	AEffect* effect;
	float * outputs[2];
	float * inputs[2];
};

class psyGear
{
public:

	// Plugin --------------------------------------------------------
	vstmi ovst;

	CMachineInterface* mi;
	int buzzdll;
	int buzzpar;
	psyInit();
	bool delay_alloc;
	int type;
	int x;
	int y;
	int machineMode;
	char editName[16];
	int inputSource[MAX_CONNECTIONS];
	int outputDest[MAX_CONNECTIONS];
	float connectionVol[MAX_CONNECTIONS];
	bool conection[MAX_CONNECTIONS];
	bool inCon[MAX_CONNECTIONS];
	CPoint connectionPoint[MAX_CONNECTIONS];
	int numInputs;
	int numOutputs;
	float *samplesLeft;
	float *samplesRight;
	float lVol;
		
	float rVol;
	int panning;
	void changePan(int newpan);
	void Tick(unsigned char* pData);
	void TickFx();
	void BuzzInit(const CMachineInfo *mif);

	// Generator members
	CTrack trackObj[MAX_TRACKS][MAX_SUBTRACKS];
	int SubTrack[MAX_TRACKS];
	int currSubTrack[MAX_TRACKS];

	int numSubTracks;
	int interpol;
	void AllocateDelayBuffer();

	int GetFreeSubTrack(int trk);
	void IsolateSubTracks(int trk,int strk,int pinst);

	psyGear();
	~psyGear();

	int DSPVol(float *pSamplesL,float *pSamplesR,int numSamples);

	void Work(float *pSamplesL, float *pSamplesR, int numSamples,int numtracks);
	dllTweakAll(int numPars);

	// Master
	void MasterWork(float *pSamples,int numSamplesm);
	
	// Generator
	void WorkTrackerBusMono(float *pSamples, float *pSamplesR,int numSamples,int tracks);
	
	// Fx
	void WorkDistortion(float *pSamples,int numSamples);
	void WorkTestSine(float *pSamplesL,float *pSamplesR,int numSamples);
	void WorkDelay(float *pSamplesL,float *pSamplesR,int numSamples);
	void WorkFilter(float *pSamplesL,float *pSamplesR,int numSamples);
	void WorkFlanger(float *pSamplesL, float *pSamplesR, int numSamples);
	void WorkGainer(float *pSamplesL,float *pSamplesR,int numSamples);
	
	void UpdateDelay(int lt, int rt, int fdbckl, int fdbckr);
	void UpdateFilter();
	void UpdateFlanger();
	void SetPreset(int mm,int preset);
	
	float Filter2p_l(float input,float f,float q,float fa,float fb);
	float Filter2p_r(float input,float f,float q,float fa,float fb);
	
	bool Worked;
	float v;	

	int LMAX;
	int RMAX;
	float lmax;
	float rmax;
	bool clip;

//////////////////////////////////////////////////////////////////////
//	Gear attributes
	
	unsigned cpuCost;
	
	int outDry;
	int outWet;

	int distPosThreshold;
	int distPosClamp;
	int distNegThreshold;
	int distNegClamp;

	unsigned char sinespeed;
	unsigned char sineglide;
	unsigned char sinevolume;
	unsigned char sinelfospeed;
	unsigned char sinelfoamp;
	float sineSpeed;
	float sineVolume;
	float sineOsc;
	float sineRealspeed;
	float sineGlide;
	float sineLfospeed;
	float sineLfoamp;
	float sineLfoosc;
	
	float* delayBufferL;
	float* delayBufferR;
	int delayTimeL;
	int delayTimeR;
	int delayCounterL;
	int delayCounterR;
	int delayedCounterL;
	int delayedCounterR;
	int delayFeedbackL;
	int delayFeedbackR;

	int filterCutoff;
	int filterResonance;
	int filterLfospeed;
	int filterLfoamp;
	int filterLfophase;
	int filterMode;
	float f_bufl0;
	float f_bufl1;
	float f_bufr0;
	float f_bufr1;
	float f_cutoff;
	float f_resonance;
	float f_lfospeed;
	float f_lfoamp;
	float f_lfophase;

	int VolumeCounter;

	float lfoDegree;

	void sineTick(void);
	void DoPatternEffects(int ins,int cmd,int val,int trk);

};

//////////////////////////////////////////////////////////////////////
// Main Song class

class psySong
{
public:
	CCriticalSection door;

	fxCB micb;
	bool Tweaker;
	CVst m_Vst;
	
	WaveFile m_WaveFile;
	int m_WaveStage;
	char Wavefilename[32];

	void StartRecord(char *Wavefilename);
	void StopRecord();
	int WavAlloc(int instrument,int layer,const char * str);

	char LastPatternName[256];
	unsigned char pPatternData[MAX_PATTERN_BUFFER_LEN];
	HINSTANCE hBfxDll[MAX_BUZZFX_PLUGS];
	char BfxName[MAX_BUZZFX_PLUGS][256];
	int nBfxDll;
	psySong();
	~psySong();
	void newSong();
	void Reset(void);
	int GetFreeMachine(void);
	int GetIntDLL(char *dllname);
	bool CreateMachine(int machineType,int xl,int yl,int omac,int ndll);
	void DestroyMachine(int mac);
	void DestroyAllMachines();
	bool AllocNewPattern(int pattern,char *name,int lines);
	void DeleteAllPatterns(void);
	void DeleteInstrument(int i);
	void DeleteInstruments();
	void DeleteLayer(int i,int c);
	void SetBPM(int bpm,int srate);
	bool InsertConnection(int src,int dst);
	void Play(int line);
	void ReadBuzzfx(char *path);
	void DestroyBuzzfx(void);
	void ExecuteLine();
	int GetNumPatternsUsed();
	int GetFreeBus();

	void Stop();
	char Name[64];								// Song Name
	char Author[64];							// Song Author
	char Comment[256];							// Song Comment

	unsigned cpuIdle;
	unsigned CPUHZ;

	int LastPatternLines;

	int BeatsPerMin;
	int SamplesPerTick;
	int TicksRemaining;
	int LineCounter;
	bool LineChanged;
	
	bool Invalided;

	unsigned char currentOctave;

	// Buses data
	unsigned char busMachine[MAX_BUSES];

	// Pattern data
	int editPattern;
	int playPattern;
	int playLength;
	
	int editPosition;
	int playPosition;
	unsigned char playOrder[MAX_SONG_POSITIONS];

	int PlayMode;

	int patternLines[MAX_PATTERNS];
	char patternName[MAX_PATTERNS][32];
	int SONGTRACKS;

	// InstrumentData
	int instSelected;
	char instName[MAX_INSTRUMENTS][32];

	//////////////////////////////////////////////////////////////////
	// Loop stuff

	bool instLoop[MAX_INSTRUMENTS];
	int instLines[MAX_INSTRUMENTS];

	//////////////////////////////////////////////////////////////////
	// NNA values overview:
	//
	// 0 = Note Cut			[Fast Release 'Default']
	// 1 = Note Release		[Release Stage]
	// 2 = Note Continue	[No NNA]
	unsigned char instNNA[MAX_INSTRUMENTS];
	
	//////////////////////////////////////////////////////////////////
	// Amplitude Envelope overview:
	//
	int instENV_AT[MAX_INSTRUMENTS];	// Attack Time [Samples]
	int instENV_DT[MAX_INSTRUMENTS];	// Decay Time [Samples]
	int instENV_SL[MAX_INSTRUMENTS];	// Sustain Level [100=1.0]
	int instENV_RT[MAX_INSTRUMENTS];	// Release Time [Samples]
	
	// Filter 
	int instENV_F_AT[MAX_INSTRUMENTS];	// Attack Time [Samples]
	int instENV_F_DT[MAX_INSTRUMENTS];	// Decay Time [Samples]
	int instENV_F_SL[MAX_INSTRUMENTS];	// Sustain Level [32768 max]
	int instENV_F_RT[MAX_INSTRUMENTS];	// Release Time [Samples]

	int instENV_F_CO[MAX_INSTRUMENTS];	// Cutoff Frequency [32768]
	int instENV_F_RQ[MAX_INSTRUMENTS];	// Resonance [0-127]
	int instENV_F_EA[MAX_INSTRUMENTS];	// EnvAmount [0-32768]
	int instENV_F_TP[MAX_INSTRUMENTS];	// Filter Type [0-4]

	int instPAN[MAX_INSTRUMENTS];
	bool instRPAN[MAX_INSTRUMENTS];
	bool instRCUT[MAX_INSTRUMENTS];
	bool instRRES[MAX_INSTRUMENTS];
	bool track_st[MAX_TRACKS];

	// WaveData ------------------------------------------------------
	int waveSelected;
	char waveName[MAX_INSTRUMENTS][MAX_WAVES][32];
	unsigned short waveVolume[MAX_INSTRUMENTS][MAX_WAVES];
	signed short *waveDataL[MAX_INSTRUMENTS][MAX_WAVES];
	signed short *waveDataR[MAX_INSTRUMENTS][MAX_WAVES];
	unsigned int waveLength[MAX_INSTRUMENTS][MAX_WAVES];
	unsigned int waveLoopStart[MAX_INSTRUMENTS][MAX_WAVES];
	unsigned int waveLoopEnd[MAX_INSTRUMENTS][MAX_WAVES];
	int waveFinetune[MAX_INSTRUMENTS][MAX_WAVES];

	bool waveLoopType[MAX_INSTRUMENTS][MAX_WAVES];
	bool waveStereo[MAX_INSTRUMENTS][MAX_WAVES];

	// Machines ------------------------------------------------------
	bool Activemachine[MAX_MACHINES];
	psyGear* machine[MAX_MACHINES];

	int seqBus;
	int patTrack;

	// Previews waving

	void PW_Work(float *psamplesL, float *pSamplesR, int numSamples);
	void PW_Play();
	
	int PW_Phase;
	int PW_Stage;
	int PW_Length;
};
