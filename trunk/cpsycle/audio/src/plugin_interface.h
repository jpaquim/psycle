// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
// Psycle FX PLUGIN API

#ifndef __MACHINE_INTERFACE_H
#define __MACHINE_INTERFACE_H

#include <stdio.h>
#include <assert.h>

#define MI_VERSION				11		//Psycle 1.6.
#define SEQUENCER				1
#define EFFECT					0
#define GENERATOR				3
#define NOTE_MAX								119
#define NOTE_NO									120
#define NOTE_OFF								255

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

#define PI 3.14159265358979323846;

#define MAX_BUFFER_LENGTH		256		// in number of samples (per channel)

// CMachineParameter flags
#define MPF_NULL				0
#define MPF_STATE				2
#define MPF_SLIDER				4
#define MPF_LEVEL				8

typedef struct {
	char const *Name;		// Short name: "Cutoff"
	char const *Description;// Longer description: "Cutoff Frequency (0-7f)"
	int MinValue;			// 0
	int MaxValue;			// 127
	int Flags;
	int DefValue;			// default value for params that have MPF_STATE flag set
} CMachineParameter;

typedef struct {
	/// API version. Use MI_VERSION
	short APIVersion;
	/// plug version. Your machine version. Shown in Hexadecimal.
	short PlugVersion;
	/// Machine flags. Defines the type of machine
	int Flags;
	/// number of parameters.
	int numParameters;
	/// a pointer to an array of pointers to parameter infos
	CMachineParameter const * const * const Parameters;
	/// "Name of the machine in listing"
	char  * Name;
	/// "Name of the machine in machine Display"
	char  * ShortName;
	/// "Name of author"
	char  * Author;
	/// "Text to show as custom command (see Command method)"
	char  * Command;
	/// number of columns to display in the parameters' window
	int numCols;
} CMachineInfo;
		

#ifdef __cplusplus

/// callback functions to let plugins communicate with the host.
/// DO NOT CHANGE the order of the functions. This is an exported class!
class CFxCallback {
	public:
		virtual void MessBox(char const * /*message*/, char const * /*caption*/, unsigned int /*type*/) const = 0;
		///\todo: doc
		virtual int CallbackFunc(int /*cbkID*/, int /*par1*/, int /*par2*/, void* /*par3*/) = 0;

		/// unused vtable slot kept for binary compatibility with old closed-source plugins
		virtual float * unused0(int, int) = 0;
		/// unused vtable slot kept for binary compatibility with old closed-source plugins
		virtual float * unused1(int, int) = 0;

		virtual int GetTickLength() const = 0;
		virtual int GetSamplingRate() const = 0;
		virtual int GetBPM() const = 0;
		virtual int GetTPB() const = 0;
		/// do not move this destructor from here. Since this is an interface, the position matters.
		virtual ~CFxCallback() throw() {}
		/// Open a load (openMode=true) or save (openMode=false) dialog.
		/// filter is in MFC format: description|*.ext|description2|*ext2|| 
		/// if you indicate a directory in inoutName, it will be used. Else, you need to provide
		/// an empty string ([0]='\0') and the plugin dir will be used instead.
		/// returns true if the user pressed open/save, else return false.
		/// filter can be any size you want. inoutname has to be 1024 chars.
		virtual bool FileBox(bool openMode, char filter[], char inoutName[]) = 0;
};
#endif

/*////////////////////////////////////////////////////////////////////////*/

/// base machine class from which plugins derived.
/// Note: We keep empty definitions of the functions in the header so that
/// plugins don't need to implement everything nor link with a default implementation.
#ifdef __cplusplus
class CMachineInterface {
	public:
		virtual ~CMachineInterface() {}
		/// Initialization method called by the Host at initialization time.
		/// pCB callback pointer is available here.
		virtual void Init() {}
		/// Called by the Host each sequence tick (in psyclemfc, means each line).
		/// It is called even when the playback is stopped, so that the plugin can synchronize correctly.
		virtual void SequencerTick() {}
		/// Called by the host when the user changes a paramter from the UI or a tweak from the pattern.
		/// It is also called just after calling Init, to set each parameter to its default value, so you
		/// don't need to explicitely do so.
		virtual void ParameterTweak(int /*par*/, int /*val*/) {}

		/// Called by the host when it needs audio data. the pointers are input-output pointers
		/// (read the data in case of effects, and write the new data over). 
		/// numsamples is the amount of samples (per channel) to generate and tracks is mostly unused. It carries
		/// the current number of tracks of the song.
		virtual void Work(float * /*psamplesleft*/, float * /*psamplesright*/, int /*numsamples*/, int /*tracks*/) {}

		/// Called by the host when the user presses the stop button.
		virtual void Stop() {}

