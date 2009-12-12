#include <psycle/plugin_interface.hpp>
#include <psycle/helpers/math.hpp>
#include <psycle/helpers/math/pi.hpp>
#include <psycle/helpers/math/rint.hpp>
#include "SynthTrack.hpp"
#include <cstdlib>

using namespace psycle::plugin_interface;
using namespace psycle::helpers;

int const MAX_ENV_TIME = 250000;
int const NUMPARAMETERS = 28;
#define USE_NEW_WAVETABLES 1

CMachineParameter const paraOSC1wave = {
	"OSC1 Wave",
	"OSC1 Wave",
	0,				
	4,
	MPF_STATE,
	1
};

CMachineParameter const paraOSC2wave = {
	"OSC2 Wave",
	"OSC2 Wave",
	0,
	4,
	MPF_STATE,
	1
};


CMachineParameter const paraOSC2detune = {
	"OSC2 Detune",
	"OSC2 Detune",
	-36,
	36,
	MPF_STATE,
	0
};

CMachineParameter const paraOSC2finetune = {
	"OSC2 Finetune",
	"OSC2 Finetune",
	0,				
	256,
	MPF_STATE,
	27
};

CMachineParameter const paraOSC2sync = {
	"OSC2 Sync",
	"OSC2 Sync",
	0,
	1,
	MPF_STATE,
	0
};

CMachineParameter const paraVCAattack = {
	"VCA Attack",
	"VCA Attack",
	32,
	MAX_ENV_TIME,
	MPF_STATE,
	32
};


CMachineParameter const paraVCAdecay = {
	"VCA Decay",
	"VCA Decay",
	32,
	MAX_ENV_TIME,
	MPF_STATE,
	6341
};

CMachineParameter const paraVCAsustain = {
	"VCA Sustain",
	"VCA Sustain level",
	0,
	256,
	MPF_STATE,
	0
};


CMachineParameter const paraVCArelease = {
	"VCA Release",
	"VCA Release",
	32,
	MAX_ENV_TIME,
	MPF_STATE,
	2630
};


CMachineParameter const paraVCFattack = {
	"VCF Attack",
	"VCF Attack",
	32,
	MAX_ENV_TIME,
	MPF_STATE,
	589
};


CMachineParameter const paraVCFdecay = {
	"VCF Decay",
	"VCF Decay",
	32,
	MAX_ENV_TIME,
	MPF_STATE,
	2630
};

CMachineParameter const paraVCFsustain = {
	"VCF Sustain",
	"VCF Sustain level",
	0,
	256,
	MPF_STATE,
	0
};

CMachineParameter const paraVCFrelease = {
	"VCF Release",
	"VCF Release",
	32,
	MAX_ENV_TIME,
	MPF_STATE,
	2630
};

CMachineParameter const paraVCFlfospeed = {
	"VCF LFO Speed",
	"VCF LFO Speed",
	1,
	65536,
	MPF_STATE,
	32
};


CMachineParameter const paraVCFlfoamplitude = {
	"VCF LFO Amplitude",
	"VCF LFO Amplitude",
	0,				
	240,
	MPF_STATE,
	0
};

CMachineParameter const paraVCFcutoff = {
	"VCF Cutoff",
	"VCF Cutoff",
	0,				
	240,
	MPF_STATE,
	120
};

CMachineParameter const paraVCFresonance = {
	"VCF Resonance",
	"VCF Resonance",
	1,
	240,
	MPF_STATE,
	128
};

CMachineParameter const paraVCFtype = {
	"VCF Type",
	"VCF Type",
	0,
	19,
	MPF_STATE,
	0
};


CMachineParameter const paraVCFenvmod = 
{ 
	"VCF Envmod",
	"VCF Envmod",
	-240,
	240,
	MPF_STATE,
	80
};

CMachineParameter const paraOSCmix = {
	"OSC Mix",
	"OSC Mix",
	0,
	256,
	MPF_STATE,
	128
};

CMachineParameter const paraOUTvol = {
	"Volume",
	"Volume",
	0,				
	256,
	MPF_STATE,
	128
};

CMachineParameter const paraARPmode = {
	"Arpeggiator",
	"Arpeggiator",
	0,
	16,
	MPF_STATE,
	0
};

CMachineParameter const paraARPbpm = {
	"Arp. BPM",
	"Arp. BPM",
	32,
	1024,
	MPF_STATE,
	125
};


