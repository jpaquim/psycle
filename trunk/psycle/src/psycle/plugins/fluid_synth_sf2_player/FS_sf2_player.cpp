/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov FluidSynth plugin for PSYCLE
//
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <fluidsynth/fluidsynth.h>

#define NUMPARAMETERS	24
#define FILEVERSION		1
#define MAXPATH			260

struct SYNPAR
{
	int version;
	char SFPathName[MAXPATH];

	int curSFid;

	int curPreset;
	int curChannel;

	int curBank;
	int curProgram;

	int Pitch;
	int WheelSens;

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
	int midi_channels;
	int interpolation;
};

CMachineParameter const paraBank = 
{
	"Bank", "Bank", 0, 255, MPF_STATE, 0
};

CMachineParameter const paraProgram = 
{
	"Program", "Program", 0, 255, MPF_STATE, 0
};

CMachineParameter const paraPitchBend = 
{
	"Pitch bend", "Pitch", -32767, 32767, MPF_STATE, 0
};

CMachineParameter const paraWheelSens = 
{
	"Wheel sens", "Wheel", -32767, 32767, MPF_STATE, 0
};


CMachineParameter const paraReverb = 
{
	"Reverb", "Reverb", 0, 1, MPF_STATE, 0
};

CMachineParameter const paraReverbRoom = 
{
	"Room", "Room", 0, 32767, MPF_STATE, 6553
};

CMachineParameter const paraReverbDamp = 
{
	"Damp", "Damp", 0, 32767, MPF_STATE, 0
};

CMachineParameter const paraReverbWidth = 
{
	"Width", "Width", 0, 32767, MPF_STATE, 16383
};

CMachineParameter const paraReverbLevel = 
{
	"Level", "Level", 0, 32767, MPF_STATE, 29490
};

CMachineParameter const paraChorus = 
{
	"Chorus", "Chorus", 0, 1, MPF_STATE, 0
};

CMachineParameter const paraChorusNr = 
{
	"No", "No", 1, 12, MPF_STATE, FLUID_CHORUS_DEFAULT_N
};

CMachineParameter const paraChorusLevel = 
{
	"Level", "Level", 0, 32767, MPF_STATE, 10000
};
CMachineParameter const paraChorusSpeed = 
{
	"Speed", "Speed", 0, 32767, MPF_STATE, 9803
};

CMachineParameter const paraChorusDepth = 
{
	"Depth", "Depth", 0, 32767, MPF_STATE, 8
};

CMachineParameter const paraChorusType = 
{
	"Mode", "Mode", FLUID_CHORUS_MOD_SINE, FLUID_CHORUS_MOD_TRIANGLE, MPF_STATE, FLUID_CHORUS_MOD_SINE
};

CMachineParameter const paraPolyphony = 
{
	"Polyphony", "Polyphony", 1, 256, MPF_STATE, 128
};

CMachineParameter const paraInter = 
{
	"Interpolation", "Interpolation", FLUID_INTERP_NONE, FLUID_INTERP_HIGHEST, MPF_STATE, FLUID_INTERP_DEFAULT
};

CMachineParameter const paraNull = 
{
	"", "", 0, 0, MPF_STATE, 0
};

CMachineParameter const *pParameters[] = 
{ 
	&paraBank,
	&paraProgram,
	&paraNull,
	&paraPitchBend,
	&paraWheelSens,
	&paraNull,

	&paraReverb,
	&paraReverbRoom,
	&paraReverbDamp,
	&paraReverbWidth,
	&paraReverbLevel,
	&paraNull,

	&paraChorus,
	&paraChorusNr,
	&paraChorusLevel,
	&paraChorusSpeed,
	&paraChorusDepth,
	&paraChorusType,

	&paraPolyphony,
	&paraInter,
	&paraNull,
	&paraNull,
	&paraNull,
	&paraNull
};

enum {
	e_paraBank,
	e_paraProgram,
	e_paraNull0,
	e_paraPitchBend,
	e_paraWheelSens,
	e_paraNull1,

	e_paraReverb,
	e_paraReverbRoom,
	e_paraReverbDamp,
	e_paraReverbWidth,
	e_paraReverbLevel,
	e_paraNull2,

	e_paraChorus,
	e_paraChorusNr,
	e_paraChorusLevel,
	e_paraChorusSpeed,
	e_paraChorusDepth,
	e_paraChorusType,

	e_paraPolyphony,
	e_paraInter,
	e_paraNull3,
	e_paraNull4,
	e_paraNull5,
	e_paraNull6,
//	NUMPARAMETERS
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	GENERATOR,								// flags
	NUMPARAMETERS,							// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"FluidSynth Fluid Synth SF2 player (Debug build)",		// name
#else
	"Fluid Synth SF2 player",						// name
#endif
	"FluidSynth",							// short name
	"Peter Hanappe and others\n\thttp://fluidsynth.org/\n\nSoundFont(R) is a registered trademark of E-mu Systems, Inc.\n\n\t\tported by Sartorius",							// author
	"Load SoundFont",									// A command, that could be use for open an editor, etc...
	4
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

