/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov FluidSynth plugin for PSYCLE
//

#include <psycle/plugin_interface.hpp>

#include <diversalis.hpp>
#if !defined DIVERSALIS__OS__MICROSOFT || !defined DIVERSALIS__COMPILER__MICROSOFT
	#error sorry, this code uses mfc!
#else
	#include <universalis/os/include_windows_without_crap.hpp>
#endif

#include <fluidsynth.h>

#include <string>
#include <cstring>
#include <cstdio>

using namespace psycle::plugin_interface;

#define MAX_PATH_A_LA_CON (1 << 12) // TODO total shit.. paths are loaded/saved with this length!
#define FILEVERSION 3 // TODO fix endianess and max path and upgrade the version
#define MAXMIDICHAN MAX_TRACKS
#define MAXINSTR MAXMIDICHAN

struct INSTR {
	int bank;
	int prog;
	int pitch;
	int wheel;
};

struct SYNPAR {
	int version;
	char SFPathName[MAX_PATH_A_LA_CON];

	INSTR instr[MAXINSTR];
	int curChannel;

	int reverb_on;
	int roomsize;
	int damping;
	int width;
	int r_level;

	int chorus_on;
	int nr;
	int c_level;
	int speed;
	int depth_ms;
	int type;

	int polyphony;
	int interpolation;
	int gain;
};

CMachineParameter const paraInstr = {
	"Channel / Instrument", "Channel", 0, 1, MPF_STATE||MPF_LABEL, 0
};

CMachineParameter const paraChannel = {
	"No.", "Number", 0, MAXMIDICHAN-1, MPF_STATE, 0
};

CMachineParameter const paraBank = {
	"Bank", "Bank", 0, 255, MPF_STATE, 0
};

CMachineParameter const paraProgram = {
	"Program", "Program", 0, 255, MPF_STATE, 0
};

CMachineParameter const paraPitchBend = {
	"Pitch bend", "Pitch", -32767, 32767, MPF_STATE, 0
};

CMachineParameter const paraWheelSens = {
	"Wheel sens", "Wheel", 0, 127, MPF_STATE, 0
};

CMachineParameter const paraReverb = {
	"Reverb", "Reverb", 0, 0, MPF_STATE||MPF_LABEL, 0
};

CMachineParameter const paraReverbOn = {
	"On/Off", "On/Off", 0, 1, MPF_STATE, 0
};

CMachineParameter const paraReverbRoom = {
	"Room", "Room", 0, 100, MPF_STATE, 20
};

CMachineParameter const paraReverbDamp = {
	"Damp", "Damp", 0, 100, MPF_STATE, 0
};

CMachineParameter const paraReverbWidth = {
	"Width", "Width", 0, 100, MPF_STATE, 50
};

CMachineParameter const paraReverbLevel = {
	"Level", "Level", 0, 1000, MPF_STATE, 900
};

CMachineParameter const paraChorus = {
	"Chorus", "Chorus", 0, 0, MPF_STATE||MPF_LABEL, 0
};

CMachineParameter const paraChorusOn = {
	"On/Off", "On/Off", 0, 1, MPF_STATE, 0
};

CMachineParameter const paraChorusNr = {
	"No", "No", 1, 99, MPF_STATE, FLUID_CHORUS_DEFAULT_N
};

CMachineParameter const paraChorusLevel = {
	"Level", "Level", 1, 10000, MPF_STATE, 2000
};

CMachineParameter const paraChorusSpeed = {
	"Speed", "Speed", 3, 50, MPF_STATE, 3
};

CMachineParameter const paraChorusDepth = {
	"Depth", "Depth", 0, 1000, MPF_STATE, 8
};

CMachineParameter const paraChorusType = {
	"Mode", "Mode", FLUID_CHORUS_MOD_SINE, FLUID_CHORUS_MOD_TRIANGLE, MPF_STATE, FLUID_CHORUS_MOD_SINE
};

CMachineParameter const paraPolyphony = {
	"Polyphony", "Polyphony", 1, 256, MPF_STATE, 128
};

CMachineParameter const paraInter = {
	"Interpolation", "Interpolation", FLUID_INTERP_NONE, FLUID_INTERP_HIGHEST+3, MPF_STATE, FLUID_INTERP_DEFAULT
};

CMachineParameter const paraGain = {
	"Gain", "Gain", 0, 256, MPF_STATE, 64
};

CMachineParameter const paraNull = {
	"", "", 0, 0, MPF_STATE, 0
};

