///\interface psycle native plugin interface api

#ifndef PSYCLE__PLUGIN_INTERFACE__INCLUDED
#define PSYCLE__PLUGIN_INTERFACE__INCLUDED
#pragma once

// *** Note ***
// Because this file may be used outside of the psycle project itself,
// we should not introduce any dependency by including
// anything that is not part of the c++ standard library.

namespace psycle { namespace plugin_interface {

/// machine interface version
int const MI_VERSION = 11;

/// max number of pattern tracks
int const MAX_TRACKS = 64;

/// max number of samples (per channel) that the Work function may ask to return
int const MAX_BUFFER_LENGTH = 256;

///\name note values
///\{
	/// value of B-9. NOTE: C-0 is note number 0!
	int const NOTE_MAX = 119;
	/// value of the "off" note
	int const NOTE_NO = 120;
	/// empty value
	int const NOTE_OFF = 255;
///\}

/// the pi constant.
/// note: this is also defined in <psycle/helpers/math/pi.hpp> but we want no dependency here
double const pi = 3.14159265358979323846;

/*////////////////////////////////////////////////////////////////////////*/

/// class to define the modificable parameters of the machine
class CMachineParameter {
	public:
		/// Short name: "Cutoff"
		char const *Name;
		/// Longer description: "Cutoff Frequency (0-7f)"
		char const *Description;
		/// >= 0
		int MinValue;
		/// <= 65535
		int MaxValue;
		/// flags.
		int Flags;
		/// default value for params that have MPF_STATE flag set
		int DefValue;
};

///\name CMachineParameter flags
///\{
	/// shows a line with background
	int const MPF_NULL = 0;
	/// shows a line with the text in a centered label
	int const MPF_LABEL = 1;
	/// shows a tweakable knob and text
	int const MPF_STATE = 2;
///\}

/*////////////////////////////////////////////////////////////////////////*/

/// class defining the machine properties
class CMachineInfo {
	public:
		CMachineInfo(int version, int flags, int numParameters, CMachineParameter const * const * parameters,
			char const * name, char const * shortName, char const * author, char const * command, int numCols)
		: Version(version), Flags(flags), numParameters(numParameters), Parameters(parameters),
		Name(name), ShortName(shortName), Author(author), Command(command), numCols(numCols) {}
		/// API version. Use MI_VERSION
		int const Version;
		/// Machine flags. Defines the type of machine
		int const Flags;
		/// number of parameters.
		int const numParameters;
		/// a pointer to an array of pointers to parameter infos
		CMachineParameter const * const * const Parameters;
		/// "Name of the machine in listing"
		char const * const Name;
		/// "Name of the machine in machine Display"
		char const * const ShortName;
		/// "Name of author"
		char const * const Author;
		/// "Text to show as custom command (see Command method)"
		char const * const Command;
		/// number of columns to display in the parameters' window
		int numCols;
};

///\name CMachineInfo flags
///\{
	/// Machine is an effect (can receive audio)
	int const EFFECT = 0;
	///\todo: unused
	int const SEQUENCER = 1;
	/// Machine is a generator (does not receive audio)
	int const GENERATOR = 3;
	///\todo: unused
	int const CUSTOM_GUI = 16;
///\}

/*////////////////////////////////////////////////////////////////////////*/

/// callback functions to let plugins communicate with the host.
/// DO NOT CHANGE the order of the functions. This is an exported class!
class CFxCallback {
	public:
		virtual void MessBox(char const * /*message*/, char const * /*caption*/, unsigned int /*type*/) const {}
		virtual int CallbackFunc(int /*cbkID*/, int /*par1*/, int /*par2*/, int /*par3*/) const { return 0; }
		/// unused slot kept for binary compatibility for (old) closed-source plugins on msvc++ on mswindows.
		virtual float * unused0(int, int) const { return 0; }
		/// unused slot kept for binary compatibility for (old) closed-source plugins on msvc++ on mswindows.
		virtual float * unused1(int, int) const { return 0; }
		virtual int GetTickLength() const { return 2048; }
		virtual int GetSamplingRate() const { return 44100; }
		virtual int GetBPM() const { return 125; }
		virtual int GetTPB() const { return 4; }
		// Don't get fooled by the above return values.
		// You get a pointer to a subclass of this one that returns the correct ones.
		virtual ~CFxCallback() throw() {}
};

/*////////////////////////////////////////////////////////////////////////*/

/// base machine class from which plugins derived.
/// Note: We keep empty definitions of the functions in the header so that
/// plugins don't need to implement everything nor link with a default implementation.
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
			/// Called by the host when loading a song or preset. The pointer contains the data saved
		/// by the plugin with GetData()
			/// It is called after all parameters have been set with ParameterTweak.
			virtual void PutData(void * /*pData*/) {}
			/// Called by the host when saving a song or preset. Use it to to save extra data that you need
			/// The values of the parameters will be automatically restored via calls to parameterTweak().
			virtual void GetData(void * /*pData*/) {}
			/// Called by the host before calling GetData to know the size to allocate for pData before calling
			/// GetData()
			virtual int GetDataSize() { return 0; }
		///\}

