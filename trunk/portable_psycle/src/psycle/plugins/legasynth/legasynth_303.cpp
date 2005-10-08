/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth TB303 plugin for PSYCLE
// v0.1 beta
//

#include <project.private.hpp>
#include "./303/303_voice.h"
#include "./lib/chorus.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <psycle/plugin_interface.hpp>

#define NUMPARAMETERS 28

CMachineParameter const paraCoarse = 
{
	"Coarse",
	"Coarse",									// description
	-24,											// MinValue	
	24,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraFine = 
{
	"Fine",
	"Fine",									// description
	-65535,											// MinValue	
	65535,											// MaxValue
	MPF_STATE,										// Flags
	0
};


CMachineParameter const paraDiv1 = 
{
	"",
	"",									// description
	0,											// MinValue	
	1,											// MaxValue
	MPF_STATE||MPF_LABEL,										// Flags
	0
};

CMachineParameter const paraExpression = 
{
	"Expression",
	"Expression",									// description
	0,											// MinValue	
	127,											// MaxValue
	MPF_STATE,										// Flags
	127
};

CMachineParameter const paraDiv2 = 
{
	"",
	"",									// description
	0,											// MinValue	
	1,											// MaxValue
	MPF_STATE||MPF_LABEL,										// Flags
	0
};

CMachineParameter const paraFilter = 
{
	"Envelope&Filter",
	"Envelope&Filter",									// description
	0,											// MinValue	
	1,											// MaxValue
	MPF_STATE||MPF_LABEL,						// Flags
	0
};


CMachineParameter const paraCutOff = 
{
	"Cutoff",
	"Cutoff",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0x7FFF
};

CMachineParameter const paraCutSweep = 
{
	"Cutoff sweep",
	"Cutoff sweep",									// description
	-127,											// MinValue	
	127,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraCutLFOSpeed = 
{
	"Cutoff LFO speed",
	"Cutoff",									// description
	0,											// MinValue	
	0xFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraCutLFODepth = 
{
	"Cutoff LFO depth",
	"Cutoff LFO depth",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};


CMachineParameter const paraResonance = 
{
	"Resonance",
	"Resonance",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};
CMachineParameter const paraMod = 
{
	"Modulation",
	"Modulation",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraDecay = 
{
	"Decay",
	"Modulation",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};


CMachineParameter const paraDiv3 = 
{
	"",
	"",									// description
	0,											// MinValue	
	1,											// MaxValue
	MPF_STATE||MPF_LABEL,										// Flags
	0
};

CMachineParameter const paraLFO = 
{
	"LFO&Sweep",
	"LFO&Sweep",									// description
	0,											// MinValue	
	1,											// MaxValue
	MPF_STATE||MPF_LABEL,						// Flags
	0
};

CMachineParameter const paraSpeed = 
{
	"Speed",
	"Speed",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraDepth = 
{
	"Depth",
	"Depth",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraType = 
{
	"Type",
	"Type",									// description
	0,											// MinValue	
	2,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraDelay = 
{
	"Delay",
	"Delay",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraSweepDelay = 
{
	"Sweep delay",
	"Sweep delay",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraSweepValue = 
{
	"Sweep value",
	"Sweep value",									// description
	0,											// MinValue	
	0xFFFF,											// MaxValue
	MPF_STATE,										// Flags
	0
};
CMachineParameter const paraChorus = 
{
	"Chorus",
	"Chorus",									// description
	0,											// MinValue	
	1,											// MaxValue
	MPF_STATE||MPF_LABEL,										// Flags
	0
};

CMachineParameter const paraChorusOnOff = 
{
	"On/Off",
	"On/Off",									// description
	0,											// MinValue	
	1,											// MaxValue
	MPF_STATE,										// Flags
	0
};
CMachineParameter const paraChorusDelay = 
{
	"Delay",
	"Delay",									// description
	0,											// MinValue	
	255,											// MaxValue
	MPF_STATE,										// Flags
	3
};

CMachineParameter const paraChorusLFOSpeed = 
{
	"LFO Speed",
	"LFO Speed",									// description
	0,											// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	2
};

CMachineParameter const paraChorusLFODepth = 
{
	"LFO Depth",
	"LFO Depth",									// description
	0,											// MinValue	
	127,											// MaxValue
	MPF_STATE,										// Flags
	1
};


CMachineParameter const paraChorusFB = 
{
	"Feedback",
	"Feedback",									// description
	1,											// MinValue	
	100,											// MaxValue
	MPF_STATE,										// Flags
	64
};

CMachineParameter const paraChorusWidth = 
{
	"Width",
	"Width",									// description
	-127,											// MinValue	
	127,											// MaxValue
	MPF_STATE,										// Flags
	-10
};


CMachineParameter const *pParameters[] = 
{ 
	&paraCoarse,
	&paraFine,
	&paraDiv1,
	&paraExpression,
	&paraDiv2,
	&paraFilter,
	&paraCutOff,
	&paraCutSweep,
	&paraCutLFOSpeed,
	&paraCutLFODepth,
	&paraResonance,
	&paraMod,
	&paraDecay,
	&paraDiv3,
	&paraLFO,
	&paraSpeed,
	&paraDepth,
	&paraType,
	&paraDelay,
	&paraSweepDelay,
	&paraSweepValue,
	&paraChorus,
	&paraChorusOnOff,
	&paraChorusDelay,
	&paraChorusLFOSpeed,
	&paraChorusLFODepth,
	&paraChorusFB,
	&paraChorusWidth

};


CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	GENERATOR,								// flags
	NUMPARAMETERS,							// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"LegaSynth TB303 (Debug build)",		// name
#else
	"LegaSynth TB303",						// name
#endif
	"TB303",							// short name
	"Juan Linietsky, ported by Sartorius",							// author
	"Help",									// A command, that could be use for open an editor, etc...
	2
};

class mi : public CMachineInterface
{
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

	TB303_Voice track[MAX_TRACKS];
	Voice::Default_Data track_def_data;
	TB303_Voice::Data track_data;
	Chorus	chorus;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals=new int[NUMPARAMETERS];
}

mi::~mi()
{
	// Destroy dinamically allocated objects/memory here
	delete Vals;
}

void mi::Init()
{
// Initialize your stuff here
	track_def_data.relative_pan=0;
	track_def_data.LFO_speed=0;
	track_def_data.LFO_depth=0;
	track_def_data.LFO_type=0; // 0 - sine / 1 - saw / 2- pulse
	track_def_data.LFO_delay=0; 
	
	track_def_data.sweep_delay=0;
	track_def_data.sweep_value=0;
	
        /* Channel Default Stuff */
	
	//track_def_data.vibrato_depth=0;
	//track_def_data.vibrato_speed=30;
	//track_def_data.channel_pan=0;
	//track_def_data.main_volume=127;
	//track_def_data.default_chorus=0;
	//track_def_data.portamento_time_coarse=0;
	//track_def_data.portamento_time_fine=0;
	//track_def_data.do_portamento=false;

	track_data.coarse = 0;
    track_data.fine=0; //coarse -127 to 127
	track_data.envelope_cutoff=0x7FFF; //"Cut", "Filter envelope cutoff 0 - 0xFFFF"
	track_data.resonance=0; //"Res", "Filter resonance - 0 - 0xFFFF 
	track_data.envelope_mod=0; //"Env", "Filter envelope modulation"
	track_data.envelope_decay=0; // "Filter envelope decay" - 0 - 0xFFFF

	track_data.cutoff_sweep=0; // -127 to 127
	track_data.cutoff_lfo_speed=0; // 0 - 0xFF
	track_data.cutoff_lfo_depth=0; // 0 - 0xFF


	for(int i=0;i<MAX_TRACKS;i++)
	{
		track[i].set_default_data(&track_def_data);
		track[i].set_data(&track_data);

		track[i].set_mix_frequency(44100);
		track[i].set_note_off(0);
		track[i].set_sustain(false);
	}
	chorus.set_mixfreq(44100);
}

void mi::Stop()
{
	for(int c=0;c<MAX_TRACKS;c++)
	{
		track[c].set_note_off(0);
	}
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	char expr = Vals[3];
	Vals[par]=val;
	switch (par) {
		case 0:track_data.coarse = Vals[par]; break;
		case 1:track_data.fine = Vals[par]; break;
		case 2: break;
		case 3:expr = Vals[par];break;
		case 4: break;
		case 5: break;
		case 6:track_data.envelope_cutoff = Vals[par]; break;
		case 7:track_data.cutoff_sweep = Vals[par]; break;
		case 8:track_data.cutoff_lfo_speed = Vals[par]; break;
		case 9:track_data.cutoff_lfo_depth = Vals[par]; break;
		case 10:track_data.resonance = Vals[par]; break;
		case 11:track_data.envelope_mod = Vals[par]; break;
		case 12:track_data.envelope_decay = Vals[par]; break;
		case 13:break;
		case 14:break;
		case 15:track_def_data.LFO_speed = Vals[par];break;
		case 16:track_def_data.LFO_depth = Vals[par];break;
		case 17:track_def_data.LFO_type = Vals[par];break;
		case 18:track_def_data.LFO_delay = Vals[par];break;
		case 19:track_def_data.sweep_delay = Vals[par];break;
		case 20:track_def_data.sweep_value = Vals[par];break;
		case 21:break;
		case 22:break;
		case 23:chorus.set_delay(Vals[par]);break;
		case 24:chorus.set_lfo_speed(Vals[par]);break;
		case 25:chorus.set_lfo_depth(Vals[par]);break;
		case 26:chorus.set_feedback(Vals[par]);break;
		case 27:chorus.set_width(Vals[par]);break;
	}
	if (par<20){
		for(int c=0;c<MAX_TRACKS;c++)
			{
				track[c].set_data(&track_data);
				track[c].set_default_data(&track_def_data);
				track[c].set_expression(expr);
			}
	}
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[2048];

sprintf(
		buffer,"%s",
		"LegaSynth TB303\n"
		);

pCB->MessBox(buffer,"LegaSynth TB303",0);

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
	//float sl=0;
	bool const use_chorus=(Vals[22]==1);

	std::vector<float> s_l(numsamples), s_r(numsamples);

	for(int c=0;c<tracks;c++)
	{
		s_l.assign(numsamples,0);
		s_r.assign(numsamples,0);

		TB303_Voice *ptrack=&track[c];
		
		ptrack->mix(numsamples,&s_l.front(),&s_r.front());

		float *xpsamplesleft=psamplesleft;
		float *xpsamplesright=psamplesright;
		--xpsamplesleft;
		--xpsamplesright;

		for(int k=0;k<numsamples;k++)
		{
			float &sl = s_l.at(k);
			float &sr = s_r.at(k);
			*++xpsamplesleft+=sl;
			*++xpsamplesright+=sr;
		}
	}
	if (use_chorus){
			//--psamplesleft;
			//--psamplesright;
			chorus.process(psamplesleft,psamplesright,numsamples);
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	if(param==17)
	{
		switch(value)
		{
		case 0:sprintf(txt,"Sine");return true;break;
		case 1:sprintf(txt,"Sawtooth");return true;break;
		case 2:sprintf(txt,"Square");return true;break;
		}
	}

	if (param==22)
	{
		sprintf(txt,value?"On":"Off");
		return true;
	}
	return false;}

//////////////////////////////////////////////////////////////////////
// The SeqTick function where your notes and pattern command handlers
// should be processed. Called each tick.
// Is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	// Note Off			== 120
	// Empty Note Row	== 255
	// Less than note off value??? == NoteON!
	
	if( cmd == 0x0C) track[channel].set_preamp(val * .003921568627450980392156862745098f); // 1/255
	
	if(note!=255)
	{
		// Note off
		if(note==120)
		{
			track[channel].set_note_off(0);
		}
		else 
		// Note on
		{
			//track[channel].set_note_off(127);
			track[channel].set_note(note,Vals[3]);
		}
	}
}