CMachineParameter const *pParameters[] = { 
	&paraInstr,
	&paraChannel,
	&paraBank,
	&paraProgram,
	&paraPitchBend,
	&paraWheelSens,
	&paraNull,
	&paraPolyphony,

	&paraReverb,
	&paraReverbOn,
	&paraReverbRoom,
	&paraReverbDamp,
	&paraReverbWidth,
	&paraReverbLevel,

	&paraNull,
	&paraInter,

	&paraChorus,
	&paraChorusOn,
	&paraChorusNr,
	&paraChorusLevel,
	&paraChorusSpeed,
	&paraChorusDepth,
	&paraChorusType,

	&paraGain
};

enum {
	e_paraInstr,
	e_paraChannel,
	e_paraBank,
	e_paraProgram,
	e_paraPitchBend,
	e_paraWheelSens,
	e_paraNull1,

	e_paraPolyphony,

	e_paraReverb,
	e_paraReverbOn,
	e_paraReverbRoom,
	e_paraReverbDamp,
	e_paraReverbWidth,
	e_paraReverbLevel,
	e_paraNull2,

	e_paraInter,

	e_paraChorus,
	e_paraChorusOn,
	e_paraChorusNr,
	e_paraChorusLevel,
	e_paraChorusSpeed,
	e_paraChorusDepth,
	e_paraChorusType,
	e_paraGain,

	// NUMPARAMETERS
};

CMachineInfo const MacInfo (
	MI_VERSION,
	0x0100,
	GENERATOR,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"FluidSynth SF2 player"
		#ifndef NDEBUG
			" (Debug build)"
		#endif
		,
	"FluidSynth",
	"Peter Hanappe and others http://fluidsynth.org Ver." FLUIDSYNTH_VERSION "\n"
		"\n"
		"SoundFont(R) is a registered trademark of E-mu Systems, Inc.\n"
		"\n"
		"Usage:\n"
		"\n"
		"C-4 02 03 0C20\tC4 - note\n"
		"\t\t02 - instrument number (required!)\n"
		"\t\t03 - machine number\n"
		"\t\t0Cxx - (optional) volume at note on (00-7F)\n"
		"\n"
		"off 02 03\t\tnote-off (the instrument number is required!)\n"
		"\n"
		"ported by Sartorius",
	"Load SoundFont",
	3
);

class mi : public CMachineInterface {
	public:
		mi();
		virtual ~mi();

		virtual void Init();
		virtual void Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks);
		virtual bool DescribeValue(char* txt,int const param, int const value);
		virtual void Command();
		virtual void ParameterTweak(int par, int val);
		virtual void SequencerTick();
		virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
		virtual void Stop();

		virtual void PutData(void * pData);
		virtual void GetData(void * pData);

		virtual int GetDataSize() { return sizeof SYNPAR; }

		
		SYNPAR globalpar;

	private:
		void SetParams();
		bool LoadSF(const char * sffile);

		fluid_settings_t* settings;
		fluid_synth_t* synth;
		fluid_preset_t* preset;

		int sf_id;
		int banks[129],progs[129];
		
		int max_bank_index;

		int lastnote[MAXINSTR][MAXMIDICHAN];

		//reverb
		double roomsize, damping, width, r_level;
		//chorus
		double nr, c_level, speed, depth_ms, type;

		bool new_sf;
		int samplerate;
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

mi::mi() {
	Vals = new int[MacInfo.numParameters];
	new_sf = true;
}


mi::~mi() {
	delete[] Vals;
	if(synth) delete_fluid_synth(synth);
	if(settings) delete_fluid_settings(settings);
}

