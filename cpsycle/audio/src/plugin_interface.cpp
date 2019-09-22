// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "plugin_interface.h"
#include <windows.h>

//#include "plugin_interface.h"
// #include <windows.h>

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);


class PluginFxCallback : public CFxCallback
{
public:
	HWND hWnd;

	virtual void MessBox(char* ptxt,char *caption,unsigned int type)
	{
		MessageBox(hWnd,ptxt,caption,type);
	}
	virtual int GetTickLength(void)
	{
		return 1024; //Global::_pSong->SamplesPerTick;
	}
	virtual int GetSamplingRate(void)
	{
#if defined(_WINAMP_PLUGIN_)
		return 44100; //Global::pConfig->_samplesPerSec;
#else		
		return 44100; //Global::pConfig->_pOutputDriver->_samplesPerSec;
#endif // _WINAMP_PLUGIN_
	}
	virtual int GetBPM(void)
	{
		return 125; //Global::pPlayer->bpm;
	}
	virtual int GetTPB(void)
	{
		return 4; //Global::pPlayer->tpb;
	}
};


static PluginFxCallback _callback;


void mi_setcallback(CMachineInterface* mi)
{
	mi->pCB = &_callback;
}

void mi_init(CMachineInterface* mi)
{
	mi->Init();
}

void mi_sequencertick(CMachineInterface* mi)
{
	mi->SequencerTick();
}		

void mi_parametertweak(CMachineInterface* mi, int par, int val)
{
	mi->ParameterTweak(par, val);
}		

void mi_work(CMachineInterface* mi, float * psamplesleft, float * psamplesright, int numsamples, int tracks)
{
	mi->Work(psamplesleft, psamplesright, numsamples, tracks);
}

void mi_stop(CMachineInterface* mi)
{
	mi->Stop();
}

void mi_putdata(CMachineInterface* mi, void * data)
{
   mi->PutData(data);
}

void mi_getdata(CMachineInterface* mi, void * data)
{
	mi->GetData(data);
}

int mi_getdatasize(CMachineInterface* mi)
{
	return mi->GetDataSize();
}

void mi_command(CMachineInterface* mi)
{
	mi->Command();
}

void mi_unused0(CMachineInterface* mi, int track)
{
	mi->unused0(track);
}

int mi_unused1(CMachineInterface* mi, int track)
{
	return mi->unused1(track);
}

void mi_midievent(CMachineInterface* mi, int channel, int midievent, int value)
{
	mi->MidiEvent(channel, midievent, value);
}

void mi_unused2(CMachineInterface* mi, unsigned int const data)
{
	mi->unused2(data);
}
		
int mi_describevalue(CMachineInterface* mi, char* txt, int const param, int const value)
{
	return mi->DescribeValue(txt, param, value); 
}

int mi_hostevent(CMachineInterface* mi, int const eventNr, int const val1, float const val2)
{
	return mi->HostEvent(eventNr, val1, val2);
}

void mi_seqtick(CMachineInterface* mi, int channel, int note, int ins, int cmd, int val)
{
	mi->SeqTick(channel, note, ins, cmd, val);
}

void mi_unused3(CMachineInterface* mi)
{
	mi->unused3();
}

CMachineInterface* mi_create(void* module)
{
	CMachineInterface* mi;
	CREATEMACHINE GetInterface;
	HINSTANCE hmodule;

	hmodule = (HINSTANCE) module;

	mi = 0;
	GetInterface =(CREATEMACHINE)GetProcAddress(hmodule, "CreateMachine");
	if (GetInterface != NULL)
	{			
		mi = GetInterface();	
		mi->pCB = &_callback; //mi_setcallback(mi);			
	}
	return mi;
}

int mi_val(CMachineInterface* mi, int param)
{
	return mi->Vals[param];
}

void mi_setval(CMachineInterface* mi, int param, int val)
{
	mi->Vals[param] = val;
}
