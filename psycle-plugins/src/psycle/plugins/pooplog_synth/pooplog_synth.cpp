/* -*- mode:c++, indent-tabs-mode:t -*- */
/*

Version History

v0.68b
5/21/2003
- arpeggio works correctly with fast envelopes now

v0.67b
5/19/2003
- more anti-denormal code

v0.66b
3:28 AM 12/20/2002
- optimizations adjusted for safety

v0.65b
10:10 PM 11/22/2002
- fixed denormals in filters
- made so works with any sample rate
- fixed note-cut bug
- fixed sticky envelopes bug

v0.64b
10:02 PM 11/12/2002
- fixed the help screen
- sync now works as it should, sync selects the osc to PASS sync to

v0.63b
2:00 AM 10/24/2002
- updated so can be used with 64 track version of psycle that is apparenty coming soon
- fixed handling of BPM changes so it syncs to songs with swings.

v0.62b
1:49 PM 10/14/2002
- fixed cutoff being lost when changing filters
- fixed moog filter B being able to blow up by compressing feedback if it is over 0db
- fixed global tuning due to popular request - you may have to tweak your songs back

v0.61b
11:29 AM 17/04/2002
- improved accuracy of WM tune
- added note delay and retrigger

v0.60b
8:56 AM 14/04/2002
- added tracker arpeggio rate command
- fixed a bug with wm where it could overflow the waveform position index
- added delays to the vibrato and tremolo

v0.59b
9:22 AM 13/04/2002
- since everyone complained so much, i fixed the WM detuning.  I kinda liked it.
- added tracker style arpeggios
- fixed some text in tracker command help window
- removed warning screen. hopefully i won't need to put it back in.

v0.58b
6:09 PM 15/03/2002
- fixed an inevitable bug with the tweak inertia knob not working

v0.57b
2:39 PM 14/03/2002
- added the long awaited tweak inertia knob

v0.56b
1:04 PM 08/03/2002
- did some major optimizations, including making internal range -1.0 to 1.0
- removed all pointer access and uses arrays instead for speed
- overhauled some overdrives
- fixed multiplication mix modes
- separated antialias filter from filter struct for speed reasons
- changed default osc vol to 100%
- fixed divide mix mode to max at 200%

v0.55b
6:09 PM 05/03/2002
- fixed all lfos that i forgot to update to new sync method
- fixed vcf lfo depth display
- added more vcf routings ( sorry if you have to change it in your settings )
- fixed it so that syncs are polled each seq tick

v0.54b
1:11 PM 05/03/2002
- increased accuracy of lfo wrapping
- fixed sync times and expanded amounts
- fixed parameter update rate to be correct (it was updating on 33 samples, not the desired 32)
- fixed display of lfo rates to be more accurate
- recoded arpeggio rate code to be more accurate
- added more arpeggio sync rates so it behaves like lfos
- did some more optimizing

v0.53b
3:35 PM 04/03/2002
- fixed tremolo depth-it was behaving very weird
- fixed zipper noise on vca
- increased resolution of source waveforms to float for greater speed and accuracy
- removed unused portion of source waveforms

v0.52b
1:40 PM 03/03/2002
- made 0f00 the cancel command and ignore 0000
- fixed it so that envelopes update per sample to get rid of vca zipper noise

v0.51b
12:41 PM 03/03/2002
- improved interpolation algorithm to provide better antialiasing
- made antialias filter reset when switched to "off"
- did some major performance optimizations (f2i)
- made oscilator phase cycle much more accurate

v0.45b-0.50b
- internal versions
- fixed bug with vcf mix modes not being described correctly
- added ksn's arpeggio patterns
- switched to new save method
- fixed envelope bypass function
- made cutoff range wider

v0.44b
2:13 PM 28/02/2002
- made it so the splash message doesn't pop up on reinit
- added some track effects, and fixed some broken ones
- song sync rate initializes correctly on new synth

v0.43b
6:33 PM 26/02/2002
- fixed bug where ultralight interpolate was screwey
- now syncs waveform phases on noteon if portamento is off and no note playing
- fixed bug where arpeggios stopped on short envelopes

v0.42b
2:31 AM 26/02/2002
- added 'ultralight' version
- made it so that filters don't reset if knob touched when they don't need to
- fixed bug where vcf 2 crashed due to bad lfo wave pointer
- fixed bug where tweaking global tune parameter didn't affect realtime
- fixed bug where tweaking osc volume parameters didn't affect realtime
- fixed bug where silence waveform wasn't silent
- removed silent waveform from lfo selections

v0.41b
1:17 PM 25/02/2002
- added 'light' version
- fixed a bug with using more than 2 oscs - noteoffs were writing data to bad parts of ram
- fixed width knob, works in correct direction
- limited feedback on moog type a to 128x max so it doesn't explode any more on full res
- added real white and brown noise generators for waveforms so you don't have to stick with crappy looped noise
- made it so osc 1 cannot use multiplication mix modes
- changed redundant osc mix mode *- to /

v0.40b 
10:17 PM 16/02/2002
- initial public release

/////////////////////////////////////////////////////////////////////

BUG LIST

TODO LIST

- add tremolo track commands like the vib commands?
- have been told that 03xx command rate is bad, that it is slow until 03f2 or so
- adjust the 04xx command, make depths non-linear, so 041a is between 040a and 041a now
- could optimize more filters by removing from filter_struct and inlining with track struct
- optimize antialias code some more
- add morphing chipsynth sync version
- delays on vib and trem have been requested... add more knobs or don't add more knobs?
- add syncs to things other than lfos (adsr, porta, inertia, etc.)?
- synth is hardcoded to 44.1khz, should i make it flexible?
- sampler thinks i should make filters go to 0hz, but that throws ranges out of whack and probably blows up some algorithms
- credits
- docs

/////////////////////////////////////////////////////////////////////
*/

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
	
#include "SynthTrack.h"
#include <cstring> // was <memory.h>

//#define PREVIEW
#define FILEVERSION 4

#define THREESEL(sel,a,b,c) ((sel)<120)?((a)+((b)-(a))*(sel)/120):((b)+((c)-(b))*((sel)-120)/120)

float SourceWaveTable[MAXLFOWAVE+1][(SAMPLE_LENGTH*2)+256];
float SyncAdd[MAXSYNCMODES+1];
int vibrato_delay;
int song_freq;
float max_vcf_cutoff;
float freq_mul;

#ifndef SYNTH_LIGHT
int tremolo_delay;
#endif

CMachineParameter const paraNULL = 
{ 
	" ",
	" ",																												// description
	0,																																																// MinValue				
	1,																																												// MaxValue
	MPF_LABEL,																																								// Flags
	0
};