	virtual void PutData(byte * pData);
	virtual void GetData(byte * pData);

	virtual int GetDataSize() { return sizeof(SYNPAR); }

	
	SYNPAR globalpar;

	void SetParams();

	bool LoadSF(const char * sffile);


private:

	fluid_settings_t* settings;
	fluid_synth_t* synth;

	int sf_id;

	int lastnote[MAX_TRACKS];

	//reverb
	double roomsize, damping, width, r_level;
	//chorus
	double nr, c_level, speed, depth_ms, type;
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
	
	if (synth) delete_fluid_synth(synth);
    if (settings) delete_fluid_settings(settings);

}

void mi::Init()
{
// Initialize your stuff here
	
	sf_id = 0;

	globalpar.version = FILEVERSION;
	//globalpar.SFPathName = "\0";
	globalpar.curSFid = sf_id;
	globalpar.curPreset = 0;
	globalpar.curChannel = 1;
	globalpar.curBank = 0;
	globalpar.curProgram = 0;
	globalpar.Pitch = 0;
	globalpar.WheelSens = 0;

	globalpar.reverb_on = paraReverb.DefValue;
	globalpar.roomsize = paraReverbRoom.DefValue;	roomsize = .2;
	globalpar.damping = paraReverb.DefValue;		damping = 0;
	globalpar.width = paraReverbWidth.DefValue;		width = .5;
	globalpar.r_level = paraReverbLevel.DefValue;	r_level = 0.9;


	globalpar.chorus_on = paraChorus.DefValue;
	globalpar.nr = nr = FLUID_CHORUS_DEFAULT_N;
	globalpar.c_level = paraChorusLevel.DefValue;	c_level = 2.;
	globalpar.speed = paraChorusSpeed.DefValue;		speed = .3;
	globalpar.depth_ms = paraChorusDepth.DefValue;	depth_ms = 8.;
	globalpar.type = type = FLUID_CHORUS_DEFAULT_TYPE;

	
	globalpar.polyphony = 128;
	globalpar.midi_channels = 16;

	globalpar.interpolation = FLUID_INTERP_DEFAULT;


	settings = new_fluid_settings();
	synth = new_fluid_synth(settings);

	SetParams();
}

void mi::PutData(byte* pData)
{
	if ((pData == NULL) || (((SYNPAR*)pData)->version != FILEVERSION))
	{
		pCB->MessBox("WARNING!\nThis fileversion does not match current plugin's fileversion.\nYour settings are probably fucked.","FluidSynth",0);
		return;
	}
	memcpy(&globalpar, pData, sizeof(SYNPAR));
	// ok now fill the vals structure

	if (LoadSF(globalpar.SFPathName))
	{
	}
		Vals[e_paraBank]=			globalpar.curBank;
		Vals[e_paraProgram]=		globalpar.curProgram;

		Vals[e_paraPitchBend]=		globalpar.Pitch;
		Vals[e_paraWheelSens]=		globalpar.WheelSens;

		Vals[e_paraReverb]=			globalpar.reverb_on;
		Vals[e_paraReverbRoom]=		globalpar.roomsize;
		Vals[e_paraReverbDamp]=		globalpar.damping;
		Vals[e_paraReverbWidth]=	globalpar.width;
		Vals[e_paraReverbLevel]=	globalpar.r_level; 

		Vals[e_paraChorus]=			globalpar.chorus_on;
		Vals[e_paraChorusNr]=		globalpar.nr;
		Vals[e_paraChorusLevel]=	globalpar.c_level;
		Vals[e_paraChorusSpeed]=	globalpar.speed;
		Vals[e_paraChorusDepth]=	globalpar.depth_ms;
		Vals[e_paraChorusType]=		globalpar.type;

		Vals[e_paraPolyphony]=		globalpar.polyphony;
		Vals[e_paraInter]=			globalpar.interpolation;

		SetParams();
	//} else {
	//	pCB->MessBox("WARNING!\nSomething wrong...","FluidSynth",0);
	//	return;
	//}
}

void mi::GetData(byte* pData)
{
	//if (pData)
	//{
		memcpy(pData, &globalpar, sizeof(SYNPAR));
	//}
}


