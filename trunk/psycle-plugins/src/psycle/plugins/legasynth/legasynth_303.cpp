/* -*- mode:c++, indent-tabs-mode:t -*- */
/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth TB303 plugin for PSYCLE
// v0.2 beta
//

#include <packageneric/pre-compiled.private.hpp>
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
	"Coarse",																																				// description
	-24,																																												// MinValue				
	24,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraFine = 
{
	"Fine",
	"Fine",																																				// description
	-65535,																																												// MinValue				
	65535,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraPB = 
{
	"Pitch bend",
	"Pitch bend",																																				// description
	0,																																												// MinValue				
	0x2000*2,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0x2000
};

CMachineParameter const paraPD = 
{
	"Pitch depth",
	"Pitch depth",																																				// description
	0,																																												// MinValue				
	12,																																												// MaxValue
	MPF_STATE,																																								// Flags
	12
};

CMachineParameter const paraExpression = 
{
	"Expression",
	"Expression",																																				// description
	0,																																												// MinValue				
	127,																																												// MaxValue
	MPF_STATE,																																								// Flags
	127
};

CMachineParameter const paraDist = 
{
	"Distortion",
	"Distortion",																																				// description
	0,																																												// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraFilter = 
{
	"Envelope&Filter",
	"Envelope&Filter",																																				// description
	0,																																												// MinValue				
	1,																																												// MaxValue
	MPF_STATE||MPF_LABEL,																								// Flags
	0
};


CMachineParameter const paraCutOff = 
{
	"Cutoff",
	"Cutoff",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0x7FFF
};

CMachineParameter const paraCutSweep = 
{
	"Cutoff sweep",
	"Cutoff sweep",																																				// description
	-127,																																												// MinValue				
	127,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraCutLFOSpeed = 
{
	"Cutoff LFO speed",
	"Cutoff",																																				// description
	0,																																												// MinValue				
	0xFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraCutLFODepth = 
{
	"Cutoff LFO depth",
	"Cutoff LFO depth",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};


CMachineParameter const paraResonance = 
{
	"Resonance",
	"Resonance",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};
CMachineParameter const paraMod = 
{
	"Modulation",
	"Modulation",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraDecay = 
{
	"Decay",
	"Modulation",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraLFO = 
{
	"LFO&Sweep",
	"LFO&Sweep",																																				// description
	0,																																												// MinValue				
	1,																																												// MaxValue
	MPF_STATE||MPF_LABEL,																								// Flags
	0
};

CMachineParameter const paraSpeed = 
{
	"Speed",
	"Speed",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraDepth = 
{
	"Depth",
	"Depth",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraType = 
{
	"Type",
	"Type",																																				// description
	0,																																												// MinValue				
	2,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraDelay = 
{
	"Delay",
	"Delay",																																				// description
	0,																																												// MinValue				
	0xFFFF,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraSweepDelay = 
{
	"Sweep delay",
	"Sweep delay",																																				// description
	0,																																												// MinValue				
	1/*xFFFF*/,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraSweepValue = 
{
	"Sweep value",
	"Sweep value",																																				// description
	//-127,																																												// MinValue				
	//127,																																												// MaxValue
	0,																																												// MinValue				
	1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};
CMachineParameter const paraChorus = 
{
	"Chorus",
	"Chorus",																																				// description
	0,																																												// MinValue				
	1,																																												// MaxValue
	MPF_STATE||MPF_LABEL,																																								// Flags
	0
};

CMachineParameter const paraChorusOnOff = 
{
	"On/Off",
	"On/Off",																																				// description
	0,																																												// MinValue				
	1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0
};
CMachineParameter const paraChorusDelay = 
{
	"Delay",
	"Delay",																																				// description
	0,																																												// MinValue				
	127,																																												// MaxValue
	MPF_STATE,																																								// Flags
	3
};

CMachineParameter const paraChorusLFOSpeed = 
{
	"LFO Speed",
	"LFO Speed",																																				// description
	0,																																												// MinValue				
	127,																																												// MaxValue
	MPF_STATE,																																								// Flags
	2
};

CMachineParameter const paraChorusLFODepth = 
{
	"LFO Depth",
	"LFO Depth",																																				// description
	0,																																												// MinValue				
	127,																																												// MaxValue
	MPF_STATE,																																								// Flags
	1
};


CMachineParameter const paraChorusFB = 
{
	"Feedback",
	"Feedback",																																				// description
	1,																																												// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	64
};

CMachineParameter const paraChorusWidth = 
{
	"Width",
	"Width",																																				// description
	-127,																																												// MinValue				
	127,																																												// MaxValue
	MPF_STATE,																																								// Flags
	-10
};


CMachineParameter const *pParameters[] = 
{ 
	&paraCoarse,
	&paraFine,
	&paraPB,
	&paraPD,
	&paraExpression,
	&paraDist,
	&paraFilter,
	&paraCutOff,
	&paraCutSweep,
	&paraCutLFOSpeed,
	&paraCutLFODepth,
	&paraResonance,
	&paraMod,
	&paraDecay,
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
	GENERATOR,																																// flags
	NUMPARAMETERS,																												// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	"LegaSynth TB303 (Debug build)",								// name
#else
	"LegaSynth TB303",																								// name
#endif
	"TB303",																												// short name
	"Juan Linietsky, ported by Sartorius",																												// author
	"Help",																																				// A command, that could be use for open an editor, etc...
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
	Chorus				chorus;
	int samplerate;
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
	samplerate = pCB->GetSamplingRate();
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

		track[i].set_mix_frequency(samplerate);
		track[i].set_note_off(0);
		track[i].set_sustain(false);
	}
	chorus.set_mixfreq(samplerate);
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
	if(samplerate!=pCB->GetSamplingRate())
	{
		samplerate=pCB->GetSamplingRate();
		chorus.set_mixfreq(samplerate);
		for(int i=0;i<MAX_TRACKS;i++)
			track[i].set_mix_frequency(samplerate);
	}

}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	int pb = Vals[2];
	int pd = Vals[3];
	char expr = (Vals[4]&0xFF);
	Vals[par]=val;
	switch (par) {
		case 0:track_data.coarse = val; break;
		case 1:track_data.fine = val; break;
		case 2:pb = val; break;
		case 3:pd = val; break;
		case 4:expr = (val&0xFF); break;
		case 5: break;
		case 6: break;
		case 7:track_data.envelope_cutoff = val; break;
		case 8:track_data.cutoff_sweep = val; break;
		case 9:track_data.cutoff_lfo_speed = val; break;
		case 10:track_data.cutoff_lfo_depth = val; break;
		case 11:track_data.resonance = val; break;
		case 12:track_data.envelope_mod = val; break;
		case 13:track_data.envelope_decay = val;break;
		case 14:break;
		case 15:track_def_data.LFO_speed = val;break;
		case 16:track_def_data.LFO_depth = val;break;
		case 17:track_def_data.LFO_type = val;break;
		case 18:track_def_data.LFO_delay = val;break;
		case 19:track_def_data.sweep_delay = 0;break; // Not yet available
		case 20:track_def_data.sweep_value = 0;break; // Not yet available
		case 21:break;
		case 22:break;
		case 23:chorus.set_delay(val);break;
		case 24:chorus.set_lfo_speed(val);break;
		case 25:chorus.set_lfo_depth(val);break;
		case 26:chorus.set_feedback(val);break;
		case 27:chorus.set_width(val);break;
	}
	if (par<14){
		for(int c=0;c<MAX_TRACKS;c++)
			{
				track[c].set_data(&track_data);
				track[c].set_pitch_bend(pb);
				track[c].set_pitch_depth(pd);
				track[c].set_expression(expr);
			}
	} else if (par>14&&par<21) {
		for(int c=0;c<MAX_TRACKS;c++)
			{
				track[c].set_default_data(&track_def_data);
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
	bool const use_chorus=(Vals[22]==1);
	float const dist=(float)Vals[5]*.01*.0000015;
	//std::vector<float> s_l(numsamples), s_r(numsamples);
	std::vector<int> s_l(numsamples), s_r(numsamples);

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
			/*float*/ int &sl = s_l.at(k);
			/*float*/ int &sr = s_r.at(k);
			*++xpsamplesleft+=(dist>0?(float)(sl-dist*(sl*sl+sl*sl*sl)):(float)sl);
			*++xpsamplesright+=(dist>0?(float)(sr-dist*(sr*sr+sr*sr*sr)):(float)sr);
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
	if(param==1)
	{
		sprintf(txt,"%.6f",(float)value/65535.f);
		return true;
	}

	if(param==5)
	{
		sprintf(txt,"%i %%",value);
		return true;
	}
	if(param==17)
	{
		switch(value)
		{
		case 0:sprintf(txt,"Sine");return true;break;
		case 1:sprintf(txt,"Sawtooth");return true;break;
		case 2:sprintf(txt,"Square");return true;break;
		}
	}
	
	if (param==19 || param==20)
	{
		sprintf(txt,"N/A");
		return true;
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
	// Note Off												== 120
	// Empty Note Row				== 255
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
			track[channel].set_note(note,Vals[4]);
		}
	}
}
