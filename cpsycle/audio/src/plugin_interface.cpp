// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
#include "plugin_interface.h"

#include "machine.h"
#include "library.h"

#include <string.h>

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

class PluginFxCallback : public CFxCallback
{

	inline virtual void MessBox(char const* ptxt,char const* caption,unsigned int type) const 
	{ 		
	}
	inline virtual int GetTickLength() const { 
		return 256;
	}
	inline virtual int GetSamplingRate() const { 
		return callback.samplerate && callback.context
			? callback.samplerate(callback.context) 
			: 44100;
	}
	inline virtual int GetBPM() const { 
		return callback.bpm && callback.context
			? callback.bpm(callback.context) 
			: 125;		
	}
	inline virtual int GetTPB() const { return 4; }
	virtual int CallbackFunc(int /*cbkID*/, int /*par1*/, int /*par2*/, void* /*par3*/)
	{
		return 0;
	}
	virtual bool FileBox(bool openMode, char filter[], char inoutName[])
	{
		return 0;
	}
	/// unused slot kept for binary compatibility for (old) closed-source plugins on msvc++ on mswindows.
	inline virtual float * unused0(int, int) { return NULL;}
	/// unused slot kept for binary compatibility for (old) closed-source plugins on msvc++ on mswindows.
	inline virtual float * unused1(int, int) { return NULL;}

	public: ///\todo private:
		psy_audio_MachineCallback callback;
};

void mi_resetcallback(CMachineInterface* mi)
{
	mi->pCB = 0;
}

void mi_setcallback(CMachineInterface* mi, const struct psy_audio_MachineCallback* callback)
{
	PluginFxCallback* pCB;

	if (mi->pCB == 0) {
		mi->pCB = new PluginFxCallback;
	}	
	pCB = dynamic_cast<PluginFxCallback*>(mi->pCB);
	if (pCB) {
		if (callback) {								
			pCB->callback = *callback;
		} else {
			memset(&mi->pCB, 0, sizeof(psy_audio_MachineCallback));
		}	
	}
}

void mi_init(CMachineInterface* mi)
{
	mi->Init();	
}

void mi_dispose(CMachineInterface* mi)
{
	delete mi->pCB;
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
	psy_Library library;
	
	library.module = module;
	library.err = 0;
	
	mi = 0;
	GetInterface = (CREATEMACHINE) psy_library_functionpointer(&library, "CreateMachine");
	if (GetInterface != NULL)
	{			
		mi = GetInterface();			
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
