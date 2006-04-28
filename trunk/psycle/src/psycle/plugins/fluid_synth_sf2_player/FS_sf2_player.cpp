/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov FluidSynth plugin for PSYCLE
//
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <fluidsynth/fluidsynth.h>
#include <memory.h>

#define NUMPARAMETERS   24
#define FILEVERSION             1
#define MAXPATH                 (MAX_PATH < (1 << 12) ? (1 << 12) : MAX_PATH)
#define MAXMIDICHAN             MAX_TRACKS

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

CMachineParameter const paraChannel = 
{
        "Channel", "Channel", 0, MAXMIDICHAN, MPF_STATE, 0
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
        "Wheel sens", "Wheel", -127, 127, MPF_STATE, 0
};


CMachineParameter const paraReverb = 
{
        "Reverb", "Reverb", 0, 1, MPF_STATE, 0
};

CMachineParameter const paraReverbRoom = 
{
        "Room", "Room", 0, 100, MPF_STATE, 20
};

CMachineParameter const paraReverbDamp = 
{
        "Damp", "Damp", 0, 100, MPF_STATE, 0
};

CMachineParameter const paraReverbWidth = 
{
        "Width", "Width", 0, 100, MPF_STATE, 50
};

CMachineParameter const paraReverbLevel = 
{
        "Level", "Level", 0, 1000, MPF_STATE, 900
};

CMachineParameter const paraChorus = 
{
        "Chorus", "Chorus", 0, 1, MPF_STATE, 0
};

CMachineParameter const paraChorusNr = 
{
        "No", "No", 1, 99, MPF_STATE, FLUID_CHORUS_DEFAULT_N
};

CMachineParameter const paraChorusLevel = 
{
        "Level", "Level", 1, 10000, MPF_STATE, 2000
};
CMachineParameter const paraChorusSpeed = 
{
        "Speed", "Speed", 3, 50, MPF_STATE, 3
};

CMachineParameter const paraChorusDepth = 
{
        "Depth", "Depth", 0, 1000, MPF_STATE, 8
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
        "Interpolation", "Interpolation", FLUID_INTERP_NONE, FLUID_INTERP_HIGHEST+3, MPF_STATE, FLUID_INTERP_DEFAULT
};

CMachineParameter const paraNull = 
{
        "", "", 0, 0, MPF_STATE, 0
};

CMachineParameter const *pParameters[] = 
{ 
        &paraChannel,
        &paraBank,
        &paraProgram,
        &paraNull,
        &paraPitchBend,
        &paraWheelSens,

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
        e_paraChannel,
        e_paraBank,
        e_paraProgram,
        e_paraNull0,
        e_paraPitchBend,
        e_paraWheelSens,

        e_paraReverb,
        e_paraReverbRoom,
        e_paraReverbDamp,
        e_paraReverbWidth,
        e_paraReverbLevel,
        e_paraNull1,

        e_paraChorus,
        e_paraChorusNr,
        e_paraChorusLevel,
        e_paraChorusSpeed,
        e_paraChorusDepth,
        e_paraChorusType,

        e_paraPolyphony,
        e_paraInter,
        e_paraNull2,
        e_paraNull3,
        e_paraNull4,
        e_paraNull5,
//      NUMPARAMETERS
};

