#include <project.h>
#include <psycle/plugin/y_midi/ymidi.h>

DWORD midichannel::BuildEvent2(const int eventtype, const int channel, const int p1, const int p2)
{
	// int tmp=0; tmp+=A;tmp<<=8;tmp+=B;tmp<<=8;tmp+=C;
	//((A)<<16)+((B)<<8)+((C)<<0)
	//int tmp=0;tmp+=(A1)&3;tmp<<=4;tmp+=(A2)&3;

	DWORD midievent;
	int event_cmd=eventtype;
	UCHAR u_p1 = p1&0xff;
	UCHAR u_p2 = p2&0xff;

	// build the 1 byte command from two nibbles 0x(midievent, channel)
//	event_cmd+=(eventtype)&3;
	event_cmd+=(channel)&3;
	midievent = DWORD(((u_p2)<<16)+((u_p1)<<8)+((event_cmd)<<0));
	return midievent;
}

// Method to start a note playing in the current channel
void midichannel::StartMidi()
{
	DWORD midievent;
	midievent = BuildEvent2(MIDI_NOTEON, Chan, Note, Vol);
	midiOutShortMsg(handle, midievent);
	Playnote = true;
}

// Method to call a note off for the current note in this channel.
void midichannel::StopMidi()
{
	DWORD midievent;
	midievent = BuildEvent2(MIDI_NOTEOFF, Chan, Note, Vol);
	midiOutShortMsg(handle, midievent);
	Playnote = false;
}

void mi::InitMidi()
{

	/* Open default MIDI Out device */
	if (!midiOutOpen(&handle, (UINT)-1, 0, 0, CALLBACK_NULL) )
	{
		midiopen = true;
	}
	else
	{
		midiopen = false;
	}
}

void mi::CloseMidi()
{
    if (midiopen) 
	{
		midiOutClose(handle);
	}
}

//////////////////////////////////////////////////////////////////////
DLL_EXPORTS
//////////////////////////////////////////////////////////////////////

midichannel::midichannel()
{
	Vol = 0;
	Note = 0;
	Playnote = false;
}

midichannel::~midichannel()
{
	Stop();
}

void midichannel::Init(HMIDIOUT handle_in)
{
	Vol = 0;
	Note = 0;
	Playnote = false;
	handle = handle_in;
}

int midichannel::GetVolume()
{
	return Vol;
}

void midichannel::SetPatch(const int patchnum)
{
	DWORD midievent;
	if (Patch != patchnum)
	{
		Patch = patchnum;
		midievent = BuildEvent2(MIDI_PATCHCHANGE, Chan, Patch, 0);
		midiOutShortMsg(handle, midievent);
	}
}

void midichannel::SetChannel(const int channel)
{
	Chan = channel;
}

// Call the channel to stop playing the last note.
void midichannel::Stop()
{
	if (Playnote)
	{
		StopMidi();
	}
}

void midichannel::Play(const int note, const int vol)
{
	Stop();		// Automatically stop old note playing first.
	
	// Set new note values after the old note has stopped!
	Note = note;
	Vol = vol;
	StartMidi();
}

mi::mi()
{
	int i;
	Vals=new int[NUMPARAMETERS];

	InitMidi();
	for(i=0;i<MAX_TRACKS; i++)
	{
		numChannel[i].Init(handle);
		numChannel[i].SetChannel(i);
	};

    // Output the C note (ie, sound the note)
    midiOutShortMsg(handle, 0x00403C90);
    midiOutShortMsg(handle, 0x00404090);
    midiOutShortMsg(handle, 0x00404390);

    // Here you should insert a delay so that you can hear the notes sounding 
    Sleep(500);

    // Now let's turn off those 3 notes 
    midiOutShortMsg(handle, 0x00003C90);
    midiOutShortMsg(handle, 0x00004090);
    midiOutShortMsg(handle, 0x00004390);
}

mi::~mi()
{
     /* Close the MIDI device */
	CloseMidi();
	delete Vals;
}

void mi::Init()
{
// Initialize your stuff here
//	InitMidi();
}


void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[256];

sprintf(

		buffer,"%s%s%s",
		"Supported Events\n\n",
		"Note ON\n\n",
		"Note OFFn"
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
}

void mi::Stop()
{
	int i;
	for (i=0;i<MAX_TRACKS;i++)
	{
		numChannel[i].Stop();
	}
}

void mi::MidiNote(int const channel, int const value, int const velocity)
{
	numChannel[channel].Play(value, velocity);		// start new note
}


bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param)
	{
		case 0:
		default : return false;
	}
}

// Process each sequence tick if note on or note off is pressed.
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	int Volume = val;
	if (Volume == 0)
	{
		Volume = 0xFF;
	};

	if(note<120) // Note on
	{
		switch (ins)
		{
			case 0x0A :
				numChannel[channel].SetPatch(cmd);
		};
		MidiNote(channel, note, Volume); 
	}
	else if (note==120)
	{
		numChannel[channel].Stop(); // Note off
	}
}