		/// Called by the host when the user selects the command menu option. Commonly used to show a help box,
		/// but can be used to show a specific editor,a configuration or other similar things.
		virtual void Command() {}
		///\todo doc. not used (yet?)
		virtual void MuteTrack(int /*track*/) {}
		///\todo doc. not used (yet?)
		virtual bool IsTrackMuted(int /*track*/) const { return false; }
		///\todo doc. not used (yet?)
		virtual void MidiNote(int /*channel*/, int /*value*/, int /*velocity*/) {}
		///\todo doc. not used (yet?)
		virtual void Event(unsigned int const /*data*/) {}
		/// Called by the host when it requires to show a description of the value of a parameter.
		/// return false to tell the host to show the numerical value. Return true and fill txt with
		/// some text to show that text to the user.
		virtual bool DescribeValue(char * /*txt*/, const int /*param*/, const int /*value*/) { return false; }
		///\todo doc. not used (prolly never)
		virtual bool PlayWave(int /*wave*/, int /*note*/, float /*volume*/) { return false; }
		/// Called by the host when there is some data to play. Only notes and pattern commands will be informed
		/// this way. Tweaks call ParameterTweak
		virtual void SeqTick(int /*channel*/, int /*note*/, int /*ins*/, int /*cmd*/, int /*val*/) {}
		///\todo doc. not used (prolly never)
		virtual void StopWave() {}

	public:
		/// initialize this member in the constructor with the size of parameters.
		int * Vals;

		/// callback.
		/// This member is initialized by the engine right after it calls CreateMachine().
		/// Don't touch it in the constructor.
		CFxCallback mutable * pCB;
};

/*////////////////////////////////////////////////////////////////////////*/
/// From the text below, you just need to know that once you've defined the MachineInteface class
/// and the MachineInfo instance, USE PSYCLE__PLUGIN__INSTANTIATOR() to export it.

#define PSYCLE__PLUGIN__INSTANTIATOR(typename, info) \
	extern "C" \
	{ \
		PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT \
		psycle::plugin_interface::CMachineInfo const * const \
		PSYCLE__PLUGIN__CALLING_CONVENTION \
		GetInfo() { return &info; } \
		\
		PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT \
		psycle::plugin_interface::CMachineInterface * \
		PSYCLE__PLUGIN__CALLING_CONVENTION \
		CreateMachine() { return new typename; } \
		\
		PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT \
		void \
		PSYCLE__PLUGIN__CALLING_CONVENTION \
		DeleteMachine(psycle::plugin_interface::CMachineInterface & plugin) { delete &plugin; } \
	}

/// we don't use universalis/diversalis here because we want no dependency
#if !defined _WIN32 && !defined __CYGWIN__ && !defined __MSYS__ && !defined _UWIN
	#define PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT
	#define PSYCLE__PLUGIN__CALLING_CONVENTION
#elif defined __GNUG__
	#define PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT __attribute__((dllexport))
	#define PSYCLE__PLUGIN__CALLING_CONVENTION __attribute__((__cdecl__))
#elif defined _MSC_VER
	#define PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT __declspec(dllexport)
	#define PSYCLE__PLUGIN__CALLING_CONVENTION __cdecl
#else
	#error please add definition for your compiler
#endif
	
}}
#endif