void mi::Init() {
	max_bank_index = -1;
	sf_id = 0;

	globalpar.version = FILEVERSION;
	std::memset(&globalpar.SFPathName, 0, MAX_PATH_A_LA_CON);

	for(int i = 0; i < MAXINSTR; ++i) {
		globalpar.instr[i].bank  = paraBank.DefValue;
		globalpar.instr[i].prog  = paraProgram.DefValue;
		globalpar.instr[i].pitch = paraPitchBend.DefValue;
		globalpar.instr[i].wheel = paraWheelSens.DefValue;
	}
	globalpar.curChannel = paraChannel.DefValue;

	globalpar.reverb_on = paraReverbOn.DefValue;
	globalpar.roomsize  = paraReverbRoom.DefValue;      
	globalpar.damping   = paraReverb.DefValue;          
	globalpar.width     = paraReverbWidth.DefValue;             
	globalpar.r_level   = paraReverbLevel.DefValue;     

	roomsize = (double) globalpar.roomsize * .01;
	damping  = (double) globalpar.damping * .01;
	width    = (double) globalpar.width * .01;
	r_level  = (double) globalpar.r_level * .001;


	globalpar.chorus_on = paraChorusOn.DefValue;
	globalpar.nr        = nr = FLUID_CHORUS_DEFAULT_N;
	globalpar.c_level   = paraChorusLevel.DefValue;     
	globalpar.speed     = paraChorusSpeed.DefValue;             
	globalpar.depth_ms  = paraChorusDepth.DefValue;     
	globalpar.type      = type = FLUID_CHORUS_DEFAULT_TYPE;

	c_level  = (double) globalpar.c_level * .001;
	speed    = (double) globalpar.speed * .1;
	depth_ms = (double) globalpar.depth_ms;

	
	globalpar.polyphony = 128;
	globalpar.interpolation = FLUID_INTERP_DEFAULT;
	globalpar.gain = 64;

	settings = new_fluid_settings();
	
	#ifndef NDEBUG
		fluid_settings_setstr(settings, "synth.verbose", "yes");
	#endif
	samplerate=pCB->GetSamplingRate();
	fluid_settings_setnum(settings, "synth.sample-rate", samplerate);
	fluid_settings_setint(settings, "synth.polyphony", globalpar.polyphony);
	fluid_settings_setint(settings, "synth.interpolation", globalpar.interpolation);
	fluid_settings_setint(settings, "synth.midi-channels", MAXMIDICHAN);
	fluid_settings_setint(settings, "synth.threadsafe-api", 0);
	fluid_settings_setint(settings, "synth.parallel-render", 0);
	synth = new_fluid_synth(settings);
	SetParams();
}

void mi::SetParams() {
	fluid_synth_set_interp_method(synth, -1, globalpar.interpolation);
	fluid_synth_set_polyphony(synth, globalpar.polyphony);
	fluid_synth_set_reverb_on(synth, globalpar.reverb_on);
	if(globalpar.reverb_on) {
		fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
	}
	fluid_synth_set_chorus_on(synth, globalpar.chorus_on);
	if(globalpar.chorus_on) {
		fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
	}

	//fluid_synth_system_reset(synth);
	
	for(int i = 0; i < MAXINSTR; ++i)
		for(int y = 0; y < MAXMIDICHAN; ++y)
			lastnote[i][y]  =255;
}

void mi::SequencerTick() {
	if(samplerate!=pCB->GetSamplingRate()) {
		fluid_settings_setnum(settings, "synth.sample-rate", samplerate);
		fluid_synth_set_sample_rate(synth, samplerate);
	}
}
void mi::PutData(void* pData) {
	sf_id = 0;
	int loadingVersion = ((SYNPAR*)pData)->version;
	if (pData == NULL || loadingVersion > FILEVERSION) {
		pCB->MessBox("WARNING!\nThis fileversion does not match current plugin's fileversion.\nYour settings are probably fucked.","FluidSynth",0);
		return;
	}
	#if defined DIVERSALIS__COMPILER__FEATURE__WARNING
		#warning "TODO ENDIANESS ISSUES HERE"
	#endif
	size_t readsize = sizeof SYNPAR;
	if (loadingVersion == 2) {
		readsize -= sizeof(globalpar.gain);
		globalpar.gain = 25;
		fluid_synth_set_gain(synth,globalpar.gain*(1.0/128.0));
	}
	std::memcpy(&globalpar, pData, readsize);

	new_sf = false;

	if(LoadSF(globalpar.SFPathName)) {
		Vals[e_paraChannel] = globalpar.curChannel;
		for(int i = 0; i < MAXINSTR; ++i) {
			fluid_synth_bank_select(synth, i, globalpar.instr[i].bank);
			fluid_synth_program_change(synth, i, globalpar.instr[i].prog);
			fluid_synth_pitch_bend(synth, i, globalpar.instr[i].pitch);
			fluid_synth_pitch_wheel_sens(synth, i, globalpar.instr[i].wheel);
		}
		Vals[e_paraBank]      = globalpar.instr[globalpar.curChannel].bank;
		Vals[e_paraProgram]   = globalpar.instr[globalpar.curChannel].prog;
		Vals[e_paraPitchBend] = globalpar.instr[globalpar.curChannel].pitch;
		Vals[e_paraWheelSens] = globalpar.instr[globalpar.curChannel].wheel;

		Vals[e_paraReverbOn]    = globalpar.reverb_on;
		Vals[e_paraReverbRoom]  = globalpar.roomsize;
		Vals[e_paraReverbDamp]  = globalpar.damping;
		Vals[e_paraReverbWidth] = globalpar.width;
		Vals[e_paraReverbLevel] = globalpar.r_level;
		
		roomsize = (double) globalpar.roomsize * .01;
		damping  = (double) globalpar.damping * .01;
		width    = (double) globalpar.width * .01;
		r_level  = (double) globalpar.r_level * .001;

		Vals[e_paraChorusOn]    = globalpar.chorus_on;
		Vals[e_paraChorusNr]    = nr = globalpar.nr;
		Vals[e_paraChorusLevel] = globalpar.c_level;
		Vals[e_paraChorusSpeed] = globalpar.speed;
		Vals[e_paraChorusDepth] = globalpar.depth_ms;
		Vals[e_paraChorusType]  = type = globalpar.type;

		c_level  = (double) globalpar.c_level * .001;
		speed    = (double) globalpar.speed * .1;
		depth_ms = (double) globalpar.depth_ms;

		Vals[e_paraPolyphony] = globalpar.polyphony;
		Vals[e_paraInter]     = globalpar.interpolation;
		Vals[e_paraGain]	  = globalpar.gain;

		SetParams();
	} else {
		pCB->MessBox("WARNING!\nSomething wrong...","FluidSynth",0);
		return;
	}
	new_sf = true;
}