CMachineParameter const paraARPcount = {
	"Arp. Steps",
	"Arp. Steps",
	0,				
	16,
	MPF_STATE,
	4
};

CMachineParameter const paraGlobalDetune = {
	"Glb. Detune",
	"Global Detune",
	-36,
	36,
	MPF_STATE,
	1 
};
// Why is the default tuning +1 +60?
// Answer:
// The original implementation generated a wavetable of 2048 samples.
// At 44100Hz (which Asynth assumed), this is a ~21.5Hz signal.
// A standard tune is for A-5 (i.e, note 69) to be 440Hz, which is
// ~ 20.4335 times the sampled signal.
// log2 of this value is ~ 4.3528.
// Multiply this amount by 12 (notes/octave) to get 52.2344, which stands
// for note 52 and finetune 0.2344.
// The original compensation was using note-18 in the SeqTick() call ( 69-18 = 51 )
// So to correctly compensate, 1 seminote and fine of 60 is added (0.2344 * 256 ~ 60.01)
//
// With the new implementation where the wavetable is generated depending
// on the sampling rate, this correction is maintained for compatibility.

CMachineParameter const paraGlobalFinetune = {
	"Gbl. Finetune",
	"Global Finetune",
	-256,
	256,
	MPF_STATE,
	60
};