CMachineInfo const MacInfo = 
{
        MI_VERSION,     
        GENERATOR,                                                              // flags
        NUMPARAMETERS,                                                  // numParameters
        pParameters,                                                    // Pointer to parameters
#ifdef _DEBUG
        "FluidSynth Fluid Synth SF2 player (Debug build)",              // name
#else
        "Fluid Synth SF2 player",                                               // name
#endif
        "FluidSynth",                                                   // short name
        "Peter Hanappe and others\n\thttp://fluidsynth.org/\n\nSoundFont(R) is a registered trademark of E-mu Systems, Inc.\n\n\t\tported by Sartorius",                                                        // author
        "Load SoundFont",                                                                       // A command, that could be use for open an editor, etc...
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

        virtual void PutData(void * pData);
        virtual void GetData(void * pData);

        virtual int GetDataSize() { return sizeof(SYNPAR); }

        
        SYNPAR globalpar;

        void SetParams();

        bool LoadSF(const char * sffile);
        void BuildPresets();


private:

        fluid_settings_t* settings;
        fluid_synth_t* synth;
        fluid_preset_t* preset;

        int sf_id;
        int banks[129],progs[129];
        int max_bank_index;
        int midi_channel[MAXMIDICHAN];
        int lastnote[MAXMIDICHAN];

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
        
        max_bank_index = -1;
        //for (int i = 0; i<129; i++)
        //{
        //      banks[i] = -1;
        //      progs[i] = -1;
        //}
        
        sf_id = 0;

        globalpar.version               = FILEVERSION;
        std::memset(&globalpar.SFPathName,0,MAXPATH);
        globalpar.curSFid               = sf_id;
        globalpar.curPreset             = 0;
        globalpar.curChannel    = paraChannel.DefValue;
        globalpar.curBank               = 0;
        globalpar.curProgram    = 0;
        globalpar.Pitch                 = 0;
        globalpar.WheelSens             = 0;

        globalpar.reverb_on             = paraReverb.DefValue;
        globalpar.roomsize              = paraReverbRoom.DefValue;      
        globalpar.damping               = paraReverb.DefValue;          
        globalpar.width                 = paraReverbWidth.DefValue;             
        globalpar.r_level               = paraReverbLevel.DefValue;     

        roomsize         =      (double)globalpar.roomsize * .01;
        damping          =      (double)globalpar.damping * .01;
        width            =      (double)globalpar.width * .01;
        r_level          =      (double)globalpar.r_level * .001;


        globalpar.chorus_on             = paraChorus.DefValue;
        globalpar.nr                    = nr = FLUID_CHORUS_DEFAULT_N;
        globalpar.c_level               = paraChorusLevel.DefValue;     
        globalpar.speed                 = paraChorusSpeed.DefValue;             
        globalpar.depth_ms              = paraChorusDepth.DefValue;     
        globalpar.type                  = type = FLUID_CHORUS_DEFAULT_TYPE;

        c_level         =       (double)globalpar.c_level * .001;
        speed           =       (double)globalpar.speed * .1;
        depth_ms        =       (double)globalpar.depth_ms;

        
        globalpar.polyphony             = 128;
        globalpar.midi_channels = MAXMIDICHAN;

        globalpar.interpolation = FLUID_INTERP_DEFAULT;


        settings = new_fluid_settings();
        
#ifdef DEBUG
        fluid_settings_setstr(settings, "synth.verbose", "yes");
#endif
        fluid_settings_setint(settings, "synth.sample-rate", pCB->GetSamplingRate());
    fluid_settings_setint(settings, "synth.polyphony", globalpar.polyphony);
        fluid_settings_setint(settings, "synth.interpolation", globalpar.interpolation);
    fluid_settings_setint(settings, "synth.midi-channels", globalpar.midi_channels);
        
        synth = new_fluid_synth(settings);
        SetParams();

}

void mi::SetParams()
{
        //fluid_settings_setint(settings, "synth.sample-rate", pCB->GetSamplingRate());
 //   fluid_settings_setint(settings, "synth.polyphony", globalpar.polyphony);
 //   fluid_settings_setint(settings, "synth.midi-channels", globalpar.midi_channels);

        fluid_synth_set_interp_method(synth, -1, globalpar.interpolation);
    fluid_synth_set_polyphony(synth, globalpar.polyphony);

        fluid_synth_set_reverb_on(synth, globalpar.reverb_on);
        if(globalpar.reverb_on)
        {
                fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
        }
    fluid_synth_set_chorus_on(synth, globalpar.chorus_on);
        if(globalpar.chorus_on)
        {
                fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
        }

    fluid_synth_system_reset( synth );
    
        for (int i=0;i<MAX_TRACKS;i++) lastnote[i]=255;
}