CMachineParameter const paraOSCnum = 
{ 
	"OSC Select",
	"OSC Select",																																				// description
	0,																																												// MinValue				
	MAXOSC-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVCFnum = 
{ 
	"VCF Select",
	"VCF Select",																																				// description
	0,																																												// MinValue				
	MAXVCF-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVCFenvtype = 
{ 
	"VCF Env Type",
	"VCF Env Type",																																				// description
	0,																																												// MinValue				
	MAXENVTYPE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCdetune = 
{ 
	"OSC Tune",
	"OSC Tune",																																				// description
	-36,																																												// MinValue				
	36,																																																// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCfinetune = 
{ 
	"OSC Finetune",
	"OSC Finetune",																																// description
	-256,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCsync = 
{ 
	"OSC Sync",
	"OSC Sync",																																				// description
	0,																																																// MinValue				
	MAXOSC,																																																// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVCAattack = 
{ 
	"VCA Attack",
	"VCA Attack",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	128
};


CMachineParameter const paraVCAdecay = 
{ 
	"VCA Decay",
	"VCA Decay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	128
};

CMachineParameter const paraVCAsustain = 
{ 
	"VCA Sustain",
	"VCA Sustain level",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	192
};


CMachineParameter const paraVCArelease = 
{ 
	"VCA Release",
	"VCA Release",																																				// description
	MIN_ENV_TIME,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	1024
};

CMachineParameter const paraVCFmixmode = 
{ 
	"VCF Mix Mode",
	"VCF Mix Mode",																																				// description
	0,																																																// MinValue				
	MAXVCFMODE,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVCFattack = 
{ 
	"VCF Attack",
	"VCF Attack",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	128
};


CMachineParameter const paraVCFdecay = 
{ 
	"VCF Decay",
	"VCF Decay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVCFdelay = 
{ 
	"VCF Delay",
	"VCF Delay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};



CMachineParameter const paraVCFsustain = 
{ 
	"VCF Sustain",
	"VCF Sustain level",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};

CMachineParameter const paraVCFrelease = 
{ 
	"VCF Release",
	"VCF Release",																																				// description
	MIN_ENV_TIME,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	1024
};

CMachineParameter const paraVCFcutoff = 
{ 
	"VCF Cutoff",
	"VCF Cutoff",																																				// description
	0,																																																// MinValue				
	MAX_VCF_CUTOFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	MAX_VCF_CUTOFF/2
};

CMachineParameter const paraVCFresonance = 
{ 
	"VCF Resonance",
	"VCF Resonance",																																// description
	1,																																																// MinValue				
	240,																																												// MaxValue
	MPF_STATE,																																								// Flags
	1
};

CMachineParameter const paraVCFtype = 
{ 
	"VCF Type",
	"VCF Type",																																								// description
	0,																																																// MinValue				
	MAXVCFTYPE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraVCFenvmod = 
{ 
	"VCF Env Mod",
	"VCF Env Mod",																																				// description
	-MAX_VCF_CUTOFF,																																												// MinValue				
	MAX_VCF_CUTOFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};



CMachineParameter const paraVCFlfospeed = 
{ 
	"VCF LFO Rate",
	"VCF LFO Rate",																																// description
	0,																																																// MinValue				
	MAX_RATE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraVCFlfoamplitude = 
{ 
	"VCF LFO Depth",
	"VCF LFO Depth",																												// description
	0,																																																// MinValue				
	MAX_VCF_CUTOFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVCFlfowave = 
{ 
	"VCF LFO Wave",
	"VCF LFO Wave",																												// description
	0,																																																// MinValue				
	MAXLFOWAVE-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};



CMachineParameter const paraVibratospeed = 
{ 
	"Vibrato Rate",
	"Vibrato Rate",																																// description
	0,																																																// MinValue				
	MAX_RATE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraVibratoamplitude = 
{ 
	"Vibrato",
	"Vibrato",																												// description
	0,																																																// MinValue				
	2304,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVibratowave = 
{ 
	"Vibrato Wave",
	"Vibrato Wave",																												// description
	0,																																																// MinValue				
	MAXLFOWAVE-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraVibratodelay = 
{ 
	"Vibrato Delay",
	"Vibrato Delay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCmixtype = 
{ 
	"OSC Mix Method",
	"OSC Mix Method",																																				// description
	0,																																																// MinValue				
	3,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOUTvol = 
{ 
	"Volume",
	"Volume",																																				// description
	0,																																												// MinValue				
	512,																																												// MaxValue
	MPF_STATE,																																								// Flags
	128
};

CMachineParameter const paraOUToverdrive = 
{ 
	"Overdrive Method",
	"Overdrive Method",																																				// description
	0,																																												// MinValue				
	MAXOVERDRIVEMETHOD,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOUToverdrivegain = 
{ 
	"Overdrive Gain",
	"Overdrive Gain",																																				// description
	0,																																												// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};



CMachineParameter const paraGlobalDetune = 
{
	"Global Tune",
	"Global Tune",																																				// description
	-36,																																												// MinValue				
	36,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraGlobalFinetune = 
{
	"Global Finetune",
	"Global Finetune",																																				// description
	-256,																																												// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraPorta = 
{
	"Portamento",
	"Portamento",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME/2,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraInertia = 
{
	"Tweak Inertia",
	"Tweak Inertia",																																				// description
	0,																																																// MinValue				
	1024,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraInterpolation = 
{
	"Antialias",
	"Antialias",																																				// description
	1,																																																// MinValue				
	MAXANTIALIAS,																																				// MaxValue
	MPF_STATE,																																								// Flags
	1
};

#ifdef SYNTH_ULTRALIGHT
CMachineParameter const paraOSCvol = 
{ 
	"OSC Volume",
	"OSC Volume",																																				// description
	0,																																												// MinValue				
	512,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};

CMachineParameter const paraOSCwave = 
{ 
	"OSC Wave",
	"OSC Wave",																																				// description
	0,																																																// MinValue				
	MAXWAVE,																																																// MaxValue
	MPF_STATE,																																								// Flags
	0
};

#else
CMachineParameter const paraOSCvolA = 
{ 
	"OSC Volume A",
	"OSC Volume A",																																				// description
	0,																																												// MinValue				
	512,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};

CMachineParameter const paraOSCvolB = 
{ 
	"OSC Volume B",
	"OSC Volume B",																																				// description
	0,																																												// MinValue				
	512,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCAwave = 
{ 
	"OSC Wave A",
	"OSC Wave A",																																				// description
	0,																																																// MinValue				
	MAXWAVE,																																																// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCBwave = 
{ 
	"OSC Wave B",
	"OSC Wave B",																																				// description
	0,																																																// MinValue				
	MAXWAVE,																																																// MaxValue
	MPF_STATE,																																								// Flags
	1
};

CMachineParameter const paraOSCwidth = 
{ 
	"OSC Width A:B",
	"OSC Width",																																				// description
	1,																																																// MinValue				
	255,																																												// MaxValue
	MPF_STATE,																																								// Flags
	128
};

CMachineParameter const paraGAINenvtype = 
{ 
	"Gain Env Type",
	"Gain Env Type",																																				// description
	0,																																												// MinValue				
	MAXENVTYPE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraGAINenvmod = 
{ 
	"Gain Env Mod",
	"Gain Env Mod",																																				// description
	-256,																																												// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraGAINattack = 
{ 
	"Gain Attack",
	"Gain Attack",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	128
};


CMachineParameter const paraGAINdecay = 
{ 
	"Gain Decay",
	"Gain Decay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraGAINdelay = 
{ 
	"Gain Delay",
	"Gain Delay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};



CMachineParameter const paraGAINsustain = 
{ 
	"Gain Sustain",
	"Gain Sustain level",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};

CMachineParameter const paraGAINrelease = 
{ 
	"Gain Release",
	"Gain Release",																																				// description
	MIN_ENV_TIME,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	1024
};


CMachineParameter const paraGAINlfospeed = 
{ 
	"Gain LFO Rate",
	"Gain LFO Rate",																																// description
	0,																																																// MinValue				
	MAX_RATE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraGAINlfoamplitude = 
{ 
	"Gain LFO Depth",
	"Gain LFO Depth",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraGAINlfowave = 
{ 
	"Gain LFO Wave",
	"Gain LFO Wave",																												// description
	0,																																																// MinValue				
	MAXLFOWAVE-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


#endif
#ifndef SYNTH_LIGHT

CMachineParameter const paraOSCphasemix = 
{ 
	"OSC Phase Mix",
	"OSC Phase Mix",																																				// description
	0,																																																// MinValue				
	MAXPHASEMIX,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCphaseamount = 
{ 
	"OSC Phase",
	"OSC Phase",																																				// description
	-SAMPLE_LENGTH*2,																																																// MinValue				
	SAMPLE_LENGTH*2,																																												// MaxValue
	MPF_STATE,																																								// Flags
	SAMPLE_LENGTH/4
};

CMachineParameter const paraOSCpenvmod = 
{ 
	"OSC PH Env Mod",
	"OSC PH Env Mod",																																				// description
	-SAMPLE_LENGTH*2,																																												// MinValue				
	SAMPLE_LENGTH*2,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCpattack = 
{ 
	"OSC PH Attack",
	"OSC PH Attack",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	128
};


CMachineParameter const paraOSCpdecay = 
{ 
	"OSC PH Decay",
	"OSC PH Decay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCpdelay = 
{ 
	"OSC PH Delay",
	"OSC PH Delay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCpsustain = 
{ 
	"OSC PH Sustain",
	"OSC PH Sustain level",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};

CMachineParameter const paraOSCprelease = 
{ 
	"OSC PH Release",
	"OSC PH Release",																																				// description
	MIN_ENV_TIME,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	1024
};

CMachineParameter const paraOSCplfospeed = 
{ 
	"OSC PH LFO Rate",
	"OSC PH LFO Rate",																																// description
	0,																																																// MinValue				
	MAX_RATE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraOSCplfoamplitude = 
{ 
	"OSC PH LFO Depth",
	"OSC PH LFO Depth",																												// description
	0,																																																// MinValue				
	SAMPLE_LENGTH*4,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCplfowave = 
{ 
	"OSC PH LFO Wave",
	"OSC PH LFO Wave",																												// description
	0,																																																// MinValue				
	MAXLFOWAVE-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCwenvmod = 
{ 
	"OSC W Env Mod",
	"OSC W Env Mod",																																				// description
	-4096,																																												// MinValue				
	4096,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCwattack = 
{ 
	"OSC W Attack",
	"OSC W Attack",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	128
};


CMachineParameter const paraOSCwdecay = 
{ 
	"OSC W Decay",
	"OSC W Decay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCwdelay = 
{ 
	"OSC W Delay",
	"OSC W Delay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraOSCwsustain = 
{ 
	"OSC W Sustain",
	"OSC W Sustain level",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};

CMachineParameter const paraOSCwrelease = 
{ 
	"OSC W Release",
	"OSC W Release",																																				// description
	MIN_ENV_TIME,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	1024
};


CMachineParameter const paraOSCwlfospeed = 
{ 
	"OSC W LFO Rate",
	"OSC W LFO Rate",																																// description
	0,																																																// MinValue				
	MAX_RATE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraOSCwlfoamplitude = 
{ 
	"OSC W LFO Depth",
	"OSC W LFO Depth",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCwlfowave = 
{ 
	"OSC W LFO Wave",
	"OSC W LFO Wave",																												// description
	0,																																																// MinValue				
	MAXLFOWAVE-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCfenvtype = 
{ 
	"OSC Frq Env Type",
	"OSC Frq Env Type",																																				// description
	0,																																												// MinValue				
	MAXENVTYPE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCwenvtype = 
{ 
	"OSC W Env Type",
	"OSC W Env Type",																																				// description
	0,																																												// MinValue				
	MAXENVTYPE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCpenvtype = 
{ 
	"OSC PH Env Type",
	"OSC PH Env Type",																																				// description
	0,																																												// MinValue				
	MAXENVTYPE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCfenvmod = 
{ 
	"OSC Frq Env Mod",
	"OSC Frq Env Mod",																																				// description
	-2304,																																												// MinValue				
	2304,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCfdelay = 
{ 
	"OSC Frq Delay",
	"OSC Frq Delay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCfattack = 
{ 
	"OSC Frq Attack",
	"OSC Frq Attack",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	128
};


CMachineParameter const paraOSCfdecay = 
{ 
	"OSC Frq Decay",
	"OSC Frq Decay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCfsustain = 
{ 
	"OSC Frq Sustain",
	"OSC Frq Sustain level",																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};

CMachineParameter const paraOSCfrelease = 
{ 
	"OSC Frq Release",
	"OSC Frq Release",																																				// description
	MIN_ENV_TIME,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	1024
};


CMachineParameter const paraOSCflfospeed = 
{ 
	"OSC Frq LFO Rate",
	"OSC Frq LFO Rate",																																// description
	0,																																																// MinValue				
	MAX_RATE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraOSCflfoamplitude = 
{ 
	"OSC Frq LFO Depth",
	"OSC Frq LFO Depth",																												// description
	0,																																																// MinValue				
	2304,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraOSCflfowave = 
{ 
	"OSC Frq LFO Wave",
	"OSC Frq LFO Wave",																												// description
	0,																																																// MinValue				
	MAXLFOWAVE-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraTremolospeed = 
{ 
	"Tremolo Rate",
	"Tremolo Rate",																																// description
	0,																																																// MinValue				
	MAX_RATE,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraTremoloamplitude = 
{ 
	"Tremolo",
	"Tremolo",																												// description
	0,																																																// MinValue				
	240,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraTremolowave = 
{ 
	"Tremolo Wave",
	"Tremolo Wave",																												// description
	0,																																																// MinValue				
	MAXLFOWAVE-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraTremolodelay = 
{ 
	"Tremolo Delay",
	"Tremolo Delay",																																				// description
	0,																																																// MinValue				
	MAX_ENV_TIME,																																				// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraARPmode = 
{
	"Arpeggio Type",
	"Arpeggio Type",																																// description
	0,																																												// MinValue				
	MAXARP-1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraARPbpm = 
{
	"Arpeggio BPM",
	"Arpeggio BPM",																																				// description
	0,																																												// MinValue				
	2048,																																								// MaxValue
	MPF_STATE,																																				// Flags
	0
};


CMachineParameter const paraARPcount = 
{
	"Arpeggio Steps",
	"Arpeggio Steps",																																				// description
	0,																																												// MinValue				
	16,																																												// MaxValue
	MPF_STATE,																																								// Flags
	4
};


#endif

#ifdef SYNTH_ULTRALIGHT
// ultralight synth
// light synth
enum {
	e_paraOSCnum,
	e_paraOSCvol,
	e_paraOSCwave,
	e_paraOSCmixtype,
	e_paraOSCsync,
	e_paraOSCdetune,
	e_paraOSCfinetune,
	e_paraNULL1,
	e_paraNULL2,
	e_paraVCFmixmode,
	e_paraVCFnum,
	e_paraVCFtype,
	e_paraVCFcutoff,
	e_paraVCFresonance,
	e_paraVCFenvtype,
	e_paraVCFenvmod,
	e_paraVCFdelay,
	e_paraVCFattack,
	e_paraVCFdecay,
	e_paraVCFsustain,
	e_paraVCFrelease,
	e_paraVCFlfoamplitude,
	e_paraVCFlfowave,
	e_paraVCFlfospeed,
	e_paraNULL3,
	e_paraOUToverdrive,
	e_paraOUToverdrivegain,
	e_paraVCAattack,
	e_paraVCAdecay,
	e_paraVCAsustain,
	e_paraVCArelease,
	e_paraNULL4,
	e_paraGlobalDetune,
	e_paraGlobalFinetune,
	e_paraNULL5,
	e_paraNULL6,
	e_paraVibratoamplitude,
	e_paraVibratowave,
	e_paraVibratospeed,
	e_paraVibratodelay,
	e_paraPorta,
	e_paraInertia,
	e_paraInterpolation,
	e_paraNULL8,
	e_paraOUTvol,
	NUMPARAMETERS
};

CMachineParameter const *pParameters[] = 
{ 
	&paraOSCnum,
	&paraOSCvol,
	&paraOSCwave,
	&paraOSCmixtype,
	&paraOSCsync,
	&paraOSCdetune,
	&paraOSCfinetune,
	&paraNULL,
	&paraNULL,
	&paraVCFmixmode,
	&paraVCFnum,
	&paraVCFtype,
	&paraVCFcutoff,
	&paraVCFresonance,
	&paraVCFenvtype,
	&paraVCFenvmod,
	&paraVCFdelay,
	&paraVCFattack,
	&paraVCFdecay,
	&paraVCFsustain,
	&paraVCFrelease,
	&paraVCFlfoamplitude,
	&paraVCFlfowave,
	&paraVCFlfospeed,
	&paraNULL,
	&paraOUToverdrive,
	&paraOUToverdrivegain,
	&paraVCAattack,
	&paraVCAdecay,
	&paraVCAsustain,
	&paraVCArelease,
	&paraNULL,
	&paraGlobalDetune,
	&paraGlobalFinetune,
	&paraNULL,
	&paraNULL,
	&paraVibratoamplitude,
	&paraVibratowave,
	&paraVibratospeed,
	&paraVibratodelay,
	&paraPorta,
	&paraInertia,
	&paraInterpolation,
	&paraNULL,
	&paraOUTvol,
	};


CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	GENERATOR,																																// flags
	NUMPARAMETERS,																												// numParameters
	pParameters,																												// Pointer to parameters
	"Pooplog FM UltraLight 0.68b",																								// name
	"Pooplog UltraL",																												// short name
	"Jeremy Evers",																												// author
	"Pattern Commands",																																				// A command, that could be use for open an editor, etc...
	NUMCOLUMNS
};
#else
#ifdef SYNTH_LIGHT
// light synth
enum {
	e_paraOSCnum,
	e_paraOSCvolA,
	e_paraOSCvolB,
	e_paraOSCAwave,
	e_paraOSCBwave,
	e_paraOSCwidth,
	e_paraOSCmixtype,
	e_paraOSCsync,
	e_paraOSCdetune,
	e_paraOSCfinetune,
	e_paraNULL1,
	e_paraNULL2,
	e_paraVCFmixmode,
	e_paraVCFnum,
	e_paraVCFtype,
	e_paraVCFcutoff,
	e_paraVCFresonance,
	e_paraVCFenvtype,
	e_paraVCFenvmod,
	e_paraVCFdelay,
	e_paraVCFattack,
	e_paraVCFdecay,
	e_paraVCFsustain,
	e_paraVCFrelease,
	e_paraVCFlfoamplitude,
	e_paraVCFlfowave,
	e_paraVCFlfospeed,
	e_paraNULL3,
	e_paraOUToverdrive,
	e_paraOUToverdrivegain,
	e_paraGAINenvtype,
	e_paraGAINenvmod,
	e_paraGAINdelay,
	e_paraGAINattack,
	e_paraGAINdecay,
	e_paraGAINsustain,
	e_paraGAINrelease,
	e_paraGAINlfoamplitude,
	e_paraGAINlfowave,
	e_paraGAINlfospeed,
	e_paraNULL4,
	e_paraVCAattack,
	e_paraVCAdecay,
	e_paraVCAsustain,
	e_paraVCArelease,
	e_paraNULL5,
	e_paraGlobalDetune,
	e_paraGlobalFinetune,
	e_paraVibratoamplitude,
	e_paraVibratowave,
	e_paraVibratospeed,
	e_paraVibratodelay,
	e_paraPorta,
	e_paraInertia,
	e_paraInterpolation,
	e_paraNULL7,
	e_paraOUTvol,
	NUMPARAMETERS
};

CMachineParameter const *pParameters[] = 
{ 
	&paraOSCnum,
	&paraOSCvolA,
	&paraOSCvolB,
	&paraOSCAwave,
	&paraOSCBwave,
	&paraOSCwidth,
	&paraOSCmixtype,
	&paraOSCsync,
	&paraOSCdetune,
	&paraOSCfinetune,
	&paraNULL,
	&paraNULL,
	&paraVCFmixmode,
	&paraVCFnum,
	&paraVCFtype,
	&paraVCFcutoff,
	&paraVCFresonance,
	&paraVCFenvtype,
	&paraVCFenvmod,
	&paraVCFdelay,
	&paraVCFattack,
	&paraVCFdecay,
	&paraVCFsustain,
	&paraVCFrelease,
	&paraVCFlfoamplitude,
	&paraVCFlfowave,
	&paraVCFlfospeed,
	&paraNULL,
	&paraOUToverdrive,
	&paraOUToverdrivegain,
	&paraGAINenvtype,
	&paraGAINenvmod,
	&paraGAINdelay,
	&paraGAINattack,
	&paraGAINdecay,
	&paraGAINsustain,
	&paraGAINrelease,
	&paraGAINlfoamplitude,
	&paraGAINlfowave,
	&paraGAINlfospeed,
	&paraNULL,
	&paraVCAattack,
	&paraVCAdecay,
	&paraVCAsustain,
	&paraVCArelease,
	&paraNULL,
	&paraGlobalDetune,
	&paraGlobalFinetune,
	&paraVibratoamplitude,
	&paraVibratowave,
	&paraVibratospeed,
	&paraVibratodelay,
	&paraPorta,
	&paraInertia,
	&paraInterpolation,
	&paraNULL,
	&paraOUTvol,
	};


CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	GENERATOR,																																// flags
	NUMPARAMETERS,																												// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	"Pooplog FM Light 0.68b(Debug build)",								// name
#else
	"Pooplog FM Light 0.68b",																								// name
#endif
	"Pooplog Light",																												// short name
	"Jeremy Evers",																												// author
	"Pattern Commands",																																				// A command, that could be use for open an editor, etc...
	NUMCOLUMNS
};
#else
// normal synth
enum {
	e_paraOSCnum,
	e_paraOSCvolA,
	e_paraOSCvolB,
	e_paraOSCAwave,
	e_paraOSCBwave,
	e_paraOSCwidth,
	e_paraOSCmixtype,
	e_paraOSCsync,
	e_paraOSCdetune,
	e_paraOSCfinetune,
	e_paraNULL1,
	e_paraOSCwenvtype,
	e_paraOSCwenvmod,
	e_paraOSCwdelay,
	e_paraOSCwattack,
	e_paraOSCwdecay,
	e_paraOSCwsustain,
	e_paraOSCwrelease,
	e_paraOSCwlfoamplitude,
	e_paraOSCwlfowave,
	e_paraOSCwlfospeed,
	e_paraOSCphasemix,
	e_paraOSCphaseamount,
	e_paraOSCpenvtype,
	e_paraOSCpenvmod,
	e_paraOSCpdelay,
	e_paraOSCpattack,
	e_paraOSCpdecay,
	e_paraOSCpsustain,
	e_paraOSCprelease,
	e_paraOSCplfoamplitude,
	e_paraOSCplfowave,
	e_paraOSCplfospeed,
	e_paraNULL2,
	e_paraOSCfenvtype,
	e_paraOSCfenvmod,
	e_paraOSCfdelay,
	e_paraOSCfattack,
	e_paraOSCfdecay,
	e_paraOSCfsustain,
	e_paraOSCfrelease,
	e_paraNULL3,
	e_paraOSCflfoamplitude,
	e_paraOSCflfowave,
	e_paraOSCflfospeed,
	e_paraNULL4,
	e_paraVCFmixmode,
	e_paraNULL5,
	e_paraVCFnum,
	e_paraVCFtype,
	e_paraVCFcutoff,
	e_paraVCFresonance,
	e_paraVCFenvtype,
	e_paraVCFenvmod,
	e_paraVCFdelay,
	e_paraVCFattack,
	e_paraVCFdecay,
	e_paraVCFsustain,
	e_paraVCFrelease,
	e_paraVCFlfoamplitude,
	e_paraVCFlfowave,
	e_paraVCFlfospeed,
	e_paraNULL6,
	e_paraOUToverdrive,
	e_paraOUToverdrivegain,
	e_paraGAINenvtype,
	e_paraGAINenvmod,
	e_paraGAINdelay,
	e_paraGAINattack,
	e_paraGAINdecay,
	e_paraGAINsustain,
	e_paraGAINrelease,
	e_paraGAINlfoamplitude,
	e_paraGAINlfowave,
	e_paraGAINlfospeed,
	e_paraNULL7,
	e_paraVCAattack,
	e_paraVCAdecay,
	e_paraVCAsustain,
	e_paraVCArelease,
	e_paraNULL8,
	e_paraGlobalDetune,
	e_paraGlobalFinetune,
	e_paraNULL9,
	e_paraTremoloamplitude,
	e_paraTremolowave,
	e_paraTremolospeed,
	e_paraTremolodelay,
	e_paraVibratoamplitude,
	e_paraVibratowave,
	e_paraVibratospeed,
	e_paraVibratodelay,
	e_paraARPmode,
	e_paraARPbpm,
	e_paraARPcount,
	e_paraNULL12,
	e_paraPorta,
	e_paraInertia,
	e_paraInterpolation,
	e_paraNULL13,
	e_paraOUTvol,
	NUMPARAMETERS
};

CMachineParameter const *pParameters[] = 
{ 
	&paraOSCnum,
	&paraOSCvolA,
	&paraOSCvolB,
	&paraOSCAwave,
	&paraOSCBwave,
	&paraOSCwidth,
	&paraOSCmixtype,
	&paraOSCsync,
	&paraOSCdetune,
	&paraOSCfinetune,
	&paraNULL,
	&paraOSCwenvtype,
	&paraOSCwenvmod,
	&paraOSCwdelay,
	&paraOSCwattack,
	&paraOSCwdecay,
	&paraOSCwsustain,
	&paraOSCwrelease,
	&paraOSCwlfoamplitude,
	&paraOSCwlfowave,
	&paraOSCwlfospeed,
	&paraOSCphasemix,
	&paraOSCphaseamount,
	&paraOSCpenvtype,
	&paraOSCpenvmod,
	&paraOSCpdelay,
	&paraOSCpattack,
	&paraOSCpdecay,
	&paraOSCpsustain,
	&paraOSCprelease,
	&paraOSCplfoamplitude,
	&paraOSCplfowave,
	&paraOSCplfospeed,
	&paraNULL,
	&paraOSCfenvtype,
	&paraOSCfenvmod,
	&paraOSCfdelay,
	&paraOSCfattack,
	&paraOSCfdecay,
	&paraOSCfsustain,
	&paraOSCfrelease,
	&paraNULL,
	&paraOSCflfoamplitude,
	&paraOSCflfowave,
	&paraOSCflfospeed,
	&paraNULL,
	&paraVCFmixmode,
	&paraNULL,
	&paraVCFnum,
	&paraVCFtype,
	&paraVCFcutoff,
	&paraVCFresonance,
	&paraVCFenvtype,
	&paraVCFenvmod,
	&paraVCFdelay,
	&paraVCFattack,
	&paraVCFdecay,
	&paraVCFsustain,
	&paraVCFrelease,
	&paraVCFlfoamplitude,
	&paraVCFlfowave,
	&paraVCFlfospeed,
	&paraNULL,
	&paraOUToverdrive,
	&paraOUToverdrivegain,
	&paraGAINenvtype,
	&paraGAINenvmod,
	&paraGAINdelay,
	&paraGAINattack,
	&paraGAINdecay,
	&paraGAINsustain,
	&paraGAINrelease,
	&paraGAINlfoamplitude,
	&paraGAINlfowave,
	&paraGAINlfospeed,
	&paraNULL,
	&paraVCAattack,
	&paraVCAdecay,
	&paraVCAsustain,
	&paraVCArelease,
	&paraNULL,
	&paraGlobalDetune,
	&paraGlobalFinetune,
	&paraNULL,
	&paraTremoloamplitude,
	&paraTremolowave,
	&paraTremolospeed,
	&paraTremolodelay,
	&paraVibratoamplitude,
	&paraVibratowave,
	&paraVibratospeed,
	&paraVibratodelay,
	&paraARPmode,
	&paraARPbpm,
	&paraARPcount,
	&paraNULL,
	&paraPorta,
	&paraInertia,
	&paraInterpolation,
	&paraNULL,
	&paraOUTvol,
	};


CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	GENERATOR,																																// flags
	NUMPARAMETERS,																												// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	"Pooplog FM Laboratory 0.68b(Debug build)",								// name
#else
	"Pooplog FM Laboratory 0.68b",																								// name
#endif
	"Pooplog",																												// short name
	"Jeremy Evers",																												// author
	"Pattern Commands",																																				// A command, that could be use for open an editor, etc...
	NUMCOLUMNS
};
#endif
#endif

struct INERTIA
{
	bool bCutoff;
	float current;
	float add;
	int * source;
	int dest;
	struct INERTIA* next;
	struct INERTIA* prev;
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks);
	virtual void SequencerTick();
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Stop();
	void NewInertia(int * source, int dest);
	void NewInertiaVC(int * source, int dest);
	void DeleteInertia(INERTIA* pI);
	void UpdateInertia();
	void InitWaveTable();

	virtual void PutData(void * pData);
	virtual void GetData(void * pData);

	virtual int GetDataSize() { return sizeof(SYNPAR); }

	SYNPAR globalpar;
	int inertia;
	struct INERTIA* pInertia;
	struct INERTIA* pLastInertia;
private:
	float * WaveTable[MAXWAVE+1];
	CSynthTrack track[MAX_TRACKS];
	

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)
//DLL_EXPORTS

mi::mi()
{
	Vals = new int[NUMPARAMETERS];
	InitWaveTable();
	pLastInertia = pInertia = NULL;
}

mi::~mi()
{

// Destroy dynamically allocated objects/memory here
	delete Vals;
	INERTIA* pI = pInertia;
	while (pI)
	{
		INERTIA* pI2 = pI->next;
		delete pI;
		pI = pI2;
	}
	pLastInertia = pInertia = NULL;
}

void mi::Init()
{
	int i;
// Initialize your stuff here
	globalpar.curOsc = 0;
	globalpar.curVcf = 0;
	globalpar.version = FILEVERSION;
	globalpar.song_sync = pCB->GetBPM();
	song_freq = pCB->GetSamplingRate();
	freq_mul = 44100.0f/song_freq;
	if (song_freq < 44100)
	{
		max_vcf_cutoff = MAX_VCF_CUTOFF_22050;
	}
	else 
	{
		max_vcf_cutoff = MAX_VCF_CUTOFF;
	}
	SyncAdd[0] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.015625f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 64 beats
	SyncAdd[1] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.03125f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 32 beats
	SyncAdd[2] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.0625f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 16 beats
	SyncAdd[3] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.125f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 8 beats
	SyncAdd[4] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.25f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 4 beats
	SyncAdd[5] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.5f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 2 beats 
	SyncAdd[6] = globalpar.song_sync*(SAMPLE_LENGTH*2*1.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // whole note
	SyncAdd[7] = globalpar.song_sync*(SAMPLE_LENGTH*2*2.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/2 note
	SyncAdd[8] = globalpar.song_sync*(SAMPLE_LENGTH*2*4.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/4 note
	SyncAdd[9] = globalpar.song_sync*(SAMPLE_LENGTH*2*8.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/8 note
	SyncAdd[10] = globalpar.song_sync*(SAMPLE_LENGTH*2*16.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/16 note
	SyncAdd[11] = globalpar.song_sync*(SAMPLE_LENGTH*2*32.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/32 note
	SyncAdd[12] = globalpar.song_sync*(SAMPLE_LENGTH*2*64.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/64 note
	for (i = 0; i < MAXOSC; i++)
	{
#ifndef SYNTH_ULTRALIGHT
		if (i) 
		{
			globalpar.gOscp[i].oscvol[0] = 0;
			globalpar.gOscp[i].oscvol[1] = 0;
		}
		else 
		{ 
			globalpar.gOscp[i].oscvol[0] = paraOSCvolA.DefValue;
			globalpar.gOscp[i].oscvol[1] = paraOSCvolB.DefValue;
		}
		globalpar.gOscp[i].Wave[0]=paraOSCAwave.DefValue;
		globalpar.gOscp[i].Wave[1]=paraOSCBwave.DefValue;
		globalpar.gOscp[i].pWave[0]=WaveTable[paraOSCAwave.DefValue];
		globalpar.gOscp[i].pWave[1]=WaveTable[paraOSCBwave.DefValue];
		globalpar.gOscp[i].oscwidth=paraOSCwidth.DefValue;
#else
		if (i) 
		{
			globalpar.gOscp[i].oscvol = 0;
		}
		else 
		{ 
			globalpar.gOscp[i].oscvol = paraOSCvol.DefValue;
		}
		globalpar.gOscp[i].Wave=paraOSCwave.DefValue;
		globalpar.gOscp[i].pWave=WaveTable[paraOSCwave.DefValue];
#endif
#ifndef SYNTH_LIGHT
		globalpar.gOscp[i].oscphasemix=paraOSCphasemix.DefValue;
		globalpar.gOscp[i].oscphase=paraOSCphaseamount.DefValue&((SAMPLE_LENGTH*2)-1);
		globalpar.gOscp[i].oscpenvtype=paraOSCpenvtype.DefValue;
		globalpar.gOscp[i].oscpenvmod=paraOSCpenvmod.DefValue;
		globalpar.gOscp[i].oscpattack=paraOSCpattack.DefValue;
		globalpar.gOscp[i].oscpdecay=paraOSCpdecay.DefValue;
		globalpar.gOscp[i].oscpdelay=paraOSCpdelay.DefValue;
		globalpar.gOscp[i].oscpsustain=paraOSCpsustain.DefValue;
		globalpar.gOscp[i].oscprelease=paraOSCprelease.DefValue;
		globalpar.gOscp[i].oscplfoamplitude=paraOSCplfoamplitude.DefValue;
		globalpar.gOscp[i].oscplfowave=paraOSCplfowave.DefValue;
		globalpar.gOscp[i].poscplfowave=SourceWaveTable[paraOSCplfowave.DefValue];
		globalpar.gOscp[i].oscplfospeed=paraOSCplfospeed.DefValue;
		globalpar.gOscp[i].oscwlfoamplitude=paraOSCwlfoamplitude.DefValue;
		globalpar.gOscp[i].oscwenvtype=paraOSCwenvtype.DefValue;
		globalpar.gOscp[i].oscwenvmod=paraOSCwenvmod.DefValue;
		globalpar.gOscp[i].oscwattack=paraOSCwattack.DefValue;
		globalpar.gOscp[i].oscwdecay=paraOSCwdecay.DefValue;
		globalpar.gOscp[i].oscwdelay=paraOSCwdelay.DefValue;
		globalpar.gOscp[i].oscwsustain=paraOSCwsustain.DefValue;
		globalpar.gOscp[i].oscwrelease=paraOSCwrelease.DefValue;
		globalpar.gOscp[i].oscwlfowave=paraOSCwlfowave.DefValue;
		globalpar.gOscp[i].poscwlfowave=SourceWaveTable[paraOSCwlfowave.DefValue];
		globalpar.gOscp[i].oscwlfospeed=paraOSCwlfospeed.DefValue;
		globalpar.gOscp[i].oscfenvmod=paraOSCfenvmod.DefValue;
		globalpar.gOscp[i].oscfenvtype=paraOSCfenvtype.DefValue;
		globalpar.gOscp[i].oscfdelay=paraOSCfdelay.DefValue;
		globalpar.gOscp[i].oscfattack=paraOSCfattack.DefValue;
		globalpar.gOscp[i].oscfdecay=paraOSCfdecay.DefValue;
		globalpar.gOscp[i].oscfsustain=paraOSCfsustain.DefValue;
		globalpar.gOscp[i].oscfrelease=paraOSCfrelease.DefValue;
		globalpar.gOscp[i].oscflfoamplitude=paraOSCflfoamplitude.DefValue;
		globalpar.gOscp[i].oscflfowave=paraOSCflfowave.DefValue;
		globalpar.gOscp[i].poscflfowave=SourceWaveTable[paraOSCflfowave.DefValue];
		globalpar.gOscp[i].oscflfospeed=paraOSCflfospeed.DefValue;
#endif
		globalpar.gOscp[i].oscmixtype=paraOSCmixtype.DefValue;
		globalpar.gOscp[i].osctune=paraOSCdetune.DefValue;
		globalpar.gOscp[i].oscfinetune=paraOSCfinetune.DefValue;
		globalpar.gOscp[i].oscsync=paraOSCsync.DefValue;
	}
	for (i = 0; i < MAXVCF; i++)
	{
		globalpar.gVcfp[i].vcftype = paraVCFtype.DefValue;
		globalpar.gVcfp[i].vcfcutoff = paraVCFcutoff.DefValue; 
		globalpar.gVcfp[i].vcfresonance = paraVCFresonance.DefValue;
		globalpar.gVcfp[i].vcfenvattack = paraVCFattack.DefValue;
		globalpar.gVcfp[i].vcfenvdecay = paraVCFdecay.DefValue;
		globalpar.gVcfp[i].vcfenvdelay = paraVCFdelay.DefValue;
		globalpar.gVcfp[i].vcfenvsustain = paraVCFsustain.DefValue;
		globalpar.gVcfp[i].vcfenvrelease = paraVCFrelease.DefValue;
		globalpar.gVcfp[i].vcfenvmod = paraVCFenvmod.DefValue;
		globalpar.gVcfp[i].vcfenvtype = paraVCFenvtype.DefValue;
		globalpar.gVcfp[i].vcflfoamplitude = paraVCFlfoamplitude.DefValue;
		globalpar.gVcfp[i].vcflfowave = paraVCFlfowave.DefValue;
		globalpar.gVcfp[i].pvcflfowave = SourceWaveTable[paraVCFlfowave.DefValue];
		globalpar.gVcfp[i].vcflfospeed = paraVCFlfospeed.DefValue;
	}
	inertia = paraInertia.DefValue;
	pLastInertia = pInertia = NULL;
	for(i=0;i<MAX_TRACKS;i++)
	{
		track[i].Init(&globalpar);
	}
#ifdef PREVIEW
	static bool bMessage = 1;
	if (bMessage)
	{
		bMessage = 0;
		char buffer[2048];
		sprintf(
				buffer,"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s",
					"WARNING!\n",
					"This is BETA software-design may change without notice.\n",
					"Known Problems:",
					"These problems are being looked into.\n",
					"You can save your song safely.\n",
					"Jeremy Evers - negspect@hotmail.com");
#ifdef SYNTH_ULTRALIGHT
		pCB->MessBox(buffer,"Pooplog FM UltraLight 0.68b",0);
#else
#ifndef SYNTH_LIGHT
		pCB->MessBox(buffer,"Pooplog FM Laboratory 0.68b",0);
#else
		pCB->MessBox(buffer,"Pooplog FM Light 0.68b",0);
#endif
#endif
	}
#endif
}

void mi::Stop()
{
	for(int c=0;c<MAX_TRACKS;c++)
	track[c].NoteOff();
}

void mi::PutData(void * pData)
{
	int i;
	if ((pData == NULL) || (((SYNPAR*)pData)->version != FILEVERSION))
	{
#ifndef SYNTH_LIGHT
		pCB->MessBox("WARNING!\nThis fileversion does not match current plugin's fileversion.\nYour settings are probably fucked.","Pooplog FM Laboratory",0);
#else
		pCB->MessBox("WARNING!\nThis fileversion does not match current plugin's fileversion.\nYour settings are probably fucked.","Pooplog FM Light",0);
#endif
		return;
	}
	memcpy(&globalpar, pData, sizeof(SYNPAR));
	// rebuild pointers
	for (i = 0; i < MAXOSC; i++)
	{
#ifndef SYNTH_ULTRALIGHT
		globalpar.gOscp[i].pWave[0]=WaveTable[globalpar.gOscp[i].Wave[0]%(MAXWAVE+1)]; 
		globalpar.gOscp[i].pWave[1]=WaveTable[globalpar.gOscp[i].Wave[1]%(MAXWAVE+1)]; 
#else
		globalpar.gOscp[i].pWave=WaveTable[globalpar.gOscp[i].Wave%(MAXWAVE+1)]; 
#endif
#ifndef SYNTH_LIGHT
		globalpar.gOscp[i].poscplfowave=SourceWaveTable[globalpar.gOscp[i].oscplfowave%MAXLFOWAVE]; 
		globalpar.gOscp[i].poscwlfowave=SourceWaveTable[globalpar.gOscp[i].oscwlfowave%MAXLFOWAVE]; 
		globalpar.gOscp[i].poscflfowave=SourceWaveTable[globalpar.gOscp[i].oscflfowave%MAXLFOWAVE]; 
#endif
	}
	for (i = 0; i < MAXVCF; i++)
	{
		globalpar.gVcfp[i].pvcflfowave=SourceWaveTable[globalpar.gVcfp[i].vcflfowave%MAXLFOWAVE];
	}
#ifndef SYNTH_LIGHT
	globalpar.ptremolo_wave=SourceWaveTable[globalpar.tremolo_wave%MAXLFOWAVE];
#endif
	globalpar.pvibrato_wave=SourceWaveTable[globalpar.vibrato_wave%MAXLFOWAVE];
#ifndef SYNTH_ULTRALIGHT
	globalpar.pgain_lfo_wave=SourceWaveTable[globalpar.gain_lfo_wave%MAXLFOWAVE];
	// ok now fill the vals structure
	Vals[e_paraOSCvolA]=																globalpar.gOscp[globalpar.curOsc].oscvol[0];
	Vals[e_paraOSCvolB]=																globalpar.gOscp[globalpar.curOsc].oscvol[1];
	Vals[e_paraOSCAwave]=												globalpar.gOscp[globalpar.curOsc].Wave[0];
	Vals[e_paraOSCBwave]=												globalpar.gOscp[globalpar.curOsc].Wave[1];
	Vals[e_paraOSCwidth]=												globalpar.gOscp[globalpar.curOsc].oscwidth;
#else
	Vals[e_paraOSCvol]=																globalpar.gOscp[globalpar.curOsc].oscvol;
	Vals[e_paraOSCwave]=												globalpar.gOscp[globalpar.curOsc].Wave;
#endif
#ifndef SYNTH_LIGHT
	Vals[e_paraOSCphasemix]=								globalpar.gOscp[globalpar.curOsc].oscphasemix;
	Vals[e_paraOSCphaseamount]=								globalpar.gOscp[globalpar.curOsc].oscphase;
	Vals[e_paraOSCpenvtype]=								globalpar.gOscp[globalpar.curOsc].oscpenvtype;
	Vals[e_paraOSCpenvmod]=												globalpar.gOscp[globalpar.curOsc].oscpenvmod;
	Vals[e_paraOSCpattack]=												globalpar.gOscp[globalpar.curOsc].oscpattack;
	Vals[e_paraOSCpdecay]=												globalpar.gOscp[globalpar.curOsc].oscpdecay;
	Vals[e_paraOSCpdelay]=												globalpar.gOscp[globalpar.curOsc].oscpdelay;
	Vals[e_paraOSCpsustain]=								globalpar.gOscp[globalpar.curOsc].oscpsustain;
	Vals[e_paraOSCprelease]=								globalpar.gOscp[globalpar.curOsc].oscprelease;
	Vals[e_paraOSCplfoamplitude]=				globalpar.gOscp[globalpar.curOsc].oscplfoamplitude;
	Vals[e_paraOSCplfospeed]=								globalpar.gOscp[globalpar.curOsc].oscplfospeed;
	Vals[e_paraOSCwlfoamplitude]=				globalpar.gOscp[globalpar.curOsc].oscwlfoamplitude;
	Vals[e_paraOSCwenvtype]=								globalpar.gOscp[globalpar.curOsc].oscwenvtype;
	Vals[e_paraOSCwenvmod]=												globalpar.gOscp[globalpar.curOsc].oscwenvmod;
	Vals[e_paraOSCwattack]=												globalpar.gOscp[globalpar.curOsc].oscwattack;
	Vals[e_paraOSCwdecay]=												globalpar.gOscp[globalpar.curOsc].oscwdecay;
	Vals[e_paraOSCwdelay]=												globalpar.gOscp[globalpar.curOsc].oscwdelay;
	Vals[e_paraOSCwsustain]=								globalpar.gOscp[globalpar.curOsc].oscwsustain;
	Vals[e_paraOSCwrelease]=								globalpar.gOscp[globalpar.curOsc].oscwrelease;
	Vals[e_paraOSCwlfospeed]=								globalpar.gOscp[globalpar.curOsc].oscwlfospeed;
	Vals[e_paraOSCfenvmod]=												globalpar.gOscp[globalpar.curOsc].oscfenvmod;
	Vals[e_paraOSCfenvtype]=								globalpar.gOscp[globalpar.curOsc].oscfenvtype;
	Vals[e_paraOSCfdelay]=												globalpar.gOscp[globalpar.curOsc].oscfdelay;
	Vals[e_paraOSCfattack]=												globalpar.gOscp[globalpar.curOsc].oscfattack;
	Vals[e_paraOSCfdecay]=												globalpar.gOscp[globalpar.curOsc].oscfdecay;
	Vals[e_paraOSCfsustain]=								globalpar.gOscp[globalpar.curOsc].oscfsustain;
	Vals[e_paraOSCfrelease]=								globalpar.gOscp[globalpar.curOsc].oscfrelease;
	Vals[e_paraOSCflfoamplitude]=				globalpar.gOscp[globalpar.curOsc].oscflfoamplitude;
	Vals[e_paraOSCflfospeed]=								globalpar.gOscp[globalpar.curOsc].oscflfospeed;
	Vals[e_paraOSCplfowave]=								globalpar.gOscp[globalpar.curOsc].oscplfowave;
	Vals[e_paraOSCflfowave]=								globalpar.gOscp[globalpar.curOsc].oscflfowave;
	Vals[e_paraOSCwlfowave]=								globalpar.gOscp[globalpar.curOsc].oscwlfowave;
#endif
	Vals[e_paraOSCmixtype]=												globalpar.gOscp[globalpar.curOsc].oscmixtype;
	Vals[e_paraOSCdetune]=												globalpar.gOscp[globalpar.curOsc].osctune;
	Vals[e_paraOSCfinetune]=								globalpar.gOscp[globalpar.curOsc].oscfinetune;
	Vals[e_paraOSCsync]=												globalpar.gOscp[globalpar.curOsc].oscsync;

	Vals[e_paraVCFtype]=				globalpar.gVcfp[globalpar.curVcf].vcftype;
	Vals[e_paraVCFcutoff]=  globalpar.gVcfp[globalpar.curVcf].vcfcutoff; 
	Vals[e_paraVCFresonance]= globalpar.gVcfp[globalpar.curVcf].vcfresonance;
	Vals[e_paraVCFattack]= globalpar.gVcfp[globalpar.curVcf].vcfenvattack;
	Vals[e_paraVCFdecay]= globalpar.gVcfp[globalpar.curVcf].vcfenvdecay;
	Vals[e_paraVCFdelay]= globalpar.gVcfp[globalpar.curVcf].vcfenvdelay;
	Vals[e_paraVCFsustain]= globalpar.gVcfp[globalpar.curVcf].vcfenvsustain;
	Vals[e_paraVCFrelease]= globalpar.gVcfp[globalpar.curVcf].vcfenvrelease;
	Vals[e_paraVCFenvmod]= globalpar.gVcfp[globalpar.curVcf].vcfenvmod;
	Vals[e_paraVCFenvtype]= globalpar.gVcfp[globalpar.curVcf].vcfenvtype;
	Vals[e_paraVCFlfoamplitude]= globalpar.gVcfp[globalpar.curVcf].vcflfoamplitude;
	Vals[e_paraVCFlfowave]= globalpar.gVcfp[globalpar.curVcf].vcflfowave;
	Vals[e_paraVCFlfospeed]= globalpar.gVcfp[globalpar.curVcf].vcflfospeed;
}

void mi::GetData(void * pData)
{
	if (pData)
	{
		memcpy(pData, &globalpar, sizeof(SYNPAR));
	}
}

void mi::NewInertia(int * source, int dest)
{
	if (inertia)
	{
		INERTIA* pI;
		pI = pInertia;
		while (pI)
		{
			if (pI->source == source)
			{
				break;
			}
			pI = pI->next;
		}
		if (*source == dest)
		{
			if (pI)
			{
				DeleteInertia(pI);
			}
			return;
		}
		if (!pI)
		{
			pI = new INERTIA;
			if (pLastInertia)
			{
				pLastInertia->next = pI;
				pI->next = NULL;
				pI->prev = pLastInertia;
				pLastInertia = pI;
			}
			else
			{
				pI->next = NULL;
				pI->prev = NULL;
				pInertia = pI;
				pLastInertia = pI;
			}
		}
		pI->current = float(*source);
		pI->source = source;
		pI->dest = dest;
		pI->add = (dest-pI->current)/inertia*freq_mul;
		pI->bCutoff = 0;
	}
	else
	{
		*source = dest;
	}
}

void mi::NewInertiaVC(int * source, int dest)
{
	if (inertia)
	{
		INERTIA* pI;
		pI = pInertia;
		while (pI)
		{
			if (pI->source == source)
			{
				break;
			}
			pI = pI->next;
		}
		if (*source == dest)
		{
			if (pI)
			{
				DeleteInertia(pI);
			}
			return;
		}
		if (!pI)
		{
			pI = new INERTIA;
			if (pLastInertia)
			{
				pLastInertia->next = pI;
				pI->next = NULL;
				pI->prev = pLastInertia;
				pLastInertia = pI;
			}
			else
			{
				pI->next = NULL;
				pI->prev = NULL;
				pInertia = pI;
				pLastInertia = pI;
			}
		}
		pI->current = float(*source);
		pI->source = source;
		pI->dest = dest;
		pI->add = (dest-pI->current)/inertia*freq_mul;
		pI->bCutoff = 1;
	}
	else
	{
		*source = dest;
		for(int c=0;c<MAX_TRACKS;c++)
		{
			track[c].lVcfv[globalpar.curVcf].VcfCutoff=float(dest);
		}
	}
}


void mi::DeleteInertia(INERTIA* pI)
{
	if (pInertia == pI)
	{
		pInertia = pI->next;
	}
	if (pLastInertia == pI)
	{
		pLastInertia = pI->prev;
	}
	if (pI->next)
	{
		pI->next->prev = pI->prev;
	}
	if (pI->prev)
	{
		pI->prev->next = pI->next;
	}
	delete pI;
}

void mi::UpdateInertia()
{
	INERTIA* pI;
	pI = pInertia;
	if (inertia)
	{
		while (pI)
		{
			pI->current += pI->add;
			if ((pI->add > 0) && (pI->current > pI->dest))
			{
				*(pI->source) = pI->dest;
				if (pI->bCutoff)
				{
					for(int c=0;c<MAX_TRACKS;c++)
					{
						track[c].lVcfv[globalpar.curVcf].VcfCutoff=float(*pI->source);
					}
				}
				INERTIA* pI2 = pI->next;
				DeleteInertia(pI);
				pI=pI2;
			}
			else if ((pI->add < 0) && (pI->current < pI->dest))
			{
			*(pI->source) = pI->dest;
				if (pI->bCutoff)
				{
					for(int c=0;c<MAX_TRACKS;c++)
					{
						track[c].lVcfv[globalpar.curVcf].VcfCutoff=float(*pI->source);
					}
				}
				INERTIA* pI2 = pI->next;
				DeleteInertia(pI);
				pI=pI2;
			}
			else if (pI->current == pI->dest)
			{
				*(pI->source) = pI->dest;
				if (pI->bCutoff)
				{
					for(int c=0;c<MAX_TRACKS;c++)
					{
						track[c].lVcfv[globalpar.curVcf].VcfCutoff=float(*pI->source);
					}
				}
				INERTIA* pI2 = pI->next;
				DeleteInertia(pI);
				pI=pI2;
			}
			else 
			{
				*pI->source = f2i(pI->current);
				if (pI->bCutoff)
				{
					for(int c=0;c<MAX_TRACKS;c++)
					{
						track[c].lVcfv[globalpar.curVcf].VcfCutoff=pI->current;
					}
				}
				pI = pI->next;
			}
		}
	}
	else
	{
		while (pI)
		{
			*(pI->source) = pI->dest;
			if (pI->bCutoff)
			{
				for(int c=0;c<MAX_TRACKS;c++)
				{
					track[c].lVcfv[globalpar.curVcf].VcfCutoff=float(*pI->source);
				}
			}
			INERTIA* pI2 = pI->next;
			DeleteInertia(pI);
			pI=pI2;
		}
	}
}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	Vals[par]=val;

	switch (par)
	{
	case e_paraOSCnum:																
		// when this one changes, we have to change a whole lot of things
		globalpar.curOsc = val%MAXOSC; 
		Vals[e_paraOSCmixtype]=												globalpar.gOscp[globalpar.curOsc].oscmixtype;
#ifndef SYNTH_ULTRALIGHT
		Vals[e_paraOSCvolA]=																globalpar.gOscp[globalpar.curOsc].oscvol[0];
		Vals[e_paraOSCvolB]=																globalpar.gOscp[globalpar.curOsc].oscvol[1];
		Vals[e_paraOSCwidth]=												globalpar.gOscp[globalpar.curOsc].oscwidth;
		Vals[e_paraOSCAwave]=												globalpar.gOscp[globalpar.curOsc].Wave[0];
		Vals[e_paraOSCBwave]=												globalpar.gOscp[globalpar.curOsc].Wave[1];
#else
		Vals[e_paraOSCvol]=																globalpar.gOscp[globalpar.curOsc].oscvol;
		Vals[e_paraOSCwave]=												globalpar.gOscp[globalpar.curOsc].Wave;
#endif
#ifndef SYNTH_LIGHT
		Vals[e_paraOSCphasemix]=								globalpar.gOscp[globalpar.curOsc].oscphasemix;
		Vals[e_paraOSCphaseamount]=								globalpar.gOscp[globalpar.curOsc].oscphase;
		Vals[e_paraOSCpenvtype]=								globalpar.gOscp[globalpar.curOsc].oscpenvtype;
		Vals[e_paraOSCpenvmod]=												globalpar.gOscp[globalpar.curOsc].oscpenvmod;
		Vals[e_paraOSCpattack]=												globalpar.gOscp[globalpar.curOsc].oscpattack;
		Vals[e_paraOSCpdecay]=												globalpar.gOscp[globalpar.curOsc].oscpdecay;
		Vals[e_paraOSCpdelay]=												globalpar.gOscp[globalpar.curOsc].oscpdelay;
		Vals[e_paraOSCpsustain]=								globalpar.gOscp[globalpar.curOsc].oscpsustain;
		Vals[e_paraOSCprelease]=								globalpar.gOscp[globalpar.curOsc].oscprelease;
		Vals[e_paraOSCplfoamplitude]=				globalpar.gOscp[globalpar.curOsc].oscplfoamplitude;
		Vals[e_paraOSCplfospeed]=								globalpar.gOscp[globalpar.curOsc].oscplfospeed;
		Vals[e_paraOSCwlfoamplitude]=				globalpar.gOscp[globalpar.curOsc].oscwlfoamplitude;
		Vals[e_paraOSCwenvtype]=								globalpar.gOscp[globalpar.curOsc].oscwenvtype;
		Vals[e_paraOSCwenvmod]=												globalpar.gOscp[globalpar.curOsc].oscwenvmod;
		Vals[e_paraOSCwattack]=												globalpar.gOscp[globalpar.curOsc].oscwattack;
		Vals[e_paraOSCwdecay]=												globalpar.gOscp[globalpar.curOsc].oscwdecay;
		Vals[e_paraOSCwdelay]=												globalpar.gOscp[globalpar.curOsc].oscwdelay;
		Vals[e_paraOSCwsustain]=								globalpar.gOscp[globalpar.curOsc].oscwsustain;
		Vals[e_paraOSCwrelease]=								globalpar.gOscp[globalpar.curOsc].oscwrelease;
		Vals[e_paraOSCwlfospeed]=								globalpar.gOscp[globalpar.curOsc].oscwlfospeed;
		Vals[e_paraOSCfenvmod]=												globalpar.gOscp[globalpar.curOsc].oscfenvmod;
		Vals[e_paraOSCfenvtype]=								globalpar.gOscp[globalpar.curOsc].oscfenvtype;
		Vals[e_paraOSCfdelay]=												globalpar.gOscp[globalpar.curOsc].oscfdelay;
		Vals[e_paraOSCfattack]=												globalpar.gOscp[globalpar.curOsc].oscfattack;
		Vals[e_paraOSCfdecay]=												globalpar.gOscp[globalpar.curOsc].oscfdecay;
		Vals[e_paraOSCfsustain]=								globalpar.gOscp[globalpar.curOsc].oscfsustain;
		Vals[e_paraOSCfrelease]=								globalpar.gOscp[globalpar.curOsc].oscfrelease;
		Vals[e_paraOSCflfoamplitude]=				globalpar.gOscp[globalpar.curOsc].oscflfoamplitude;
		Vals[e_paraOSCflfospeed]=								globalpar.gOscp[globalpar.curOsc].oscflfospeed;
		Vals[e_paraOSCplfowave]=								globalpar.gOscp[globalpar.curOsc].oscplfowave;
		Vals[e_paraOSCflfowave]=								globalpar.gOscp[globalpar.curOsc].oscflfowave;
		Vals[e_paraOSCwlfowave]=								globalpar.gOscp[globalpar.curOsc].oscwlfowave;
#endif
		Vals[e_paraOSCdetune]=												globalpar.gOscp[globalpar.curOsc].osctune;
		Vals[e_paraOSCfinetune]=								globalpar.gOscp[globalpar.curOsc].oscfinetune;
		Vals[e_paraOSCsync]=												globalpar.gOscp[globalpar.curOsc].oscsync;
		break;
	case e_paraInertia: inertia=val; break;
#ifndef SYNTH_ULTRALIGHT
	case e_paraOSCvolA:																NewInertia(&globalpar.gOscp[globalpar.curOsc].oscvol[0], val); break;
	case e_paraOSCvolB:																NewInertia(&globalpar.gOscp[globalpar.curOsc].oscvol[1], val); break;
	case e_paraOSCwidth:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwidth, val); break;
	case e_paraGAINlfoamplitude:				NewInertia(&globalpar.gain_lfo_amplitude, val); break;
	case e_paraGAINlfospeed:								NewInertia(&globalpar.gain_lfo_speed, val); break;
	case e_paraGAINattack:												NewInertia(&globalpar.gain_env_attack, val); break;
	case e_paraGAINdecay:												NewInertia(&globalpar.gain_env_decay, val); break;
	case e_paraGAINdelay:												NewInertia(&globalpar.gain_env_delay, val); break;
	case e_paraGAINsustain:												NewInertia(&globalpar.gain_env_sustain, val); break;
	case e_paraGAINrelease:												NewInertia(&globalpar.gain_env_release, val); break;
	case e_paraGAINenvmod:												NewInertia(&globalpar.gain_envmod, val); break;
#else
	case e_paraOSCvol:																NewInertia(&globalpar.gOscp[globalpar.curOsc].oscvol, val); break;
#endif
#ifndef SYNTH_LIGHT
	case e_paraOSCphaseamount:								NewInertia(&globalpar.gOscp[globalpar.curOsc].oscphase,val&((SAMPLE_LENGTH*2)-1)); break;
	case e_paraOSCpenvmod:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscpenvmod, val); break;
	case e_paraOSCpattack:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscpattack, val); break;
	case e_paraOSCpdecay:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscpdecay, val); break;
	case e_paraOSCpdelay:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscpdelay, val); break;
	case e_paraOSCpsustain:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscpsustain, val); break;
	case e_paraOSCprelease:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscprelease, val); break;
	case e_paraOSCplfoamplitude:				NewInertia(&globalpar.gOscp[globalpar.curOsc].oscplfoamplitude, val); break;
	case e_paraOSCplfospeed:								NewInertia(&globalpar.gOscp[globalpar.curOsc].oscplfospeed, val); break;
	case e_paraOSCwlfoamplitude:				NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwlfoamplitude, val); break;
	case e_paraOSCwenvmod:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwenvmod, val); break;
	case e_paraOSCwattack:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwattack, val); break;
	case e_paraOSCwdecay:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwdecay, val); break;
	case e_paraOSCwdelay:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwdelay, val); break;
	case e_paraOSCwsustain:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwsustain, val); break;
	case e_paraOSCwrelease:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwrelease, val); break;
	case e_paraOSCwlfospeed:								NewInertia(&globalpar.gOscp[globalpar.curOsc].oscwlfospeed, val); break;
	case e_paraOSCfenvmod:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscfenvmod, val); break;
	case e_paraOSCfdelay:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscfdelay, val); break;
	case e_paraOSCfattack:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscfattack, val); break;
	case e_paraOSCfdecay:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscfdecay, val); break;
	case e_paraOSCfsustain:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscfsustain, val); break;
	case e_paraOSCfrelease:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscfrelease, val); break;
	case e_paraOSCflfoamplitude:				NewInertia(&globalpar.gOscp[globalpar.curOsc].oscflfoamplitude, val); break;
	case e_paraOSCflfospeed:								NewInertia(&globalpar.gOscp[globalpar.curOsc].oscflfospeed, val); break;

	case e_paraARPbpm:																NewInertia(&globalpar.arp_bpm, val); break;

	case e_paraTremoloamplitude:				NewInertia(&globalpar.tremolo_amplitude, val); break;
	case e_paraTremolospeed:								NewInertia(&globalpar.tremolo_speed, val); break;
	case e_paraTremolodelay:								NewInertia(&tremolo_delay, val); break;
#endif
	case e_paraOSCdetune:												NewInertia(&globalpar.gOscp[globalpar.curOsc].osctune, val); break;
	case e_paraOSCfinetune:												NewInertia(&globalpar.gOscp[globalpar.curOsc].oscfinetune, val); break;

	case e_paraVCAattack:												NewInertia(&globalpar.amp_env_attack, val); break;
	case e_paraVCAdecay:												NewInertia(&globalpar.amp_env_decay, val); break;
	case e_paraVCAsustain:												NewInertia(&globalpar.amp_env_sustain, val); break;
	case e_paraVCArelease:												NewInertia(&globalpar.amp_env_release, val); break;

	case e_paraVCFcutoff:												NewInertiaVC(&globalpar.gVcfp[globalpar.curVcf].vcfcutoff, val); break;
	case e_paraVCFresonance:								NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcfresonance, val); break;
	case e_paraVCFattack:												NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcfenvattack, val); break;
	case e_paraVCFdecay:												NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcfenvdecay, val); break;
	case e_paraVCFdelay:												NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcfenvdelay, val); break;
	case e_paraVCFsustain:												NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcfenvsustain, val); break;
	case e_paraVCFrelease:												NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcfenvrelease, val); break;
	case e_paraVCFenvmod:												NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcfenvmod, val); break;
	case e_paraVCFlfoamplitude:								NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcflfoamplitude, val); break;
	case e_paraVCFlfospeed:												NewInertia(&globalpar.gVcfp[globalpar.curVcf].vcflfospeed, val); break;

	case e_paraGlobalDetune:								NewInertia(&globalpar.globaltune, val); break;
	case e_paraGlobalFinetune:								NewInertia(&globalpar.globalfinetune, val); break;
	case e_paraPorta:																NewInertia(&globalpar.synthporta, val); break;
	case e_paraVibratoamplitude:				NewInertia(&globalpar.vibrato_amplitude, val); break;
	case e_paraVibratospeed:								NewInertia(&globalpar.vibrato_speed, val); break;
	case e_paraVibratodelay:								NewInertia(&vibrato_delay, val); break;
	case e_paraOUTvol:																NewInertia(&globalpar.out_vol, val); break;
	case e_paraOUToverdrive:								globalpar.overdrive=val; break;
	case e_paraOUToverdrivegain:				NewInertia(&globalpar.overdrivegain, val); break;
#ifndef SYNTH_ULTRALIGHT
	case e_paraOSCAwave:												globalpar.gOscp[globalpar.curOsc].pWave[0]=WaveTable[val%(MAXWAVE+1)]; globalpar.gOscp[globalpar.curOsc].Wave[0]=val%(MAXWAVE+1);break;
	case e_paraOSCBwave:												globalpar.gOscp[globalpar.curOsc].pWave[1]=WaveTable[val%(MAXWAVE+1)]; globalpar.gOscp[globalpar.curOsc].Wave[1]=val%(MAXWAVE+1);break;
	case e_paraGAINlfowave: globalpar.pgain_lfo_wave=SourceWaveTable[val%MAXLFOWAVE]; globalpar.gain_lfo_wave=val%MAXLFOWAVE; break;
	case e_paraGAINenvtype: globalpar.gain_envtype=val; break;
#else
	case e_paraOSCwave:																globalpar.gOscp[globalpar.curOsc].pWave=WaveTable[val%(MAXWAVE+1)]; globalpar.gOscp[globalpar.curOsc].Wave=val%(MAXWAVE+1);break;
#endif
#ifndef SYNTH_LIGHT
	case e_paraOSCphasemix:												globalpar.gOscp[globalpar.curOsc].oscphasemix=val; break;
	case e_paraOSCpenvtype:												globalpar.gOscp[globalpar.curOsc].oscpenvtype=val; break;
	case e_paraOSCplfowave:												globalpar.gOscp[globalpar.curOsc].poscplfowave=SourceWaveTable[val%MAXLFOWAVE]; globalpar.gOscp[globalpar.curOsc].oscplfowave=val%MAXLFOWAVE; break;
	case e_paraOSCwenvtype:												globalpar.gOscp[globalpar.curOsc].oscwenvtype=val; break;
	case e_paraOSCwlfowave:												globalpar.gOscp[globalpar.curOsc].poscwlfowave=SourceWaveTable[val%MAXLFOWAVE]; globalpar.gOscp[globalpar.curOsc].oscwlfowave=val%MAXLFOWAVE; break;
	case e_paraOSCfenvtype:												globalpar.gOscp[globalpar.curOsc].oscfenvtype=val; break;
	case e_paraOSCflfowave:												globalpar.gOscp[globalpar.curOsc].poscflfowave=SourceWaveTable[val%MAXLFOWAVE]; globalpar.gOscp[globalpar.curOsc].oscflfowave=val%MAXLFOWAVE; break;

	case e_paraARPmode: globalpar.arp_mod=val; if (!val) Stop(); break;
	case e_paraARPcount: globalpar.arp_cnt=val; break;

	case e_paraTremolowave: globalpar.ptremolo_wave=SourceWaveTable[val%MAXLFOWAVE]; globalpar.tremolo_wave=val%MAXLFOWAVE; break;
#endif
	case e_paraOSCmixtype:												globalpar.gOscp[globalpar.curOsc].oscmixtype=val; break;
	case e_paraOSCsync:																globalpar.gOscp[globalpar.curOsc].oscsync=val; break;
	case e_paraVCFlfowave: globalpar.gVcfp[globalpar.curVcf].pvcflfowave=SourceWaveTable[val%MAXLFOWAVE]; globalpar.gVcfp[globalpar.curVcf].vcflfowave=val%MAXLFOWAVE; break;
	case e_paraVCFenvtype: globalpar.gVcfp[globalpar.curVcf].vcfenvtype=val; break;
	case e_paraVCFnum: 
		globalpar.curVcf = val%MAXVCF;
		Vals[e_paraVCFtype]=				globalpar.gVcfp[globalpar.curVcf].vcftype;
		Vals[e_paraVCFcutoff]=  globalpar.gVcfp[globalpar.curVcf].vcfcutoff; 
		Vals[e_paraVCFresonance]= globalpar.gVcfp[globalpar.curVcf].vcfresonance;
		Vals[e_paraVCFattack]= globalpar.gVcfp[globalpar.curVcf].vcfenvattack;
		Vals[e_paraVCFdecay]= globalpar.gVcfp[globalpar.curVcf].vcfenvdecay;
		Vals[e_paraVCFdelay]= globalpar.gVcfp[globalpar.curVcf].vcfenvdelay;
		Vals[e_paraVCFsustain]= globalpar.gVcfp[globalpar.curVcf].vcfenvsustain;
		Vals[e_paraVCFrelease]= globalpar.gVcfp[globalpar.curVcf].vcfenvrelease;
		Vals[e_paraVCFenvmod]= globalpar.gVcfp[globalpar.curVcf].vcfenvmod;
		Vals[e_paraVCFenvtype]= globalpar.gVcfp[globalpar.curVcf].vcfenvtype;
		Vals[e_paraVCFlfoamplitude]= globalpar.gVcfp[globalpar.curVcf].vcflfoamplitude;
		Vals[e_paraVCFlfowave]= globalpar.gVcfp[globalpar.curVcf].vcflfowave;
		Vals[e_paraVCFlfospeed]= globalpar.gVcfp[globalpar.curVcf].vcflfospeed;
		{
			for(int c=0;c<MAX_TRACKS;c++)
			{
				track[c].lVcfv[globalpar.curVcf].VcfCutoff=float(globalpar.gVcfp[globalpar.curVcf].vcfcutoff);
			}
		}
		break;

	case e_paraVCFmixmode: globalpar.vcfmixmode=val; break;
	case e_paraVCFtype: 
		if (globalpar.gVcfp[globalpar.curVcf].vcftype!=val)
		{
			globalpar.gVcfp[globalpar.curVcf].vcftype=val; 
			{
				for(int c=0;c<MAX_TRACKS;c++)
				{
					track[c].m_filter[globalpar.curVcf].reset();
				}
			}
		}
		break;
	case e_paraVibratowave: globalpar.pvibrato_wave=SourceWaveTable[val%MAXLFOWAVE]; globalpar.vibrato_wave=val%MAXLFOWAVE; break;
	case e_paraInterpolation: 
		globalpar.interpolate=val; 
		if (val == 1)
		{
			for(int c=0;c<MAX_TRACKS;c++)
			{
				track[c].antialias_reset();
			}
		}
		break;
	}
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[2048];

sprintf(
#ifdef SYNTH_ULTRALIGHT
		buffer,"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
		"Pooplog FM UltraLight 0.68b pattern commands:\n",
#else
#ifndef SYNTH_LIGHT
		buffer,"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s%s\n%s\n%s\n%s\n",
		"Pooplog FM Laboratory 0.68b pattern commands:\n",
#else
		buffer,"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s%s\n%s\n%s\n%s\n",
		"Pooplog FM Light 0.68b pattern commands:\n",
#endif
#endif
		"01xx - Pitch Slide Up [speed $00-$ff]",
		"02xx - Pitch Slide Down [speed $00-$ff]",
		"03xx - Track Portamento [speed $00-$ff] - noteon does not trigger envelope",
		"04xy - Vibrato [x=depth, y=speed]",
		"0500 - Bypass Envelope - noteon does not trigger envelope",
		"06xx - Panning Fade Left [speed $00-$ff]",
		"07xx - Panning Fade Right [speed $00-$ff]",
		"08xx - Panning [$01-$ff, $01=left ,$80=50%, $ff=right]",
		"09xy - Pan to Dest [x=dest,y=speed]",
		"0axx - Volume Fade Up [speed $00-$ff]",
		"0bxx - Volume Fade Down [speed $00-$ff]",
		"0cxx - Volume [amount $00-$ff]",
		"0dxy - Volume Fade to Dest [x=dest,y=speed]",
		"0exx - Note Cut [xx=time]",
		"0f00 - Cancel Current Slide/Fade/Etc.",
		"10xx - VCF 1 Cutoff Slide Up [speed $00-$ff]",
		"11xx - VCF 1 Cutoff Slide Down [speed $00-$ff]",
		"12xx - VCF 1 Cutoff [amount $00-$ff]",
		"13xy - VCF 1 Cutoff Slide to Dest [x=dest,y=speed]",
		"14xx - VCF 2 Cutoff Slide Up [speed $00-$ff]",
		"15xx - VCF 2 Cutoff Slide Down [speed $00-$ff]",
		"16xx - VCF 2 Cutoff [amount $00-$ff]",
		"17xy - VCF 2 Cutoff Slide to Dest [x=dest,y=speed]",
		"1cxx - Note Delay [xx=rate]",
		"1dxx - Note Retrigger [xx=rate]",
		"1exx - Tracker Arpeggio Rate [xx=rate, $00=fastest]",
		"1fxy - Tracker Arpeggio [x=transpose 1,y=transpose 2]",
		"2xyy - Waveform Phase [x=osc(0=all),y=phase]",
#ifndef SYNTH_LIGHT
		"3xyy - Waveform Width LFO Phase [x=osc(0=all),y=phase]",
		"4xyy - Waveform Phase LFO Phase [x=osc(0=all),y=phase]",
		"5xyy - Waveform Frequency LFO Phase [x=osc(0=all),y=phase]",
#endif
		"6xyy - VCF LFO Phase [x=vcf(0=all),y=phase]",
#ifndef SYNTH_ULTRALIGHT
		"70xx - Gain LFO Phase [$00-$ff]",
#endif
#ifndef SYNTH_LIGHT
		"71xx - Tremolo LFO Phase [$00-$ff]",
#endif
		"72xx - Vibrato LFO Phase [$00-$ff]",

		"\nJeremy Evers - negspect@hotmail.com",
		"\nthanks to dj_d, ksn, sampler, Alk, _sue_, [JAZ] and the rest of the #psycle folks for ideas, comments and testing"
		);

#ifdef SYNTH_ULTRALIGHT
	pCB->MessBox(buffer,"Pooplog FM UltraLight 0.68b",0);
#else
#ifndef SYNTH_LIGHT
	pCB->MessBox(buffer,"Pooplog FM Laboratory 0.68b",0);
#else
	pCB->MessBox(buffer,"Pooplog FM Light 0.68b",0);
#endif
#endif
}

void mi::SequencerTick()
{
	if (song_freq != pCB->GetSamplingRate())  
	{
		song_freq = pCB->GetSamplingRate();
		freq_mul = 44100.0f/song_freq;
		globalpar.song_sync = pCB->GetBPM();
		if (song_freq < 44100)
		{
			max_vcf_cutoff = MAX_VCF_CUTOFF_22050;
		}
		else 
		{
			max_vcf_cutoff = MAX_VCF_CUTOFF;
		}
		// recalculate song sync rate
		// optimizations coming here soon
		SyncAdd[0] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.015625f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 64 beats
		SyncAdd[1] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.03125f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 32 beats
		SyncAdd[2] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.0625f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 16 beats
		SyncAdd[3] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.125f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 8 beats
		SyncAdd[4] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.25f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 4 beats
		SyncAdd[5] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.5f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 2 beats 
		SyncAdd[6] = globalpar.song_sync*(SAMPLE_LENGTH*2*1.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // whole note
		SyncAdd[7] = globalpar.song_sync*(SAMPLE_LENGTH*2*2.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/2 note
		SyncAdd[8] = globalpar.song_sync*(SAMPLE_LENGTH*2*4.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/4 note
		SyncAdd[9] = globalpar.song_sync*(SAMPLE_LENGTH*2*8.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/8 note
		SyncAdd[10] = globalpar.song_sync*(SAMPLE_LENGTH*2*16.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/16 note
		SyncAdd[11] = globalpar.song_sync*(SAMPLE_LENGTH*2*32.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/32 note
		SyncAdd[12] = globalpar.song_sync*(SAMPLE_LENGTH*2*64.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/64 note
		for (int c = 0; c < MAXOSC; c++)
		{
			for (unsigned int i = 0; i < MAXVCF; i++)
			{
				track[c].m_filter[i].init(song_freq);
			}
		}
	}
	else if (globalpar.song_sync != pCB->GetBPM())  
	{
		globalpar.song_sync = pCB->GetBPM();
		// recalculate song sync rate
		// optimizations coming here soon
		SyncAdd[0] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.015625f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 64 beats
		SyncAdd[1] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.03125f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 32 beats
		SyncAdd[2] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.0625f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 16 beats
		SyncAdd[3] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.125f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 8 beats
		SyncAdd[4] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.25f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 4 beats
		SyncAdd[5] = globalpar.song_sync*(SAMPLE_LENGTH*2*0.5f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 2 beats 
		SyncAdd[6] = globalpar.song_sync*(SAMPLE_LENGTH*2*1.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // whole note
		SyncAdd[7] = globalpar.song_sync*(SAMPLE_LENGTH*2*2.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/2 note
		SyncAdd[8] = globalpar.song_sync*(SAMPLE_LENGTH*2*4.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/4 note
		SyncAdd[9] = globalpar.song_sync*(SAMPLE_LENGTH*2*8.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/8 note
		SyncAdd[10] = globalpar.song_sync*(SAMPLE_LENGTH*2*16.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/16 note
		SyncAdd[11] = globalpar.song_sync*(SAMPLE_LENGTH*2*32.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/32 note
		SyncAdd[12] = globalpar.song_sync*(SAMPLE_LENGTH*2*64.0f*(FILTER_CALC_TIME/(song_freq*60.0f)));  // 1/64 note
	}
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
	float sl=0;
//				float sr=0;
	unsigned int xnumsamples;
	float *xpsamplesleft;
	float *xpsamplesright;
	UpdateInertia();

	for(int c=0;c<tracks;c++)
	{
#ifndef SYNTH_LIGHT
		if((track[c].AmpEnvStage) || (track[c].arpflag) || (track[c].note_delay))
#else
		if((track[c].AmpEnvStage) || (track[c].note_delay))
#endif
		{
			xpsamplesleft=psamplesleft;
			xpsamplesright=psamplesright;
			
			xnumsamples=numsamples;
		
			CSynthTrack *ptrack=&track[c];

			if(ptrack->NoteCut) ptrack->NoteCutTime-=numsamples;
		
			ptrack->PerformFx();
			{				do
				{
					sl=ptrack->GetSample();
					*xpsamplesleft++ +=sl*ptrack->LMix;
					*xpsamplesright++ +=sl*ptrack->RMix;
				} while(--xnumsamples);
			}
		}
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	float fv,fv2;
	// Oscillators waveform descriptions
	switch (param)
	{
#ifndef SYNTH_ULTRALIGHT
	case e_paraOSCvolA:
		// percentage - volume
		// sustain
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		sprintf(txt,"%.4f%%",(float)value*0.390625f);
		return true;
		break;
	case e_paraOSCvolB:
		// percentage - volume
		// sustain
		if ( value == 0 )
		{
			sprintf(txt,"Use OSC Vol A");
			return true;
		}
		sprintf(txt,"%.4f%%",(float)value*0.390625f);
		return true;
		break;
	case e_paraOSCAwave:
	case e_paraOSCBwave:
		// osc waveforms
		switch(value)
		{
		case 0:sprintf(txt,"Sine +");return true;break;
		case 1:sprintf(txt,"Sine -");return true;break;
		case 2:sprintf(txt,"Triangle +");return true;break;
		case 3:sprintf(txt,"Triangle -");return true;break;
		case 4:sprintf(txt,"L Saw +");return true;break;
		case 5:sprintf(txt,"L Saw -");return true;break;
		case 6:sprintf(txt,"R Saw +");return true;break;
		case 7:sprintf(txt,"R Saw -");return true;break;
		case 8:sprintf(txt,"Square +");return true;break;
		case 9:sprintf(txt,"Square -");return true;break;
		case 10:sprintf(txt,"Harmonic Sine +");return true;break;
		case 11:sprintf(txt,"Harmonic Sine -");return true;break;
		case 12:sprintf(txt,"Inverted Sine +");return true;break;
		case 13:sprintf(txt,"Inverted Sine -");return true;break;
		case 14:sprintf(txt,"Inv Triangle +");return true;break;
		case 15:sprintf(txt,"Inv Triangle -");return true;break;
		case 16:sprintf(txt,"Soft L Saw +");return true;break;
		case 17:sprintf(txt,"Soft L Saw -");return true;break;
		case 18:sprintf(txt,"Soft R Saw +");return true;break;
		case 19:sprintf(txt,"Soft R Saw -");return true;break;
		case 20:sprintf(txt,"Soft Square +");return true;break;
		case 21:sprintf(txt,"Soft Square -");return true;break;
		case 22:sprintf(txt,"Super MW +");return true;break;
		case 23:sprintf(txt,"Super MW -");return true;break;
		case 24:sprintf(txt,"Racer +");return true;break;
		case 25:sprintf(txt,"Racer -");return true;break;
		case 26:sprintf(txt,"White Noise");return true;break;
		case 27:sprintf(txt,"White Noise Loop 1");return true;break;
		case 28:sprintf(txt,"White Noise Loop 2");return true;break; // WHITENOISEGEN
		case 29:sprintf(txt,"Brown Noise");return true;break;
		case 30:sprintf(txt,"Brown Noise Loop 1");return true;break;
		case 31:sprintf(txt,"Brown Noise Loop 2");return true;break; // BROWNNOISEGEN
		case 32:sprintf(txt,"Silence");return true;break;
		}
		break;
	case e_paraOSCwidth:
		// osc width
		fv=(float)value*0.390625f;
		sprintf(txt,"%.2f%% : %.2f%%",100-fv,fv);
		return true;
		break;
	case e_paraGAINenvmod:
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		if (globalpar.gain_envtype > 1)
		{
			fv=(float)44100*64*(value*value*0.000030517f)/(SAMPLE_LENGTH*2*FILTER_CALC_TIME);
			sprintf(txt,"%.4f hz",fv);
			return true;
		}
		if (value >= 0)
		{
			fv=value*value*100.0f/OVERDRIVEDIVISOR;
		}
		else
		{
			fv=-value*value*100.0f/OVERDRIVEDIVISOR;
		}
		sprintf(txt,"%.4f%%",fv);
		return true;
		break;
	case e_paraGAINlfoamplitude:
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		fv=value*value*100.0f/OVERDRIVEDIVISOR;
		sprintf(txt,"%.4f%%",fv);
		return true;
		break;
#else
	case e_paraOSCvol:
		// percentage - volume
		// sustain
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		sprintf(txt,"%.4f%%",(float)value*0.390625f);
		return true;
		break;
	case e_paraOSCwave:
		// osc waveforms
		switch(value)
		{
		case 0:sprintf(txt,"Sine");return true;break;
		case 1:sprintf(txt,"Triangle");return true;break;
		case 2:sprintf(txt,"L Saw");return true;break;
		case 3:sprintf(txt,"R Saw");return true;break;
		case 4:sprintf(txt,"Square");return true;break;
		case 5:sprintf(txt,"Harmonic Sine");return true;break;
		case 6:sprintf(txt,"Inverted Sine");return true;break;
		case 7:sprintf(txt,"Inv Triangle");return true;break;
		case 8:sprintf(txt,"Soft L Saw");return true;break;
		case 9:sprintf(txt,"Soft R Saw");return true;break;
		case 10:sprintf(txt,"Soft Square");return true;break;
		case 11:sprintf(txt,"Super MW");return true;break;
		case 12:sprintf(txt,"Racer");return true;break;
		case 13:sprintf(txt,"White Noise");return true;break;
		case 14:sprintf(txt,"White Noise Loop");return true;break; // WHITENOISEGEN
		case 15:sprintf(txt,"Brown Noise");return true;break;
		case 16:sprintf(txt,"Brown Noise Loop");return true;break;
		case 17:sprintf(txt,"Silence");return true;break;
		}
		break;
#endif
#ifndef SYNTH_LIGHT
	case e_paraOSCplfowave:
	case e_paraOSCwlfowave:
	case e_paraOSCflfowave:
	case e_paraTremolowave:
#endif
#ifndef SYNTH_ULTRALIGHT
	case e_paraGAINlfowave:
#endif
	case e_paraVCFlfowave:
	case e_paraVibratowave:
		// lfo waveform
		switch(value)
		{
		case 0:sprintf(txt,"Sine");return true;break;
		case 1:sprintf(txt,"Triangle");return true;break;
		case 2:sprintf(txt,"L Saw");return true;break;
		case 3:sprintf(txt,"R Saw");return true;break;
		case 4:sprintf(txt,"Square");return true;break;
		case 5:sprintf(txt,"Harmonic Sine");return true;break;
		case 6:sprintf(txt,"Inverted Sine");return true;break;
		case 7:sprintf(txt,"Inv Triangle");return true;break;
		case 8:sprintf(txt,"Soft L Saw");return true;break;
		case 9:sprintf(txt,"Soft R Saw");return true;break;
		case 10:sprintf(txt,"Soft Square");return true;break;
		case 11:sprintf(txt,"Super MW");return true;break;
		case 12:sprintf(txt,"Racer");return true;break;
		case 13:sprintf(txt,"White Noise");return true;break;
		case 14:sprintf(txt,"Brown Noise");return true;break;
		case 15:sprintf(txt,"Silence");return true;break;
		}
		break;
#ifndef SYNTH_LIGHT
	case e_paraTremoloamplitude:
		// vcf lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		sprintf(txt,"%.4f%%",(float)value*0.416666666667f);
		return true;
		break;

	case e_paraARPmode:
		// arpeggio mode
		switch(value)
		{
		case 0:sprintf(txt,"Off");return true;break;
		case 1:sprintf(txt,"Basic Major 1");return true;break;
		case 2:sprintf(txt,"Basic Major 2");return true;break;
		case 3:sprintf(txt,"Basic Major 3");return true;break;
		case 4:sprintf(txt,"Basic Minor 1");return true;break;
		case 5:sprintf(txt,"Basic Minor 2");return true;break;
		case 6:sprintf(txt,"Basic Minor 3");return true;break;
		case 7:sprintf(txt,"Minor 1");return true;break;
		case 8:sprintf(txt,"Major 1");return true;break;
		case 9:sprintf(txt,"Minor 2");return true;break;
		case 10:sprintf(txt,"Major 2");return true;break;
		case 11:sprintf(txt,"Riffer 1");return true;break;
		case 12:sprintf(txt,"Riffer 2");return true;break;
		case 13:sprintf(txt,"Riffer 3");return true;break;
		case 14:sprintf(txt,"Minor Bounce");return true;break;
		case 15:sprintf(txt,"Major Bounce");return true;break;
		case 16:sprintf(txt,"Major Pentatonic");return true;break;
		case 17:sprintf(txt,"Minor Pentatonic");return true;break;
		case 18:sprintf(txt,"Blues");return true;break;
		case 19:sprintf(txt,"Dorian");return true;break;
		case 20:sprintf(txt,"Phrygian");return true;break;
		case 21:sprintf(txt,"Lydian");return true;break;
		case 22:sprintf(txt,"Mixolydian");return true;break;
		case 23:sprintf(txt,"Locrian");return true;break;
		case 24:sprintf(txt,"Melodic Minor");return true;break;
		case 25:sprintf(txt,"Harmonic Minor");return true;break;
		case 26:sprintf(txt,"Ionian (Major)");return true;break;
		case 27:sprintf(txt,"Aeolian (Minor)");return true;break;
		case 28:sprintf(txt,"Whole Tone");return true;break;
		case 29:sprintf(txt,"Half-Whole Dim");return true;break;
		case 30:sprintf(txt,"Whole-Half Dim");return true;break;
		case 31:sprintf(txt,"ksn 1");return true;break;
		case 32:sprintf(txt,"ksn 2");return true;break;
		case 33:sprintf(txt,"ksn 3");return true;break;
		case 34:sprintf(txt,"ksn 4");return true;break;
		case 35:sprintf(txt,"ksn 5");return true;break;
		case 36:sprintf(txt,"ksn 6");return true;break;
		}
		break;
	case e_paraARPbpm: 
		switch (value)
		{
			case 0: sprintf(txt,"Sync 64 beats"); return true; break;
			case 1: sprintf(txt,"Sync 32 beats"); return true; break;
			case 2: sprintf(txt,"Sync 16 beats"); return true; break;
			case 3: sprintf(txt,"Sync 8 beats"); return true; break;
			case 4: sprintf(txt,"Sync 4 beats"); return true; break;
			case 5: sprintf(txt,"Sync 2 beats"); return true; break;
			case 6: sprintf(txt,"Sync whole note"); return true; break;
			case 7: sprintf(txt,"Sync 1/2 note"); return true; break;
			case 8: sprintf(txt,"Sync 1/4 note"); return true; break;
			case 9: sprintf(txt,"Sync 1/8 note"); return true; break;
			case 10: sprintf(txt,"Sync 1/16 note"); return true; break;
			case 11: sprintf(txt,"Sync 1/32 note"); return true; break;
			case 12: sprintf(txt,"Sync 1/64 note"); return true; break;
		}
		break;
	case e_paraOSCflfoamplitude:
		// osc freq lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		fv=(float)value*0.015625f;
		sprintf(txt,"%.4f semi",fv);
		return true;
		break;
	case e_paraOSCplfoamplitude:
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		// osc freq lfo amplitude
		fv=(float)value*360/(SAMPLE_LENGTH*2);
		sprintf(txt,"%.4f degrees",fv);
		return true;
		break;
	case e_paraOSCphaseamount:
		// osc freq lfo amplitude
		fv=(float)value*360/(SAMPLE_LENGTH*2);
		sprintf(txt,"%.4f degrees",fv);
		return true;
		break;
	case e_paraOSCwlfoamplitude:
		// osc width lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		fv=(float)value*0.390625f;
		sprintf(txt,"%.4f%%",fv);
		return true;
		break;
	case e_paraOSCwenvmod:
		// osc width lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		if (globalpar.gOscp[globalpar.curOsc].oscwenvtype > 1)
		{
			fv=(float)44100*(value*value*0.000030517f)/(SAMPLE_LENGTH*2*FILTER_CALC_TIME);
			sprintf(txt,"%.4f hz",fv);
			return true;
		}
		fv=(float)value*0.024414062f;
		sprintf(txt,"%.4f%%",fv);
		return true;
		break;
	case e_paraOSCfenvmod:
		// osc freq lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		if (globalpar.gOscp[globalpar.curOsc].oscfenvtype > 1)
		{
			fv=(float)44100*(value*value*0.000030517f)/(SAMPLE_LENGTH*2*FILTER_CALC_TIME);
			sprintf(txt,"%.4f hz",fv);
			return true;
		}
		fv=(float)value*0.015625f;
		sprintf(txt,"%.4f semi",fv);
		return true;
		break;
	case e_paraOSCphasemix:
		switch(value)
		{
		case 0:sprintf(txt,"Off");return true;break;
		case 1:sprintf(txt,"Add");return true;break;
		case 2:sprintf(txt,"Subtract");return true;break;
		case 3:sprintf(txt,"Multiply");return true;break;
		case 4:sprintf(txt,"Negative Multiply");return true;break;
		case 5:sprintf(txt,"F Add");return true;break;
		case 6:sprintf(txt,"F Subtract");return true;break;
		case 7:sprintf(txt,"F Multiply");return true;break;
		case 8:sprintf(txt,"F Negative Multiply");return true;break;
		case 9:sprintf(txt,"Replace");return true;break;
		}
		break;
	case e_paraOSCpenvmod:
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		if (globalpar.gOscp[globalpar.curOsc].oscpenvtype > 1)
		{
			fv=(float)44100*(value*value*0.000030517f)/(SAMPLE_LENGTH*2*FILTER_CALC_TIME);
			sprintf(txt,"%.4f hz",fv);
			return true;
		}
		// osc freq lfo amplitude
		fv=(float)value*360/(SAMPLE_LENGTH*2);
		sprintf(txt,"%.4f degrees",fv);
		return true;
		break;
	case e_paraOSCwenvtype:
	case e_paraOSCfenvtype:
	case e_paraOSCpenvtype:
#endif
#ifndef SYNTH_ULTRALIGHT
	case e_paraGAINenvtype:
#endif
	case e_paraVCFenvtype:
		switch (value)
		{
		case 0:sprintf(txt,"LFO+Env");return true;break;
		case 1:sprintf(txt,"LFO Depth+=Env");return true;break;
		case 2:sprintf(txt,"LFO Rate+=Env");return true;break;
		}
		break;
	case e_paraVCFenvmod:
		// vcf lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		if (globalpar.gVcfp[globalpar.curVcf].vcfenvtype > 1)
		{
			fv=(float)44100*64*(value*value*0.000030517f)/(SAMPLE_LENGTH*2*FILTER_CALC_TIME);
			sprintf(txt,"%.4f hz",fv);
			return true;
		}
		sprintf(txt,"%.4f%%",(float)value*(100.0f/MAX_VCF_CUTOFF));
		return true;
		break;
	case e_paraVibratoamplitude:
		// osc freq lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		fv=(float)value*0.015625f;
		sprintf(txt,"%.4f semi",fv);
		return true;
		break;
	case e_paraOSCmixtype: 
		if (globalpar.curOsc)
		{
			switch(value)
			{
			case 0:sprintf(txt,"Add");return true;break;
			case 1:sprintf(txt,"Subtract");return true;break;
			case 2:sprintf(txt,"Multiply");return true;break;
			case 3:sprintf(txt,"Divide");return true;break;
			}
		}
		else
		{
			sprintf(txt,"None");return true;break;
		}
		break;
#ifndef SYNTH_LIGHT
	case e_paraOSCplfospeed:
	case e_paraOSCwlfospeed:
	case e_paraOSCflfospeed:
	case e_paraTremolospeed:
#endif
#ifndef SYNTH_ULTRALIGHT
	case e_paraGAINlfospeed:
#endif
	case e_paraVibratospeed:
	case e_paraVCFlfospeed:
		switch (value)
		{
			case 0: sprintf(txt,"Sync 64 beats"); return true; break;
			case 1: sprintf(txt,"Sync 32 beats"); return true; break;
			case 2: sprintf(txt,"Sync 16 beats"); return true; break;
			case 3: sprintf(txt,"Sync 8 beats"); return true; break;
			case 4: sprintf(txt,"Sync 4 beats"); return true; break;
			case 5: sprintf(txt,"Sync 2 beats"); return true; break;
			case 6: sprintf(txt,"Sync whole note"); return true; break;
			case 7: sprintf(txt,"Sync 1/2 note"); return true; break;
			case 8: sprintf(txt,"Sync 1/4 note"); return true; break;
			case 9: sprintf(txt,"Sync 1/8 note"); return true; break;
			case 10: sprintf(txt,"Sync 1/16 note"); return true; break;
			case 11: sprintf(txt,"Sync 1/32 note"); return true; break;
			case 12: sprintf(txt,"Sync 1/64 note"); return true; break;
		}
		// filter lfo rates
		fv=(float)44100*((value-MAXSYNCMODES)*(value-MAXSYNCMODES)*0.000030517f)/(SAMPLE_LENGTH*2*FILTER_CALC_TIME);
		sprintf(txt,"%.4f hz",fv);
		return true;
		break;
	case e_paraOSCdetune:
	case e_paraGlobalDetune:
		// detune
		sprintf(txt,"%d semi",value);
		return true;
		break;
	case e_paraOSCfinetune:
	case e_paraGlobalFinetune:
		// fine detune
		fv=(float)value*0.00390625f;
		sprintf(txt,"%.4f semi",fv);
		return true;
		break;
	case e_paraInterpolation:
		// interpolation
		if (value == 1)
		{
			sprintf(txt,"Off");
			return true;
		}
		sprintf(txt,"%dx Oversample",value);
		return true;
		break;
	case e_paraOSCsync:
		// osc 2 sync
		if ((globalpar.curOsc == value-1) || (value == 0))
		{
			sprintf(txt,"Off");
			return true;
		}
		sprintf(txt,"Sync OSC%d",value);
		return true;
		break;
#ifndef SYNTH_LIGHT
	case e_paraOSCpdelay:
	case e_paraOSCpattack:
	case e_paraOSCpdecay:
	case e_paraOSCprelease:
	case e_paraOSCwdelay:
	case e_paraOSCwattack:
	case e_paraOSCwdecay:
	case e_paraOSCwrelease:
	case e_paraOSCfdelay:
	case e_paraOSCfattack:
	case e_paraOSCfdecay:
	case e_paraOSCfrelease:
	case e_paraTremolodelay:
#endif
#ifndef SYNTH_ULTRALIGHT
	case e_paraGAINdelay:
	case e_paraGAINattack:
	case e_paraGAINdecay:
	case e_paraGAINrelease:
#endif
	case e_paraVCAattack:
	case e_paraVCAdecay:
	case e_paraVCArelease:
	case e_paraPorta:
	case e_paraVCFdelay:
	case e_paraVCFattack:
	case e_paraVCFdecay:
	case e_paraVCFrelease:
	case e_paraVibratodelay:
		if (value == 0)
		{
			sprintf(txt,"Off");
			return true;
		}
		fv=(float)(value*0.022676f*FILTER_CALC_TIME);
		sprintf(txt,"%.4f ms",fv);
		return true;
		break;
	case e_paraInertia:
		if (value == 0)
		{
			sprintf(txt,"Off");
			return true;
		}
		fv=(float)(value*0.022676f*256);
		sprintf(txt,"%.4f ms",fv);
		return true;
		break;
#ifndef SYNTH_LIGHT
	case e_paraOSCpsustain:
	case e_paraOSCwsustain:
	case e_paraOSCfsustain:
#endif
#ifndef SYNTH_ULTRALIGHT
	case e_paraGAINsustain:
#endif
	case e_paraVCAsustain:
	case e_paraVCFsustain:
	case e_paraOUTvol:
		// percentage - volume
		// sustain
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		sprintf(txt,"%.4f%%",(float)value*0.390625f);
		return true;
		break;
	case e_paraVCFtype:
		// vcf type
		if (value > MAXFILTER)
		{
			sprintf(txt,"%d Stage Phaser",value-MAXFILTER);
			return true;
		}
		switch(value)
		{
		case 0 :sprintf(txt,"Off");return true;break;
		case 1 :sprintf(txt,"Moog Lowpass A");return true;break;
		case 2 :sprintf(txt,"Moog Lowpass B");return true;break;
		case 3 :sprintf(txt,"Moog Hipass A");return true;break;
		case 4 :sprintf(txt,"Moog Hipass B");return true;break;
		case 5 :sprintf(txt,"Moog Bandpass A");return true;break;
		case 6 :sprintf(txt,"Moog Bandpass B");return true;break;
		case 7 :sprintf(txt,"Lowpass A");return true;break;
		case 8 :sprintf(txt,"Lowpass B");return true;break;
		case 9 :sprintf(txt,"Hipass A");return true;break;
		case 10:sprintf(txt,"Hipass B");return true;break;
		case 11:sprintf(txt,"Bandpass A");return true;break;
		case 12:sprintf(txt,"Bandpass B");return true;break;
		case 13:sprintf(txt,"Bandreject A");return true;break;
		case 14:sprintf(txt,"Bandreject B");return true;break;
		case 15:sprintf(txt,"ParaEQ1 A");return true;break;
		case 16:sprintf(txt,"ParaEQ1 B");return true;break;
		case 17:sprintf(txt,"InvParaEQ1 A");return true;break;
		case 18:sprintf(txt,"InvParaEQ1 B");return true;break;
		case 19:sprintf(txt,"ParaEQ2 A");return true;break;
		case 20:sprintf(txt,"ParaEQ2 B");return true;break;
		case 21:sprintf(txt,"InvParaEQ2 A");return true;break;
		case 22:sprintf(txt,"InvParaEQ2 B");return true;break;
		case 23:sprintf(txt,"ParaEQ3 A");return true;break;
		case 24:sprintf(txt,"ParaEQ3 B");return true;break;
		case 25:sprintf(txt,"InvParaEQ3 A");return true;break;
		case 26:sprintf(txt,"InvParaEQ3 B");return true;break;
		}
		break;
	case e_paraVCFcutoff:
		// cutoff
		// vcf type
		if(globalpar.gVcfp[globalpar.curVcf].vcftype)
		{
			if ((globalpar.gVcfp[globalpar.curVcf].vcftype > MAXFILTER) || (globalpar.gVcfp[globalpar.curVcf].vcftype < MAXMOOG))
			{
				fv=CUTOFFCONV(value);
				sprintf(txt,"%.2f hz",fv);
				return true;
				break;
			}
			switch ((globalpar.gVcfp[globalpar.curVcf].vcftype-MAXMOOG-1)/2)
			{
			case 0: 
			case 1: 
			case 2: 
			case 3: 
			case 4: 
			case 5: 
					fv=CUTOFFCONV(value);
					sprintf(txt,"%.2f hz",fv);
					return true;
					break;
			case 6: 
			case 7: 
					fv=THREESEL((float)value,270,400,800);
					fv2=THREESEL((float)value,2140,800,1150);
					sprintf(txt,"%.2f, %.2f hz",fv, fv2);
					return true;
					break;
			case 8: 
			case 9: 
					fv=THREESEL((float)value,270,400,650);
					fv2=THREESEL((float)value,2140,1700,1080);
					sprintf(txt,"%.2f, %.2f hz",fv, fv2);
					return true;
					break;
			}
		}
		else
		{
			fv=CUTOFFCONV(value);
			sprintf(txt,"%.2f hz",fv);
			return true;
			break;
		}
		break;
	case e_paraVCFresonance:
		// resonance
		sprintf(txt,"%.4f%%",(float)(value-1)*0.418410042f);
		return true;
		break;
	case e_paraVCFlfoamplitude:
		// vcf lfo amplitude
		if ( value == 0 )
		{
			sprintf(txt,"Off");
			return true;
		}
		sprintf(txt,"%.4f%%",(float)value*(100.0f/MAX_VCF_CUTOFF));
		return true;
		break;

	case e_paraVCFmixmode:
		switch(value)
		{
		case 0:sprintf(txt,"Series 1->2->OD");return true;break;
		case 1:sprintf(txt,"Series 1->OD->2");return true;break;
		case 2:sprintf(txt,"Series OD->1->2");return true;break;
		case 3:sprintf(txt,"Series 2->1->OD");return true;break;
		case 4:sprintf(txt,"Series 2->OD->1");return true;break;
		case 5:sprintf(txt,"Series OD->2->1");return true;break;
		case 6:sprintf(txt,"Parallel +>OD");return true;break;
		case 7:sprintf(txt,"Parallel ->OD");return true;break;
		case 8:sprintf(txt,"Parallel *>OD");return true;break;
		case 9:sprintf(txt,"Parallel -*>OD");return true;break;
#ifndef SYNTH_LIGHT
		case 10:sprintf(txt,"(135>1)+(246>2)>OD");return true;break;
		case 11:sprintf(txt,"(135>1)-(246>2)>OD");return true;break;
		case 12:sprintf(txt,"(135>1)*(246>2)>OD");return true;break;
		case 13:sprintf(txt,"(135>1)-*(246>2)>OD");return true;break;
#else
		case 10:sprintf(txt,"(13->1)+(24->2)>OD");return true;break;
		case 11:sprintf(txt,"(13->1)-(24->2)>OD");return true;break;
		case 12:sprintf(txt,"(13->1)*(24->2)>OD");return true;break;
		case 13:sprintf(txt,"(13->1)-*(24->2)>OD");return true;break;
#endif
		case 14:sprintf(txt,"OD>Parallel +");return true;break;
		case 15:sprintf(txt,"OD>Parallel -");return true;break;
		case 16:sprintf(txt,"OD>Parallel *");return true;break;
		case 17:sprintf(txt,"OD>Parallel -*");return true;break;
#ifndef SYNTH_LIGHT
		case 18:sprintf(txt,"OD>(135>1)+(246>2)");return true;break;
		case 19:sprintf(txt,"OD>(135>1)-(246>2)");return true;break;
		case 20:sprintf(txt,"OD>(135>1)*(246>2)");return true;break;
		case 21:sprintf(txt,"OD>(135>1)-*(246>2)");return true;break;
#else
		case 18:sprintf(txt,"OD>(13->1)+(24->2)");return true;break;
		case 19:sprintf(txt,"OD>(13->1)-(24->2)");return true;break;
		case 20:sprintf(txt,"OD>(13->1)*(24->2)");return true;break;
		case 21:sprintf(txt,"OD>(13->1)-*(24->2)");return true;break;
#endif

		}
		break;
	case e_paraOUToverdrive:
		switch(value)
		{
		case 0 :sprintf(txt,"Off");return true;break;
		case 1 :sprintf(txt,"Soft Clip 1");return true;break;
		case 2 :sprintf(txt,"Soft Clip 2");return true;break;
		case 3 :sprintf(txt,"Soft Clip 3");return true;break;
		case 4 :sprintf(txt,"Hard Clip 1");return true;break;
		case 5 :sprintf(txt,"Hard Clip 2");return true;break;
		case 6 :sprintf(txt,"Hard Clip 3");return true;break;
		case 7 :sprintf(txt,"Parabolic Distortion 1");return true;break;
		case 8 :sprintf(txt,"Parabolic Distortion 2");return true;break;
		case 9 :sprintf(txt,"Nutty Remapper");return true;break;
		case 10:sprintf(txt,"VCA->Soft 1");return true;break;
		case 11:sprintf(txt,"VCA->Soft 2");return true;break;
		case 12:sprintf(txt,"VCA->Soft 3");return true;break;
		case 13:sprintf(txt,"VCA->Hard 1");return true;break;
		case 14:sprintf(txt,"VCA->Hard 2");return true;break;
		case 15:sprintf(txt,"VCA->Hard 3");return true;break;
		case 16:sprintf(txt,"VCA->Parabolic 1");return true;break;
		case 17:sprintf(txt,"VCA->Parabolic 2");return true;break;
		case 18:sprintf(txt,"VCA->Remapper");return true;break;
		}
		break;
	case e_paraOUToverdrivegain:
		fv=value*value*100.0f/OVERDRIVEDIVISOR;
		sprintf(txt,"%.4f%%",fv);
		return true;
		break;
	case e_paraOSCnum:
		sprintf(txt,"OSC %d",value+1);
		return true;
		break;
	case e_paraVCFnum:
		sprintf(txt,"VCF %d",value+1);
		return true;
		break;
	case e_paraNULL1:
	case e_paraNULL2:
	case e_paraNULL3:
	case e_paraNULL4:
	case e_paraNULL5:
#ifdef SYNTH_ULTRALIGHT
	case e_paraNULL6:
#else
	case e_paraNULL7:
#endif
#ifndef SYNTH_LIGHT
	case e_paraNULL9:
	case e_paraNULL12:
	case e_paraNULL13:
#endif
		sprintf(txt," ");
		return true;
		break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// The SeqTick function where your notes and pattern command handlers
// should be processed. Called each tick.
// Is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	// Note Off												== 120
	// Empty Note Row				== 255
	// Less than note off value??? == NoteON!
	if ((cmd == TFX_NoteDelay) || (cmd == TFX_NoteRetrig))
	{
		track[channel].note_delay = val;
		track[channel].note_delay_counter = 0;
		track[channel].note = note;
		track[channel].sp_cmd = cmd;
		if (cmd == TFX_NoteDelay)
		{
			return;
		}
	}
	else
	{
		track[channel].note_delay = 0;
		if ((track[channel].sp_cmd == TFX_NoteDelay) || (track[channel].sp_cmd == TFX_NoteRetrig))
		{
			track[channel].sp_cmd = 0;
		}
	}
	if(note<120)
	{
		track[channel].NoteOn(note,cmd,val);
	}

	// Note off
	if(note==120)
	{
		track[channel].NoteOff();
	}

	if (cmd)
	{
		track[channel].InitEffect(cmd,val);
	}
}

void mi::InitWaveTable()
{
	int rand = 156;
	for(int c=0;c<(SAMPLE_LENGTH*2)+256;c++)
	{
		int c2 = c & ((SAMPLE_LENGTH*2)-1);
		double sval=(double)(c*0.00306796157577128245943617517898389)/(SAMPLE_LENGTH/1024);

		// sine
		SourceWaveTable[0][c]=float(sin(sval));

		// triangle
		if (c2<SAMPLE_LENGTH/2)				
		{
			SourceWaveTable[1][c]=(c2*(1.0f/(SAMPLE_LENGTH/2)));
		}
		else if (c2<(SAMPLE_LENGTH*3)/2) 
		{
			SourceWaveTable[1][c]=1.0f-((c2-(SAMPLE_LENGTH/2))*(1.0f/(SAMPLE_LENGTH/2)));
		}
		else
		{
			SourceWaveTable[1][c]=((c2-((SAMPLE_LENGTH*3)/2))*(1.0f/(SAMPLE_LENGTH/2)))-1.0f;
		}

		// left saw
		SourceWaveTable[2][c]=(1.0f/SAMPLE_LENGTH)*((SAMPLE_LENGTH*2)-c2)-1.0f;

		// right saw
		SourceWaveTable[3][c]=(1.0f/SAMPLE_LENGTH)*(c2)-1.0f;

		// square
		if (c2 < SAMPLE_LENGTH)
		{
			SourceWaveTable[4][c]=1.0f;
		}
		else
		{
			SourceWaveTable[4][c]=-1.0f;
		}

		// harmonic distortion sin 1
		SourceWaveTable[5][c] = float((sin(sval)*0.75f) + (sin(sval*2.0)*0.5f) + (sin(sval*4.0)*0.25f) + (sin(sval*16.0)*0.125f));
		// bounce off limits
		if (SourceWaveTable[5][c] > 1.0f)
		{
			SourceWaveTable[5][c] -= 2*(SourceWaveTable[5][c]-1.0f);
		}
		else if (SourceWaveTable[7][c] < -1.0f)
		{
			SourceWaveTable[5][c] -= 2*(SourceWaveTable[5][c]+1.0f);
		}

		// inv sin
		if (c2<SAMPLE_LENGTH/2)				
		{
			// phase 1
			SourceWaveTable[6][c] = float((1.0-sin(sval+PI/2)));
		}
		else if (c2<SAMPLE_LENGTH)				
		{
			// phase 2
			SourceWaveTable[6][c] = float((1.0-sin(sval-PI/2)));
		}
		else if (c2<SAMPLE_LENGTH*3/2)				
		{
			// phase 3
			SourceWaveTable[6][c] = float((-1.0-sin(sval+PI/2)));
		}
		else 
		{
			// phase 4
			SourceWaveTable[6][c] = float((-1.0-sin(sval-PI/2)));
		}


		// inv tri
		SourceWaveTable[7][c] = ((SourceWaveTable[4][c]*0.5f)+(SourceWaveTable[0][c])-(SourceWaveTable[1][c]*0.5f)) ;
		// bounce off limits
		if (SourceWaveTable[7][c] > 1.0f)
		{
			SourceWaveTable[7][c] -= 2*(SourceWaveTable[7][c]-1.0f);
		}
		else if (SourceWaveTable[7][c] < -1.0f)
		{
			SourceWaveTable[7][c] -= 2*(SourceWaveTable[7][c]+1.0f);
		}

		// L soft saw
		SourceWaveTable[8][c] = float(sin((PI/4)+(double)(c2*0.00306796157577128245943617517898389)/(SAMPLE_LENGTH/768)));

		// R soft saw
		SourceWaveTable[9][c] = float(sin((PI*5/4)+(double)(c2*0.00306796157577128245943617517898389)/(SAMPLE_LENGTH/768)));


		// soft square
		if (c2<SAMPLE_LENGTH/2)				
		{
			SourceWaveTable[10][c] = ((SourceWaveTable[0][c]*0.5f) + (SourceWaveTable[4][c]*0.5f));// - fabs(sin(sval*32.0f)*0.2f*SourceWaveTable[5][c]));
		}
		else
		{
			SourceWaveTable[10][c] = ((SourceWaveTable[0][c]*0.5f) + (SourceWaveTable[4][c]*0.5f));// + fabs(sin(sval*32.0f)*0.2f*SourceWaveTable[5][c]));
		}

		// super mw
		if (c2<SAMPLE_LENGTH/2)				
		{
			// phase 1
			SourceWaveTable[11][c] = float((sin(sval+PI/2)));
		}
		else if (c2<SAMPLE_LENGTH)				
		{
			// phase 2
			SourceWaveTable[11][c] = float((sin(sval-PI/2)));
		}
		else if (c2<SAMPLE_LENGTH*3/2)				
		{
			// phase 3
			SourceWaveTable[11][c] = float((sin(sval+PI/2)));
		}
		else 
		{
			// phase 4
			SourceWaveTable[11][c] = float((sin(sval-PI/2)));
		}

		// racer
		if (c2<SAMPLE_LENGTH/2)				
		{
			// phase 1
			SourceWaveTable[12][c] = float((sin(sval+PI/2)));
		}
		else if (c2<SAMPLE_LENGTH)				
		{
			// phase 2
			SourceWaveTable[12][c] = float((sin(sval*2)));
		}
		else if (c2<SAMPLE_LENGTH*3/2)				
		{
			// phase 3
			SourceWaveTable[12][c] = float((sin(sval+PI/2)));
		}
		else 
		{
			// phase 4
			SourceWaveTable[12][c] = float((sin(sval*2+PI)));
		}

		// white noise
		rand = (rand*171)+145;
		SourceWaveTable[13][c]=((rand&0xffff)-0x8000)/32768.0f;

		// brown noise
		if (c > 0)
		{
			SourceWaveTable[14][c] = SourceWaveTable[14][c-1] + (SourceWaveTable[13][c]*0.5f);  
			// bounce off limits
			if (SourceWaveTable[14][c] > 1.0f)
			{
				SourceWaveTable[14][c] -= 2*(SourceWaveTable[14][c]-1.0f);
			}
			else if (SourceWaveTable[14][c] < -1.0f)
			{
				SourceWaveTable[14][c] -= 2*(SourceWaveTable[14][c]+1.0f);
			}
		}
		else
		{
			SourceWaveTable[14][0] = 0;
		}
		// none
		SourceWaveTable[15][c] = 0;
	}

	/*
		case 0:sprintf(txt,"Sine +");return true;break;
		case 1:sprintf(txt,"Sine -");return true;break;
		case 2:sprintf(txt,"Triangle +");return true;break;
		case 3:sprintf(txt,"Triangle -");return true;break;
		case 4:sprintf(txt,"L Saw +");return true;break;
		case 5:sprintf(txt,"L Saw -");return true;break;
		case 6:sprintf(txt,"R Saw +");return true;break;
		case 7:sprintf(txt,"R Saw -");return true;break;
		case 8:sprintf(txt,"Square +");return true;break;
		case 9:sprintf(txt,"Square -");return true;break;
		case 10:sprintf(txt,"Harmonic Sine +");return true;break;
		case 11:sprintf(txt,"Harmonic Sine -");return true;break;
		case 12:sprintf(txt,"Inverted Sine +");return true;break;
		case 13:sprintf(txt,"Inverted Sine -");return true;break;
		case 14:sprintf(txt,"Inv Triangle +");return true;break;
		case 15:sprintf(txt,"Inv Triangle -");return true;break;
		case 16:sprintf(txt,"Soft L Saw +");return true;break;
		case 17:sprintf(txt,"Soft L Saw -");return true;break;
		case 18:sprintf(txt,"Soft R Saw +");return true;break;
		case 19:sprintf(txt,"Soft R Saw -");return true;break;
		case 20:sprintf(txt,"Soft Square +");return true;break;
		case 21:sprintf(txt,"Soft Square -");return true;break;
		case 22:sprintf(txt,"White Noise 1");return true;break;
		case 23:sprintf(txt,"White Noise 2");return true;break;
		case 24:sprintf(txt,"Brown Noise 1");return true;break;
		case 25:sprintf(txt,"Brown Noise 2");return true;break;
		case 26:sprintf(txt,"Silence");return true;break;
	*/
#ifndef SYNTH_ULTRALIGHT
	WaveTable[0] = &SourceWaveTable[0][0];
	WaveTable[1] = &SourceWaveTable[0][SAMPLE_LENGTH];
	WaveTable[2] = &SourceWaveTable[1][0];
	WaveTable[3] = &SourceWaveTable[1][SAMPLE_LENGTH];
	WaveTable[4] = &SourceWaveTable[2][0];
	WaveTable[5] = &SourceWaveTable[2][SAMPLE_LENGTH];
	WaveTable[6] = &SourceWaveTable[3][SAMPLE_LENGTH];
	WaveTable[7] = &SourceWaveTable[3][0];
	WaveTable[8] = &SourceWaveTable[4][0];
	WaveTable[9] = &SourceWaveTable[4][SAMPLE_LENGTH];
	WaveTable[10] = &SourceWaveTable[5][0];
	WaveTable[11] = &SourceWaveTable[5][SAMPLE_LENGTH];
	WaveTable[12] = &SourceWaveTable[6][0];
	WaveTable[13] = &SourceWaveTable[6][SAMPLE_LENGTH];

	WaveTable[14] = &SourceWaveTable[7][0];
	WaveTable[15] = &SourceWaveTable[7][SAMPLE_LENGTH];
	WaveTable[16] = &SourceWaveTable[8][0];
	WaveTable[17] = &SourceWaveTable[8][SAMPLE_LENGTH];
	WaveTable[18] = &SourceWaveTable[9][SAMPLE_LENGTH];
	WaveTable[19] = &SourceWaveTable[9][0];
	WaveTable[20] = &SourceWaveTable[10][0];
	WaveTable[21] = &SourceWaveTable[10][SAMPLE_LENGTH];
	WaveTable[22] = &SourceWaveTable[11][0];
	WaveTable[23] = &SourceWaveTable[11][SAMPLE_LENGTH];
	WaveTable[24] = &SourceWaveTable[12][0];
	WaveTable[25] = &SourceWaveTable[12][SAMPLE_LENGTH];
	WaveTable[26] = &SourceWaveTable[13][0];
	WaveTable[27] = &SourceWaveTable[13][0];
	WaveTable[28] = &SourceWaveTable[13][SAMPLE_LENGTH];
	WaveTable[29] = &SourceWaveTable[14][0];
	WaveTable[30] = &SourceWaveTable[14][0];
	WaveTable[31] = &SourceWaveTable[14][SAMPLE_LENGTH];
	WaveTable[32] = &SourceWaveTable[15][0];
#else
	WaveTable[0] = &SourceWaveTable[0][0];
	WaveTable[1] = &SourceWaveTable[1][0];
	WaveTable[2] = &SourceWaveTable[2][0];
	WaveTable[3] = &SourceWaveTable[3][0];
	WaveTable[4] = &SourceWaveTable[4][0];
	WaveTable[5] = &SourceWaveTable[5][0];
	WaveTable[6] = &SourceWaveTable[6][0];
	WaveTable[7] = &SourceWaveTable[7][0];
	WaveTable[8] = &SourceWaveTable[8][0];
	WaveTable[9] = &SourceWaveTable[9][0];
	WaveTable[10] = &SourceWaveTable[10][0];
	WaveTable[11] = &SourceWaveTable[11][0];
	WaveTable[12] = &SourceWaveTable[12][0];
	WaveTable[13] = &SourceWaveTable[13][0];
	WaveTable[14] = &SourceWaveTable[13][0];
	WaveTable[15] = &SourceWaveTable[14][0];
	WaveTable[16] = &SourceWaveTable[14][0];
	WaveTable[17] = &SourceWaveTable[15][0];
#endif
}
