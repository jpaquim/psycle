#include <packageneric/pre-compiled.private.hpp>
#include "y_midi.hpp"

int mi::midiopencount(0);
HMIDIOUT mi::handle(0);

//////////////////////////////////////////////////////////////////////////
// midichannel class
midichannel::midichannel()
:Chan(0)
,handle(0)
{
}

midichannel::~midichannel()
{
}

void midichannel::Init(HMIDIOUT handle_in, const int channel)
{
	handle = handle_in;
	Chan = channel;
}

void midichannel::Play(const int note, const int vol)
{
	SendMidi(BuildEvent(MIDI_NOTEON,note,vol));
}

void midichannel::Stop(const int note)
{
	SendMidi(BuildEvent(MIDI_NOTEOFF,note));
}

void midichannel::SetPatch(const int patchnum)
{
	SendMidi(BuildEvent(MIDI_PATCHCHANGE,patchnum));
}


MidiEvent midichannel::BuildEvent(const int eventtype, const int p1, const int p2)
{
	MidiEvent mevent;
	mevent.split.message= (eventtype&0xFF) | (Chan&0x0F);
	mevent.split.byte1= p1&0xFF;
	mevent.split.byte2= p2&0xFF;

	return mevent;
}

void midichannel::SendMidi(MidiEvent event)
{
	midiOutShortMsg(handle, event.value);
}
void midichannel::StopMidi()
{
	SendMidi(BuildEvent(MIDI_CCONTROLLER,0x7B)); // All Notes Off
}

//////////////////////////////////////////////////////////////////////////
//  mi machine class.

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	int i;
	Vals=new int[NUMPARAMETERS];

	InitMidi();
	for(i=0;i<MIDI_TRACKS; i++)
	{
		numChannel[i].Init(handle,i);
	};
	for(i=0;i<MAX_TRACKS; i++)
	{
		numC[i]=0;
		notes[i]=-1;
	};
}

mi::~mi()
{
	FreeMidi();
	delete[] Vals;
}

void mi::Init()
{
// Initialize your stuff here
}


void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[256];

sprintf(

		buffer,"%s%s%s%s",
		"Commands:\n",
		"C1xx - Set Program to xx\n",
		"C2xx - Set Midi Channel for this track\n\n",
		"CCxx - Set Volume\n\n",
		"Use the Aux column to force a midi channel for this note\n"
		);

pCB->MessBox(buffer,"·-=<YanniS> YMidi Midi Output v." YMIDI_VERSION "=-·",0);

}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
	switch(par)
	{
	case 0:
		{
			///\todo: finish this.
			pars.portidx = val;
		}
	case 1: numChannel[0].SetPatch(val);break;
	case 2: numChannel[1].SetPatch(val);break;
	case 3: numChannel[2].SetPatch(val);break;
	case 4: numChannel[3].SetPatch(val);break;
	case 5: numChannel[4].SetPatch(val);break;
	case 6: numChannel[5].SetPatch(val);break;
	case 7: numChannel[6].SetPatch(val);break;
	case 8: numChannel[7].SetPatch(val);break;
	case 9: numChannel[8].SetPatch(val);break;
	case 10: numChannel[9].SetPatch(val);break;
	case 11: numChannel[10].SetPatch(val);break;
	case 12: numChannel[11].SetPatch(val);break;
	case 13: numChannel[12].SetPatch(val);break;
	case 14: numChannel[13].SetPatch(val);break;
	case 15: numChannel[14].SetPatch(val);break;
	case 16: numChannel[15].SetPatch(val);break;
	default:break;
	}
}

void mi::Stop()
{
	for (int i=0;i<MIDI_TRACKS;i++)
	{
		numChannel[i].StopMidi();
	}
}

bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param)
	{
		///\todo: finish this.
	case 0:
	default : return false;
	}
}

// Process each sequence tick if note on or note off is pressed.
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	if ( cmd == 0xC2 ) numC[channel]= val&0x0F;
	else if ( ins != 255 ) numC[channel]= ins&0x0F;
	else if (cmd == 0xC1) numChannel[numC[channel]].SetPatch(val&0x7F);

	if(note<120) // Note on
	{
		if (notes[channel]!= -1) numChannel[channel].Stop(notes[channel]);
		numChannel[numC[channel]].Play(note, (cmd == 0xCC)?val&0x7F:0x64);
		notes[channel]=note;
	}
	else if (note==120 && notes[channel]!=-1) // Note off
	{
		numChannel[channel].Stop(notes[channel]);
	}
}

void mi::InitMidi()
{
	if (!midiOutOpen(&handle, (UINT)-1, 0, 0, CALLBACK_NULL) )
	{
		midiopencount++;
	}
}

void mi::FreeMidi()
{
	midiopencount--;
	if (!midiopencount) 
	{
		midiOutClose(handle);
	}
}