void mi::Stop()
{
	fluid_synth_system_reset(synth);
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	Vals[par]=val;
/*		Vals[e_paraBank]=			globalpar.curBank;
		Vals[e_paraProgram]=		globalpar.curProgram;

		Vals[e_paraPitchBend]=		globalpar.Pitch;
		Vals[e_paraWheelSens]=		globalpar.WheelSens;

		Vals[e_paraReverb]=			globalpar.reverb_on;
		Vals[]=		;
		Vals[]=		globalpar.damping;
		Vals[e_paraReverbWidth]=	globalpar.width;
		Vals[e_paraReverbLevel]=	globalpar.r_level; 

		Vals[e_paraChorus]=			globalpar.chorus_on;
		Vals[e_paraChorusNr]=		globalpar.nr;
		Vals[e_paraChorusbLevel]=	globalpar.c_level;
		Vals[e_paraChorusSpeed]=	globalpar.speed;
		Vals[e_paraChorusDepth]=	globalpar.depth_ms;
		Vals[e_paraChorusType]=		globalpar.type;

		Vals[e_paraPolyphony]=		globalpar.polyphony;
*/		
	switch(par)
	{
		case e_paraReverb:
			globalpar.reverb_on = val;
			fluid_synth_set_reverb_on(synth, globalpar.reverb_on);
			if (globalpar.reverb_on)
			{
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbRoom:
			globalpar.roomsize = val;
			roomsize = globalpar.roomsize /32767;
			if (globalpar.reverb_on)
			{
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbDamp:
			globalpar.damping = val;
			damping = globalpar.damping /32767;
			if (globalpar.reverb_on)
			{
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbWidth:
			globalpar.width = val;
			width = globalpar.width /32767;
			if (globalpar.reverb_on)
			{
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraReverbLevel:
			globalpar.r_level = val;
			r_level = globalpar.r_level /32767;
			if (globalpar.reverb_on)
			{
				fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
			}
			break;
		case e_paraChorus:
			globalpar.chorus_on = val;
			fluid_synth_set_chorus_on(synth, globalpar.chorus_on);
			if(globalpar.chorus_on)
			{
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraChorusNr:
			globalpar.nr = nr = val;
			if(globalpar.chorus_on)
			{
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraChorusLevel:
			globalpar.c_level = val; 
			c_level = (double)globalpar.c_level;
			if(globalpar.chorus_on)
			{
				fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
			}
			break;
		case e_paraPolyphony:
            globalpar.polyphony = val;
			fluid_settings_setint(settings, "synth.polyphony", globalpar.polyphony);
			break;
		case e_paraInter:
            globalpar.interpolation = val;
			fluid_synth_set_interp_method(synth, -1, globalpar.interpolation);
			break;
		default:
			break;
	}

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
	if(sf_id)
	{
		float *xpl, *xpr;
		xpl = psamplesleft;
		xpr = psamplesright;
		--xpl;
		--xpr;
		
		fluid_synth_write_float(synth, numsamples, psamplesleft, 0, 1, psamplesright, 0, 1);
		
		for (int i =0; i<numsamples;i++)
		{
			*++xpl *= 32767.f;
			*++xpr *= 32767.f;
		}
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	return false;
}

//////////////////////////////////////////////////////////////////////
// The SeqTick function where your notes and pattern command handlers
// should be processed. Called each tick.
// Is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	// Note Off			== 120
	// Empty Note Row	== 255
	// Less than note off value??? == NoteON!

	if (note<120)
	{
        if (lastnote[channel]!=255) fluid_synth_noteoff(synth, globalpar.curChannel, lastnote[channel]);
        lastnote[channel] = note;
        fluid_synth_noteon(synth, globalpar.curChannel, note, 127);
	}
	if (note==120)
	{
		fluid_synth_noteoff(synth, globalpar.curChannel, lastnote[channel]);
		lastnote[channel] = 255;
	}


}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button

   char szFilters[]="SF2 (*.sf2)|*.sf2|All Files (*.*)|*.*||";

   // Create an Open dialog; the default file name extension is ".my".
   CFileDialog fileDlg (TRUE, "SF2", "*.sf2", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, NULL);
   
   // Display the file dialog. When user clicks OK, fileDlg.DoModal() 
   // returns IDOK.
   if( fileDlg.DoModal ()==IDOK )
   {
		std::string sfPathName = fileDlg.GetPathName().GetBuffer(4);
		const char * sffile = sfPathName.c_str();
		bool ok = LoadSF(sffile);
   }
}

void mi::SetParams()
{
	fluid_settings_setint(settings, "synth.sample-rate", pCB->GetSamplingRate());
    fluid_settings_setint(settings, "synth.polyphony", globalpar.polyphony);
    fluid_settings_setint(settings, "synth.midi-channels", globalpar.midi_channels);

	fluid_synth_set_reverb_on(synth, globalpar.reverb_on);
    fluid_synth_set_chorus_on(synth, globalpar.chorus_on);
	fluid_synth_set_interp_method(synth, -1, globalpar.interpolation);
    
    fluid_synth_system_reset( synth );
    
	for (int i=0;i<MAX_TRACKS;i++) lastnote[i]=255;
}

bool mi::LoadSF(const char *sf_file)
{
	//char* temp;
	//std::sprintf(temp, sf_file);
	//pCB->MessBox(temp,"SF2 Loader",0);
	if(fluid_is_soundfont(sf_file))
		{
			if(sf_id)
			{
				fluid_synth_sfunload(synth,sf_id,1);
			}
			sf_id = fluid_synth_sfload(synth, sf_file, 1);
			if(sf_id==-1)
			{
				pCB->MessBox("Error loading!","SF2 Loader",0);
				sf_id = 0;
				return false;
			}
			fluid_synth_sfont_select(synth, globalpar.curChannel, sf_id);
			fluid_synth_bank_select(synth, globalpar.curChannel, globalpar.curBank);
			
			std::sprintf(globalpar.SFPathName, sf_file);
			return true;

		} else {
			//pCB->MessBox("It's not a SoundFont file or file not found!","SF2 Loader",0);
			return false;
		}
}