		///\name Export / Import
		///\{
			/// Called by the host when loading a song or preset.
			/// The pointer contains the data saved by the plugin with GetData().
			/// It is called after all parameters have been set with ParameterTweak.
			virtual void PutData(void * /*data*/) {}
			/// Called by the host when saving a song or preset. Use it to to save extra data that you need.
			/// The values of the parameters will be automatically restored via calls to parameterTweak().
			virtual void GetData(void * /*data*/) {}
			/// Called by the host before calling GetData to know the size to allocate for pData before calling GetData()
			virtual int GetDataSize() { return 0; }
		///\}

		/// Called by the host when the user selects the command menu option. Commonly used to show a help box,
		/// but can be used to show a specific editor,a configuration or other similar things.
		virtual void Command() {}

		/// unused vtable slot kept for binary compatibility with old closed-source plugins
		virtual void unused0(int /*track*/) {}
		/// unused vtable slot kept for binary compatibility with old closed-source plugins
		virtual bool unused1(int /*track*/) const { return false; }

		///called by the host to send a midi event (mcm) to the plugin.
		virtual void MidiEvent(int /*channel*/, int /*midievent*/, int /*value*/) {}

		/// unused vtable slot kept for binary compatibility with old closed-source plugins
		virtual void unused2(unsigned int const /*data*/) {}

		/// Called by the host when it requires to show a description of the value of a parameter.
		/// return false to tell the host to show the numerical value. Return true and fill txt with
		/// some text to show that text to the user.
		virtual bool DescribeValue(char * /*txt*/, int const /*param*/, int const /*value*/) { return false; }
		///called by the host to send a control event or ask for information. See HostEvent codes.
		virtual bool HostEvent(int const /*eventNr*/, int const /*val1*/, float const /*val2*/) { return false; }
		/// Called by the host when there is some data to play. Only notes and pattern commands will be informed
		/// this way. Tweaks call ParameterTweak
		virtual void SeqTick(int /*channel*/, int /*note*/, int /*ins*/, int /*cmd*/, int /*val*/) {}

		/// unused vtable slot kept for binary compatibility with old closed-source plugins
		virtual void unused3() {}

	public:
		/// initialize this member in the constructor with the size of parameters.
		int * Vals;

		/// callback.
		/// This member is initialized by the engine right after it calls CreateMachine().
		/// Don't touch it in the constructor.
		CFxCallback mutable * pCB;
};
#else
	typedef struct CMachineInterface CMachineInterface;
#endif


#define DLL_EXPORTS extern "C" { \
__declspec(dllexport) CMachineInfo const * __cdecl GetInfo() \
{ \
	return &MacInfo; \
} \
__declspec(dllexport) CMachineInterface * __cdecl CreateMachine() \
{ \
	return new mi; \
} \
}


#ifdef __cplusplus
extern "C" {
#endif

#if defined __STDC__ || __cplusplus
extern void mi_resetcallback(CMachineInterface* mi);
extern void mi_setcallback(CMachineInterface* mi, const struct MachineCallback* callback);
extern CMachineInterface* mi_create(void* module);
extern void mi_init(CMachineInterface* mi);
extern void mi_dispose(CMachineInterface* mi);
extern void mi_sequencertick(CMachineInterface* mi);		
extern void mi_parametertweak(CMachineInterface* mi, int par, int val);
extern void mi_work(CMachineInterface* mi, float * psamplesleft, float * psamplesright, int numsamples, int tracks);
extern void mi_stop(CMachineInterface* mi);
extern void mi_putdata(CMachineInterface* mi, void * data);
extern void mi_getdata(CMachineInterface* mi, void * data);
extern int  mi_getdatasize(CMachineInterface* mi);
extern void mi_command(CMachineInterface* mi);
extern void mi_unused0(CMachineInterface* mi, int track);
extern int  mi_unused1(CMachineInterface* mi, int track);
extern void mi_midievent(CMachineInterface* mi, int channel, int midievent, int value);
extern void mi_unused2(CMachineInterface* mi, unsigned int const data);
extern int  mi_describevalue(CMachineInterface* mi, char* txt, int const param, int const value);
extern int  mi_hostevent(CMachineInterface* mi, int const eventNr, int const val1, float const val2);
extern void mi_seqtick(CMachineInterface* mi, int channel, int note, int ins, int cmd, int val);
extern void mi_unused3(CMachineInterface* mi);
extern int mi_val(CMachineInterface* mi, int param);
extern void mi_setval(CMachineInterface* mi, int param, int val);
#else
extern void mi_resetcallback();
extern void mi_setcallback(); 
extern CMachineInterface* mi_create();
extern void mi_init();
extern void mi_dispose();
extern void mi_sequencertick();		
extern void mi_parametertweak();
extern void mi_work();
extern void mi_stop();
extern void mi_putdata();
extern void mi_getdata();
extern int mi_getdatasize();
extern void mi_command();
extern void mi_unused0();
extern int mi_unused1();
extern void mi_midievent();
extern void mi_unused2();
extern int mi_describevalue();
extern int mi_hostevent();
extern void mi_seqtick();
extern void mi_unused3();
extern int mi_val();
extern void mi_setval();
#endif

#ifdef __cplusplus
}
#endif

#endif