CMachineParameter const paraGlide = {
	"Glide Depth",
	"Glide Depth",
	0,				
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraInterpolation = {
	"Interpolation",
	"I",
	0,				
	1,
	MPF_STATE,
	0
};

CMachineParameter const *pParameters[] = {
	&paraOSC1wave,
	&paraOSC2wave,
	&paraOSC2detune,
	&paraOSC2finetune,
	&paraOSC2sync,
	&paraVCAattack,
	&paraVCAdecay,
	&paraVCAsustain,
	&paraVCArelease,
	&paraVCFattack,
	&paraVCFdecay,
	&paraVCFsustain,
	&paraVCFrelease,
	&paraVCFlfospeed,
	&paraVCFlfoamplitude,
	&paraVCFcutoff,
	&paraVCFresonance,
	&paraVCFtype,
	&paraVCFenvmod,
	&paraOSCmix,
	&paraOUTvol,
	&paraARPmode,
	&paraARPbpm,
	&paraARPcount,
	&paraGlobalDetune,
	&paraGlobalFinetune,
	&paraGlide,
	&paraInterpolation
};


CMachineInfo const MacInfo (
	MI_VERSION,				
	GENERATOR,
	NUMPARAMETERS,
	pParameters,
	"Arguru Synth 2 final"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Arguru Synth",
	"J. Arguelles (arguru)",
	"help",
	4
);

class mi : public CMachineInterface {
	public:
		mi();
		virtual ~mi();
		virtual void Init();
		virtual void SequencerTick();
		virtual void Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks);
		virtual bool DescribeValue(char* txt,int const param, int const value);
		virtual void Command();
		virtual void ParameterTweak(int par, int val);
		virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
		virtual void Stop();

	private:
		void InitWaveTableOrig();
		void InitWaveTableSR(bool delArray=false);
#if USE_NEW_WAVETABLES
		signed short *WaveTable[5];
#else
		signed short WaveTable[5][2050];
#endif
		std::uint32_t waveTableSize;
		float srCorrection;
		CSynthTrack track[MAX_TRACKS];
	
		SYNPAR globalpar;
		bool reinitChannel[MAX_TRACKS];
		std::uint32_t currentSR;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi() {
	Vals = new int[NUMPARAMETERS];
	//Initialize here only those things that don't depend on
	//external values (like sampling rate)
#if !USE_NEW_WAVETABLES
	InitWaveTableOrig();
#endif
	for(int i = 0; i < MAX_TRACKS; ++i) {
		track[i].setGlobalPar(&globalpar);
		reinitChannel[i] = false;
	}
}

mi::~mi() {
	delete[] Vals;
	// Destroy dinamically allocated objects/memory here
#if USE_NEW_WAVETABLES
	for (int i=0;i < 5; i++) {
		delete[] WaveTable[i];
	}
#endif
}

void mi::Init() {
	currentSR=pCB->GetSamplingRate();
	// Initialize your stuff here (you can use pCB here without worries)
#if USE_NEW_WAVETABLES
	InitWaveTableSR();
#endif
	for (int i = 0; i < MAX_TRACKS; ++i) {
		track[i].setSampleRate(currentSR, waveTableSize, srCorrection);
	}
}

void mi::Stop() {
	for(int c=0;c<MAX_TRACKS;c++)
		track[c].NoteOff(true);
}

void mi::SequencerTick() {
	if (currentSR != pCB->GetSamplingRate()) {
		Init();
		//force an update of all the parameters.
		ParameterTweak(-1,-1);
		Stop();
	}
	for (int i = 0; i < MAX_TRACKS; ++i) reinitChannel[i] = true;
}

void mi::ParameterTweak(int par, int val) {
	// Called when a parameter is changed by the host app / user gui
	if (par >= 0 ) Vals[par]=val;
	float multiplier = currentSR/44100.0f;

	if (par == 0) globalpar.pWave=WaveTable[Vals[0]];
	if (par == 1) globalpar.pWave2=WaveTable[Vals[1]];
	globalpar.osc2detune=Vals[2];
	globalpar.osc2finetune=Vals[3];
	globalpar.osc2sync=Vals[4];
	
	if (par == 5) globalpar.amp_env_attack=Vals[5]*multiplier;
	if (par == 6) globalpar.amp_env_decay=Vals[6]*multiplier;
	globalpar.amp_env_sustain=Vals[7];
	if (par == 8) globalpar.amp_env_release=Vals[8]*multiplier;

	if (par == 9) globalpar.vcf_env_attack=Vals[9]*multiplier;
	if (par == 10) globalpar.vcf_env_decay=Vals[10]*multiplier;
	globalpar.vcf_env_sustain=Vals[11];
	if (par == 12) globalpar.vcf_env_release=Vals[12]*multiplier;
	if (par == 13) globalpar.vcf_lfo_speed=Vals[13]*multiplier;
	globalpar.vcf_lfo_amplitude=Vals[14];

	globalpar.vcf_cutoff=Vals[15];
	globalpar.vcf_resonance=Vals[16];
	globalpar.vcf_type=Vals[17];
	globalpar.vcf_envmod=Vals[18];
	globalpar.osc_mix=Vals[19];
	globalpar.out_vol=Vals[20];
	globalpar.arp_mod=Vals[21];
	globalpar.arp_bpm=Vals[22];
	globalpar.arp_cnt=Vals[23];
	globalpar.globaldetune=Vals[24];
#if USE_NEW_WAVETABLES
	globalpar.globalfinetune=Vals[25]-60;
#else
	globalpar.globalfinetune=Vals[25];
#endif
	if (par == 26 && val > paraGlide.MaxValue) {
		Vals[26] = paraGlide.MaxValue;
	}
	globalpar.synthglide=Vals[26];
	globalpar.interpolate=Vals[27];
}

void mi::Command() {
	// Called when user presses editor button
	// Probably you want to show your custom window here
	// or an about button
	char buffer[2048];

	std::sprintf(
			buffer,"%s%s%s%s%s%s%s%s%s%s%s",
			"Pattern commands\n",
			"\n01xx : Pitch slide-up",
			"\n02xx : Pitch slide-down",
			"\n03xx : Pitch glide",
			"\n04xy : Vibrato [x=depth, y=speed]",
			"\n07xx : Change vcf env modulation [$00=-128, $80=0, $FF=+128]",
			"\n08xx : Change vcf cutoff frequency",
			"\n09xx : Change vcf resonance amount",
			"\n0Exx : NoteCut in xx*32 samples",
			"\n11xx : Vcf cutoff slide-up",
			"\n12xx : Vcf cutoff slide-down\0"
			);

	pCB->MessBox(buffer,"·-=<([aRgUrU's SYNTH 2 (Final)])>=-·",0);
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks) {
	float sl=0;
	// not used: float sr=0;

	for(int c=0;c<tracks;c++) {
		if(track[c].AmpEnvStage) {
			float *xpsamplesleft=psamplesleft;
			float *xpsamplesright=psamplesright;
			--xpsamplesleft;
			--xpsamplesright;
			
			int xnumsamples=numsamples;
		
			CSynthTrack *ptrack=&track[c];
			if(reinitChannel[c]) {
				ptrack->InitEffect(0,0);
				reinitChannel[c]=false;
			}

			if(ptrack->NoteCutTime >0) ptrack->NoteCutTime-=numsamples;
		
			ptrack->PerformFx();

			if(globalpar.osc_mix == 0) {
				do {
					sl=ptrack->GetSampleOsc1();
					*++xpsamplesleft+=sl;
					*++xpsamplesright+=sl;
				} while(--xnumsamples);
			}
			else if(globalpar.osc_mix == 256) {
				do {
					sl=ptrack->GetSampleOsc2();
					*++xpsamplesleft+=sl;
					*++xpsamplesright+=sl;
				} while(--xnumsamples);
			}
			else {
				do {
					sl=ptrack->GetSample();
					*++xpsamplesleft+=sl;
					*++xpsamplesright+=sl;
				} while(--xnumsamples);
			}
		}
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value) {
	// Oscillators waveform descriptions
	switch(param) {
	case 0: //fallthrough
	case 1:
			switch(value) {
				case 0:std::strcpy(txt,"Sine");return true;
				case 1:std::strcpy(txt,"Sawtooth");return true;
				case 2:std::strcpy(txt,"Square");return true;
				case 3:std::strcpy(txt,"Triangle");return true;
				case 4:std::strcpy(txt,"Random");return true;
			}
			break;
	case 3: //fallthrough
	case 25:
			std::sprintf(txt,"%.03f cts.",Vals[param]*0.390625f);
			return true;
	case 4:	//fallthrough
	case 27:
			std::strcpy(txt,(value==0)?"Off":"On");
			return true;
	case 5: //fallthrough
	case 6: //fallthrough
	case 8: //fallthrough
	case 9: //fallthrough
	case 10: //fallthrough
	case 12:
			std::sprintf(txt,"%.03f ms",Vals[param]*1000.0f/currentSR);
			return true;
	case 7:
	case 11:
	case 20:
		std::sprintf(txt,"%.1f%%",(float)value*0.390625f);
		return true;
	case 13:
		std::sprintf(txt,"%.03f Hz",(value*0.000005f)/(2.0f*math::pi_f)*(currentSR/64.0f));
		return true;
	case 17:
			switch(value) {
			case 0:std::strcpy(txt,"Lowpass A");return true;
			case 1:std::strcpy(txt,"Hipass A");return true;
			case 2:std::strcpy(txt,"Bandpass A");return true;
			case 3:std::strcpy(txt,"Bandreject A");return true;
			case 4:std::strcpy(txt,"ParaEQ1 A");return true;
			case 5:std::strcpy(txt,"InvParaEQ1 A");return true;
			case 6:std::strcpy(txt,"ParaEQ2 A");return true;
			case 7:std::strcpy(txt,"InvParaEQ2 A");return true;
			case 8:std::strcpy(txt,"ParaEQ3 A");return true;
			case 9:std::strcpy(txt,"InvParaEQ3 A");return true;
			case 10:std::strcpy(txt,"Lowpass B");return true;
			case 11:std::strcpy(txt,"Hipass B");return true;
			case 12:std::strcpy(txt,"Bandpass B");return true;
			case 13:std::strcpy(txt,"Bandreject B");return true;
			case 14:std::strcpy(txt,"ParaEQ1 B");return true;
			case 15:std::strcpy(txt,"InvParaEQ1 B");return true;
			case 16:std::strcpy(txt,"ParaEQ2 B");return true;
			case 17:std::strcpy(txt,"InvParaEQ2 B");return true;
			case 18:std::strcpy(txt,"ParaEQ3 B");return true;
			case 19:std::strcpy(txt,"InvParaEQ3 B");return true;
			}
			break;
	case 19:
			{
				float fv=(float)value*0.390625f;

				if ( value == 0 ) std::strcpy(txt,"Osc1");
				else if ( value == 256 ) std::strcpy(txt,"Osc2");
				else std::sprintf(txt,"%.1f%% : %.1f%%",100-fv,fv);
			}
			return true;
	case 21:
			switch(value) {
			case 0:std::strcpy(txt,"Off");return true;
			case 1:std::strcpy(txt,"Minor1");return true;
			case 2:std::strcpy(txt,"Major1");return true;
			case 3:std::strcpy(txt,"Minor2");return true;
			case 4:std::strcpy(txt,"Major2");return true;
			case 5:std::strcpy(txt,"Riffer1");return true;
			case 6:std::strcpy(txt,"Riffer2");return true;
			case 7:std::strcpy(txt,"Riffer3");return true;
			case 8:std::strcpy(txt,"Minor Bounce");return true;
			case 9:std::strcpy(txt,"Major Bounce");return true;
			}
			break;
	case 26:
			if (value == 0) {std::strcpy(txt, "Off"); return true;}
			break;
	default:
			break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// The SeqTick function is where your notes and pattern command handlers
// should be processed.
// Is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val) {
	track[channel].InitEffect(cmd,val);
	reinitChannel[channel]=false;

	// Global scope synth pattern commands
	switch(cmd)
	{
	case 7: // Change envmod
		globalpar.vcf_envmod=val-128;
	break;
	
	case 8: // Change cutoff
		globalpar.vcf_cutoff=val*0.5f;
	break;
	
	case 9: // Change reso
		globalpar.vcf_resonance=val*0.5f;
	break;
	}

	// Note Off == 120
	// Empty Note Row == 255
	// Less than note off value??? == NoteON!
	if(note<120)
		track[channel].NoteOn(note-18,(cmd == 0x0C)?(val>>2)&0x3F:64);

	// Note off
	else if(note==120)
		track[channel].NoteOff();
}

void mi::InitWaveTableOrig() {
	for(int c=0;c<2050;c++) {
		double sval=(double)c*0.00306796157577128245943617517898389;
		WaveTable[0][c]=int(sin(sval)*16384.0f);

		if(c < 2048) WaveTable[1][c]=(c*16)-16384;
		else WaveTable[1][c]=((c-2048)*16)-16384;

		if(c < 1024 || c >= 2048) WaveTable[2][c]=-16384;
		else WaveTable[2][c]=16384;

		if(c < 1024) WaveTable[3][c]=(c*32)-16384;
		else if(c < 2048) WaveTable[3][c]=16384-((c-1024)*32);
		else WaveTable[3][c]=((c-2048)*32)-16384;

		//This assumes MAX_RAND is 0x7fff
		WaveTable[4][c]= std::rand() - 16384;
	}

	waveTableSize = 2048;
	srCorrection = 1.0f;
}

//New method to generate the wavetables:
//
//Generate a signal of aproximately 22Hz for the current sampling rate.
//since it will not be an integer amount of samples, store the difference
//as a factor in srCorrection, so that it can be applied when calculating
//the OSC speed.
void mi::InitWaveTableSR(bool delArray) {
	//Ensure the value is even, we need two halfs.
	const std::uint32_t amount = math::rounded((float)currentSR/22.0f) &0xFFFFFFF8;
	const std::uint32_t half = amount >> 1;

	const double sinFraction = 2.0*math::pi/(double)amount;
	const float increase = 32768.0f/(float)amount;
	const float increase2 = 65536.0f/(float)amount;

	for (std::uint32_t i=0;i < 5; i++) {
		if (delArray) {
			delete WaveTable[i];
		}
		//Two more shorts allocated for the interpolation routine.
		WaveTable[i]=new signed short[amount+2];
	}
	for(std::uint32_t c=0;c<half;c++) {
		double sval=(double)c*sinFraction;
		WaveTable[0][c]=math::lrint<short int,double>(sin(sval)*16384.0);
		WaveTable[1][c]=(c*increase)-16384;
		WaveTable[2][c]=-16384;
		WaveTable[3][c]=(c*increase2)-16384;
		//This assumes MAX_RAND is 0x7fff
		WaveTable[4][c]= std::rand() - 16384;
	}
	for(std::uint32_t c=half;c<amount;c++) {
		double sval=(double)c*sinFraction;
		WaveTable[0][c]=math::lrint<short int,double>(sin(sval)*16384.0);
		WaveTable[1][c]=(c*increase)-16384;
		WaveTable[2][c]=16384;
		WaveTable[3][c]=16384-((c-half)*increase2);
		//This assumes MAX_RAND is 0x7fff
		WaveTable[4][c]= std::rand() - 16384;
	}
	//Two more shorts allocated for the interpolation routine.
	for (std::uint32_t i=0;i < 5; i++) {
		WaveTable[i][amount]=WaveTable[i][0];
		WaveTable[i][amount+1]=WaveTable[i][1];
	}


	waveTableSize = amount;
	srCorrection = (float)amount*22.0f / (float)currentSR;

}