void mi::PutData(void* pData)
{
        sf_id = 0;
        if ((pData == NULL) || (((SYNPAR*)pData)->version != FILEVERSION))
        {
                pCB->MessBox("WARNING!\nThis fileversion does not match current plugin's fileversion.\nYour settings are probably fucked.","FluidSynth",0);
                return;
        }
        std::memcpy(&globalpar, pData, sizeof(SYNPAR));
        if (LoadSF(globalpar.SFPathName))
        {
                Vals[e_paraChannel]             =       globalpar.curChannel;
                Vals[e_paraBank]                =       globalpar.curBank;
                Vals[e_paraProgram]             =       globalpar.curProgram;

                Vals[e_paraPitchBend]   =       globalpar.Pitch;
                Vals[e_paraWheelSens]   =       globalpar.WheelSens;

                Vals[e_paraReverb]              =       globalpar.reverb_on;
                Vals[e_paraReverbRoom]  =       globalpar.roomsize;
                Vals[e_paraReverbDamp]  =       globalpar.damping;
                Vals[e_paraReverbWidth] =       globalpar.width;
                Vals[e_paraReverbLevel] =       globalpar.r_level;
                
                roomsize         =      (double)globalpar.roomsize * .01;
                damping          =      (double)globalpar.damping * .01;
                width            =      (double)globalpar.width * .01;
                r_level          =      (double)globalpar.r_level * .001;

                Vals[e_paraChorus]              =       globalpar.chorus_on;
                Vals[e_paraChorusNr]    =       nr = globalpar.nr;
                Vals[e_paraChorusLevel] =       globalpar.c_level;
                Vals[e_paraChorusSpeed] =       globalpar.speed;
                Vals[e_paraChorusDepth] =       globalpar.depth_ms;
                Vals[e_paraChorusType]  =       type = globalpar.type;

                c_level         =       (double)globalpar.c_level * .001;
                speed           =       (double)globalpar.speed * .1;
                depth_ms        =       (double)globalpar.depth_ms;

                Vals[e_paraPolyphony]   =       globalpar.polyphony;
                Vals[e_paraInter]               =       globalpar.interpolation;

                SetParams();
        } else {
                pCB->MessBox("WARNING!\nSomething wrong...","FluidSynth",0);
                return;
        }
}

void mi::GetData(void* pData)
{
        if (pData)
        {
                std::memcpy(pData, &globalpar, sizeof(SYNPAR));
        }
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
                
        switch(par)
        {
                case e_paraChannel:
                        globalpar.curChannel = val;
                        if(synth)
                        {
                                preset = fluid_synth_get_channel_preset(synth, globalpar.curChannel);
                                if(preset!=NULL)
                                {
                                        globalpar.curBank = (*(preset)->get_banknum)(preset);
                                        Vals[e_paraBank] = globalpar.curBank;
                                        globalpar.curProgram = (*(preset)->get_num)(preset);
                                        Vals[e_paraProgram] = globalpar.curProgram;
                                }
                        }
                        break;
                case e_paraBank:
                        if (Vals[par]>max_bank_index && max_bank_index!=-1)
                        {
                                Vals[e_paraBank] = max_bank_index;
                        }
                        globalpar.curBank = banks[Vals[e_paraBank]];
                        fluid_synth_bank_select(synth, globalpar.curChannel, globalpar.curBank);
                        globalpar.curProgram = Vals[e_paraProgram] = progs[globalpar.curBank];
                        fluid_synth_program_change(synth, globalpar.curChannel, globalpar.curProgram);
                        break;
                case e_paraProgram:
                        globalpar.curProgram = val;
                        fluid_synth_program_change(synth, globalpar.curChannel, globalpar.curProgram);
                        break;
                case e_paraPitchBend:
                        globalpar.Pitch = val;
                        fluid_synth_pitch_bend(synth, globalpar.curChannel, globalpar.Pitch);
                        break;
                case e_paraWheelSens:
                        globalpar.WheelSens = val;
                        fluid_synth_pitch_wheel_sens(synth, globalpar.curChannel, globalpar.WheelSens);
                        break;
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
                        roomsize = globalpar.roomsize * .01;
                        if (globalpar.reverb_on)
                        {
                                fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
                        }
                        break;
                case e_paraReverbDamp:
                        globalpar.damping = val;
                        damping = globalpar.damping * .01;
                        if (globalpar.reverb_on)
                        {
                                fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
                        }
                        break;
                case e_paraReverbWidth:
                        globalpar.width = val;
                        width = globalpar.width * .01;
                        if (globalpar.reverb_on)
                        {
                                fluid_synth_set_reverb(synth, roomsize, damping, width, r_level);
                        }
                        break;
                case e_paraReverbLevel:
                        globalpar.r_level = val;
                        r_level = globalpar.r_level * .001;
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
                        c_level = (double)globalpar.c_level*.001;
                        if(globalpar.chorus_on)
                        {
                                fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
                        }
                        break;
                case e_paraChorusSpeed:
                        globalpar.speed = val; 
                        speed = (double)globalpar.speed *.1;
                        if(globalpar.chorus_on)
                        {
                                fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
                        }
                        break;
                case e_paraChorusDepth:
                        globalpar.depth_ms = val; 
                        depth_ms = (double)globalpar.depth_ms;
                        if(globalpar.chorus_on)
                        {
                                fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
                        }
                        break;
                        case e_paraChorusType:
                        globalpar.type = type = val;
                        if(globalpar.chorus_on)
                        {
                                fluid_synth_set_chorus(synth, nr, c_level, speed, depth_ms, type);
                        }
                        break;
                case e_paraPolyphony:
            globalpar.polyphony = val;
                        fluid_synth_set_polyphony(synth, globalpar.polyphony);
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
        switch(param)
        {
                case e_paraBank:
                        std::sprintf(txt,"%i",banks[value]);
                        return true;
                case e_paraProgram:
                        fluid_preset_t* preset;
                        preset = fluid_synth_get_channel_preset(synth, globalpar.curChannel);
                        if(preset!=NULL)
                        {
                                std::sprintf(txt,"%d: %s",(*(preset)->get_num)(preset),(*(preset)->get_name)(preset));
                        } else {
                                std::sprintf(txt,"(none)");
                        }
                        return true;
                case e_paraReverb:
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
                case e_paraChorus:
                        std::sprintf(txt,value?"On":"Off");
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
                        switch(value)
                        {
                        case FLUID_INTERP_NONE:
                                std::sprintf(txt,"None");
                                return true;
                        case FLUID_INTERP_LINEAR:
                                std::sprintf(txt,"Linear");
                                return true;
                        case FLUID_INTERP_4THORDER:
                                std::sprintf(txt,"4th order");
                                return true;
                        case FLUID_INTERP_7THORDER:
                                std::sprintf(txt,"7th order");
                                return true;
                        default:
                                std::sprintf(txt,"%i",value);
                                return true;
                        }
                default:
                        return false;
        }
}

//////////////////////////////////////////////////////////////////////
// The SeqTick function where your notes and pattern command handlers
// should be processed. Called each tick.
// Is called by the host sequencer
        
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
        // Note Off                     == 120
        // Empty Note Row       == 255
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
                LoadSF(sffile);
   }
}