void mi::GetData(void* pData) {
	if(pData) std::memcpy(pData, &globalpar, sizeof SYNPAR);
}

void mi::Stop() {
	#define ALL_SOUND_OFF 0x78
	for(int chan = 0; chan < MAXMIDICHAN; ++chan) {
		fluid_synth_cc(synth,chan,ALL_SOUND_OFF,0);
		for(int i = 0; i < MAXINSTR; ++i)
			lastnote[i][chan] = 255;
	}
}

void mi::ParameterTweak(int par, int val) {
	Vals[par]=val;
				
	switch(par) {
		case e_paraChannel:
			globalpar.curChannel = val;
			preset = fluid_synth_get_channel_preset(synth, globalpar.curChannel);
			if(preset != NULL && max_bank_index != -1) {
				for(int b = 0; b <= max_bank_index; ++b)
					if(banks[b] == (*(preset)->get_banknum)(preset)) {
						Vals[e_paraBank] = b;
						break;
					}
				//banks[Vals[e_paraBank]] = (*(preset)->get_banknum)(preset);
				globalpar.instr[globalpar.curChannel].bank = (*(preset)->get_banknum)(preset);
				Vals[e_paraProgram] = globalpar.instr[globalpar.curChannel].prog = (*(preset)->get_num)(preset);
			} else {
				Vals[e_paraProgram] = paraProgram.MaxValue;
			}
			Vals[e_paraPitchBend] = globalpar.instr[globalpar.curChannel].pitch;
			fluid_synth_pitch_bend(synth, globalpar.curChannel, globalpar.instr[globalpar.curChannel].pitch);
			Vals[e_paraWheelSens] = globalpar.instr[globalpar.curChannel].wheel;
			fluid_synth_pitch_wheel_sens(synth, globalpar.curChannel, globalpar.instr[globalpar.curChannel].wheel);
			break;
		case e_paraBank:
			if(max_bank_index != 1) {
				if (Vals[e_paraBank]>max_bank_index) {
					Vals[e_paraBank] = max_bank_index;
				}
				globalpar.instr[globalpar.curChannel].bank = banks[Vals[e_paraBank]];
				globalpar.instr[globalpar.curChannel].prog = Vals[e_paraProgram] = progs[globalpar.instr[globalpar.curChannel].bank];

				fluid_synth_bank_select(synth, globalpar.curChannel, globalpar.instr[globalpar.curChannel].bank);
				fluid_synth_program_change(synth, globalpar.curChannel, globalpar.instr[globalpar.curChannel].prog);
			}
			break;
		case e_paraProgram:
			globalpar.instr[globalpar.curChannel].prog = val;
			fluid_synth_program_change(synth, globalpar.curChannel, globalpar.instr[globalpar.curChannel].prog);
			break;
		case e_paraPitchBend:
			globalpar.instr[globalpar.curChannel].pitch = val;
			fluid_synth_pitch_bend(synth, globalpar.curChannel, globalpar.instr[globalpar.curChannel].pitch);
			break;
		case e_paraWheelSens:
			globalpar.instr[globalpar.curChannel].wheel = val;
			fluid_synth_pitch_wheel_sens(synth, globalpar.curChannel, globalpar.instr[globalpar.curChannel].wheel);
			break;
		case e_paraReverbOn:
			globalpar.reverb_on = val;
			fluid_synth_set_reverb_on(synth, globalpar.reverb_on);
			if(globalpar.reverb_on) {
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbRoom:
			globalpar.roomsize = val;
			roomsize = globalpar.roomsize * .01;
			if(globalpar.reverb_on) {
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbDamp:
			globalpar.damping = val;
			damping = globalpar.damping * .01;
			if(globalpar.reverb_on) {
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbWidth:
			globalpar.width = val;
			width = globalpar.width * .01;
			if(globalpar.reverb_on) {
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbLevel:
			globalpar.r_level = val;
			r_level = globalpar.r_level * .001;
			if(globalpar.reverb_on) {
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraChorusOn:
			globalpar.chorus_on = val;
			fluid_synth_set_chorus_on(synth, globalpar.chorus_on);
			if(globalpar.chorus_on) {
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraChorusNr:
			globalpar.nr = nr = val;
			if(globalpar.chorus_on) {
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraChorusLevel:
			globalpar.c_level = val; 
			c_level = (double)globalpar.c_level*.001;
			if(globalpar.chorus_on) {
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraChorusSpeed:
			globalpar.speed = val; 
			speed = (double)globalpar.speed *.1;
			if(globalpar.chorus_on) {
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraChorusDepth:
			globalpar.depth_ms = val; 
			depth_ms = (double)globalpar.depth_ms;
			if(globalpar.chorus_on) {
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraChorusType:
			globalpar.type = type = val;
			if(globalpar.chorus_on) {
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraPolyphony:
			globalpar.polyphony = val;
			fluid_synth_set_polyphony(synth, globalpar.polyphony);
			break;
		case e_paraInter:
			if(val < FLUID_INTERP_LINEAR) {
				globalpar.interpolation = FLUID_INTERP_NONE;
			} else if(val < FLUID_INTERP_4THORDER){
				globalpar.interpolation = FLUID_INTERP_LINEAR;
			} else if(val < FLUID_INTERP_7THORDER){
				globalpar.interpolation = FLUID_INTERP_4THORDER;
			} else {
				globalpar.interpolation = FLUID_INTERP_7THORDER;
			}
			fluid_synth_set_interp_method(synth, -1, globalpar.interpolation);
			break;
		case e_paraGain:
			fluid_synth_set_gain(synth,val*(1.0/128.0));
			break;
		}
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks) {
	if(sf_id > 0) {
		float *xpl, *xpr;
		xpl = psamplesleft;
		xpr = psamplesright;
		--xpl;
		--xpr;
		
		fluid_synth_write_float(synth, numsamples, psamplesleft, 0, 1, psamplesright, 0, 1);
		
		for(int i = 0; i < numsamples; ++i) {
			*++xpl *= 32767.f;
			*++xpr *= 32767.f;
		}
	}
}

bool mi::DescribeValue(char* txt,int const param, int const value) {
	switch(param) {
		case e_paraChannel:
			std::sprintf(txt,"%i (hex %X)",value,value);
			return true;
		case e_paraBank:
			std::sprintf(txt,"%i",banks[value]);
			return true;
		case e_paraProgram:
			preset = fluid_synth_get_channel_preset(synth, globalpar.curChannel);
			if(preset != NULL) {
				std::sprintf(txt,"%d: %s",(*(preset)->get_num)(preset),(*(preset)->get_name)(preset));
			} else {
				std::sprintf(txt,"(none)");
			}
			return true;
		case e_paraReverbOn:
		case e_paraChorusOn:
			std::sprintf(txt,value?"On":"Off");
			return true;
		case e_paraReverbRoom:
			std::sprintf(txt,"%.2f",(float)roomsize);
			return true;
		case e_paraReverbDamp:
			std::sprintf(txt,"%.2f",(float)damping);
			return true;
		case e_paraReverbWidth:
			std::sprintf(txt,"%.2f",(float)width);
			return true;
		case e_paraReverbLevel:
			std::sprintf(txt,"%.3f",(float)r_level);
			return true;
		case e_paraChorusNr:
			std::sprintf(txt,"%i",value);
			return true;
		case e_paraChorusLevel:
			std::sprintf(txt,"%.2f",(float)c_level);
			return true;
		case e_paraChorusSpeed:
			std::sprintf(txt,"%.1f",(float)speed);
			return true;
		case e_paraChorusDepth:
			std::sprintf(txt,"%i ms",value);
			return true;
		case e_paraChorusType:
			std::sprintf(txt,value?"Triangle":"Sine");
			return true;
		case e_paraPolyphony:
			std::sprintf(txt,"%i",value);
			return true;
		case e_paraInter:
			if(value < FLUID_INTERP_LINEAR) {
				std::sprintf(txt,"None");
			} else if(value < FLUID_INTERP_4THORDER){
				std::sprintf(txt,"Linear");
			} else if(value < FLUID_INTERP_7THORDER){
				std::sprintf(txt,"4th order");
			} else {
				std::sprintf(txt,"7th order");
			}
			return true;
		case e_paraGain:
			std::sprintf(txt,"%.2f%%",(float)value*(1.0/1.28));
			return true;
		default:
			return false;
	}
}

void mi::SeqTick(int channel, int note, int ins, int cmd, int val) {
	if(note < 120) {
		if(lastnote[ins][channel] != 255) fluid_synth_noteoff(synth, ins, lastnote[ins][channel]);
		lastnote[ins][channel] = note;
		if(cmd == 0xC) {
			if(val > 127) val = 127;
			fluid_synth_noteon(synth, ins, note, val);
		} else {
			fluid_synth_noteon(synth, ins, note, 127);
		}
	}
	if(note == 120) {
		fluid_synth_noteoff(synth, ins, lastnote[ins][channel]);
		lastnote[ins][channel] = 255;
	}
}

void mi::Command() {
	char szFilters[]="SF2 (*.sf2)|*.sf2|All Files (*.*)|*.*||";

	CFileDialog fileDlg (TRUE, "SF2", "*.sf2", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, NULL);
	if(fileDlg.DoModal () == IDOK) {
		std::string sfPathName = fileDlg.GetPathName().GetBuffer(12);
		const char * sffile = sfPathName.c_str();
		LoadSF(sffile);
	}
}

bool mi::LoadSF(const char * sf_file) {
	if(fluid_is_soundfont(sf_file)) {
		if(sf_id) {
			fluid_synth_sfunload(synth,sf_id, 1);
			fluid_synth_system_reset(synth);
		}
		sf_id = fluid_synth_sfload(synth, sf_file, 1);
		if(sf_id == -1) {
			pCB->MessBox("Error loading!","SF2 Loader",0);
			sf_id = 0;
			return false;
		}

		fluid_synth_system_reset(synth);

		int cur_bank(-1);

		max_bank_index = -1;

		for(int i = 0; i < 129; ++i) {
			banks[i] = 0;
			progs[i] = 0;
		}
		
		const int midi_chan = fluid_synth_count_midi_channels(synth);
		for(int i = 0; i < midi_chan; ++i) {
			preset = fluid_synth_get_channel_preset(synth, i);
			if(preset != NULL) {
				if(cur_bank != (*(preset)->get_banknum)(preset)) {
					cur_bank = (*(preset)->get_banknum)(preset);
					++max_bank_index;
					banks[max_bank_index] = cur_bank;
					progs[cur_bank] = (*(preset)->get_num)(preset);
					if(new_sf) {
						globalpar.instr[i].bank = banks[max_bank_index];
						globalpar.instr[i].prog = progs[banks[max_bank_index]];
					}
				}
			}
		}
		// set channel to the first available instrument
		if(new_sf)
			for(int i = 0; i < midi_chan; ++i) {
				preset = fluid_synth_get_channel_preset(synth, i);
				if(preset != NULL) {
					Vals[e_paraChannel] = globalpar.curChannel = i;
					Vals[e_paraBank] = globalpar.instr[globalpar.curChannel].bank;
					Vals[e_paraProgram] = globalpar.instr[globalpar.curChannel].prog;
					break;
				}
			}

		std::sprintf(globalpar.SFPathName, sf_file);

		return true;

	} else {
		char txt[1024];
		std::sprintf(txt, "It's not a SoundFont file or file %s not found!", sf_file);
		pCB->MessBox(txt, "SF2 Loader",0);
		return false;
	}
}
