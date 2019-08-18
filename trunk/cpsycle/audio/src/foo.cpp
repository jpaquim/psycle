#include "foo.h"
#include <windows.h>


// typedef CMachineInfo * (*GETINFO)(void);
// typedef CMachineInterface * (*CREATEMACHINE)(void);


static PluginFxCallback _callback;

PluginFxCallback Foo::_callback;

Foo::Foo(CMachineInterface* mi) : _interface(mi), a_(0) {
	mi->pCB = &_callback;

}



void Foo::foo(int a) { a_ = a; }

Foo* cplusplus_callback_function(Foo* foo)
{
/*	HINSTANCE _dll;
	CMachineInfo* pInfo;

	_dll = LoadLibrary("Arguru Synth 2f.dll");
	GETINFO GetInfo =(GETINFO)GetProcAddress(_dll, "GetInfo");


	if (GetInfo == NULL)
	{
		FreeLibrary(_dll);
		_dll=NULL;
		return false;
	}

	pInfo = GetInfo();*/

	return foo;
}

Foo* foo_create(CMachineInterface* _interface)
{
	return new Foo(_interface);
}

void foo_delete(Foo* foo)
{
	delete foo;	
}

void foo_setcallback(CMachineInterface* mi)
{
	mi->pCB = &_callback;
}


void mi_init(CMachineInterface* mi) {

}
		/// Called by the Host each sequence tick (in psyclemfc, means each line).
		/// It is called even when the playback is stopped, so that the plugin can synchronize correctly.
void mi_sequencertick(CMachineInterface* mi) {
	mi->SequencerTick();
}		
		/// Called by the host when the user changes a paramter from the UI or a tweak from the pattern.
		/// It is also called just after calling Init, to set each parameter to its default value, so you
		/// don't need to explicitely do so.
void mi_parametertweak(CMachineInterface* mi, int par, int val) {
	mi->ParameterTweak(par, val);
}		

		/// Called by the host when it needs audio data. the pointers are input-output pointers
		/// (read the data in case of effects, and write the new data over). 
		/// numsamples is the amount of samples (per channel) to generate and tracks is mostly unused. It carries
		/// the current number of tracks of the song.
void mi_work(CMachineInterface* mi, float * psamplesleft, float * psamplesright, int numsamples, int tracks) {
	mi->Work(psamplesleft, psamplesright, numsamples, tracks);
}

		/// Called by the host when the user presses the stop button.
void mi_stop(CMachineInterface* mi)
{
	mi->Stop();
}

		///\name Export / Import
		///\{
			/// Called by the host when loading a song or preset.
			/// The pointer contains the data saved by the plugin with GetData().
			/// It is called after all parameters have been set with ParameterTweak.
void mi_putdata(CMachineInterface* mi, void * data) {
   mi->PutData(data);
}
			
			/// Called by the host when saving a song or preset. Use it to to save extra data that you need.
			/// The values of the parameters will be automatically restored via calls to parameterTweak().
void mi_getdata(CMachineInterface* mi, void * data) {
	mi->GetData(data);
}
			/// Called by the host before calling GetData to know the size to allocate for pData before calling GetData()
int mi_getdatasize(CMachineInterface* mi)
{
	return mi->GetDataSize();
}

		/// Called by the host when the user selects the command menu option. Commonly used to show a help box,
		/// but can be used to show a specific editor,a configuration or other similar things.
void mi_command(CMachineInterface* mi)
{
	mi->Command();
}

void mi_unused0(CMachineInterface* mi, int track)
{
	mi->unused0(track);
}

bool mi_unused1(CMachineInterface* mi, int track)
{
	return mi->unused1(track);
}

void mi_midievent(CMachineInterface* mi, int channel, int midievent, int value)
{
	mi->MidiEvent(channel, midievent, value);
}

		/// unused vtable slot kept for binary compatibility with old closed-source plugins
void mi_unused2(CMachineInterface* mi, unsigned int const data)
{
	mi->unused2(data);
}

		/// Called by the host when it requires to show a description of the value of a parameter.
		/// return false to tell the host to show the numerical value. Return true and fill txt with
		/// some text to show that text to the user.
bool mi_describeValue(CMachineInterface* mi, char* txt, int const param, int const value)
{
	return mi->DescribeValue(txt, param, value); 
}
		///called by the host to send a control event or ask for information. See HostEvent codes.
bool mi_hostevent(CMachineInterface* mi, int const eventNr, int const val1, float const val2)
{
	return mi->HostEvent(eventNr, val1, val2);
}

/// Called by the host when there is some data to play. Only notes and pattern commands will be informed
/// this way. Tweaks call ParameterTweak
void mi_seqtick(CMachineInterface* mi, int channel, int note, int ins, int cmd, int val)
{
	mi->SeqTick(channel, note, ins, cmd, val);
}

/// unused vtable slot kept for binary compatibility with old closed-source plugins
void mi_unused3(CMachineInterface* mi)
{
	mi->unused3();
}