bool mi::LoadSF(const char *sf_file)
{
        if(fluid_is_soundfont(sf_file))
                {
                        if(sf_id)
                        {
                                fluid_synth_sfunload(synth,sf_id,1);
                                fluid_synth_system_reset(synth);
                        }
                        sf_id = fluid_synth_sfload(synth, sf_file, 1);
                        if(sf_id==-1)
                        {
                                pCB->MessBox("Error loading!","SF2 Loader",0);
                                sf_id = 0;
                                return false;
                        }

                        BuildPresets();

                        //for (int i=0;i<globalpar.midi_channels;i++)
                        //{
                        //      //fluid_synth_sfont_select(synth, i, sf_id);
                        //      fluid_synth_bank_select(synth, i, 0);
                        //}

                        
                        
                        std::sprintf(globalpar.SFPathName, sf_file);

                        return true;

                } else {
                        pCB->MessBox("It's not a SoundFont file or file not found!","SF2 Loader",0);
                        return false;
                }
}

void mi::BuildPresets()
{
        int i;
        int cur_bank(-1);

        max_bank_index = -1;

        for (i = 0; i<129; i++)
        {
                banks[i] = 0;
                progs[i] = 0;
        }
        const int midi_chan = fluid_synth_count_midi_channels(synth);
        for (i = 0; i < midi_chan; i++)
        {
                preset = fluid_synth_get_channel_preset(synth, i);
                if(preset!=NULL)
                {
                        if( cur_bank != (*(preset)->get_banknum)(preset))
                        {
                                cur_bank = (*(preset)->get_banknum)(preset);
                                max_bank_index++;
                                banks[max_bank_index] = cur_bank;
                                progs[max_bank_index] = (*(preset)->get_num)(preset);
                        }
                }
        }

        // set all channels to first available preset
        if(max_bank_index!=-1)
        {
                        for ( i=0; i < midi_chan; i++)
                        {
                                fluid_synth_bank_select(synth, i, banks[0]);
                                fluid_synth_program_change(synth, i, progs[banks[0]]);
                        }
        }
}
