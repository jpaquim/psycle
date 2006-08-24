#error error on purpose to test the build bot ... again
///\file
///\brief the original machine interface api
///\see plugin.hpp another, more object-oriented, interface api
#pragma once

namespace psycle
{
	namespace plugin_interface
	{
		///\todo use #include <cstdint> for that!
		typedef /* std::uint8_t  */ unsigned char      uint8;
		typedef /* std::uint16_t */ unsigned short int uint16;
		typedef /* std::uint32_t */ unsigned long int  uint32;

		/// machine interface version
		int const MI_VERSION = 11;

		/// Max number of pattern tracks
		int const MAX_TRACKS = 64;

		/// The max number of samples (per channel) that the Work function will ask to return.
		int const MAX_BUFFER_LENGTH = 256;

		///\name note values
		///\{
		int const NOTE_MAX = 119;	// value of B-9. NOTE: C-0 is note number 0!
		int const NOTE_NO = 120;	// value of the "off" note.
		int const NOTE_OFF = 255;	// empty value.
		///\}

		#if !defined PI
			double const PI = 
				#if defined M_PI
					M_PI
				#else
					3.14159265358979323846
				#endif
			;
		#endif


//////////////////////////////////////////////////////////////////////////
// Class to define the modificable parameters of the machine.

		///\name CMachineParameter flags.
		///\{
		int const MPF_NULL = 0;		// Shows a line with background
		int const MPF_LABEL = 1;	// Shows a line with the text in a centered label.
		int const MPF_STATE = 2;	// Shows a tweakable knob and text.
		///\}

		class CMachineParameter
		{
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

//////////////////////////////////////////////////////////////////////////
//	Class defining the machine properties.

		///\name CMachineInfo flags.
		///\{
			int const EFFECT = 0;
			int const SEQUENCER = 1;	//\todo: unused.
			int const GENERATOR = 3;
			int const CUSTOM_GUI = 16;
		///\}

		class CMachineInfo
		{
		public:
			int Version;
			int Flags;
			int numParameters;
			CMachineParameter const **Parameters;
			/// "Rambo Delay"
			char const *Name;
			/// "Delay"
			char const *ShortName;
			/// "John Rambo"
			char const *Author;
			/// "About"
			char const *Command;
			/// number of columns
			int numCols;
		};

//////////////////////////////////////////////////////////////////////////
//	Callback function to let plugins communicate with the host.

		class CFxCallback
		{
		public:
			virtual inline ~CFxCallback() throw() {}
			virtual void MessBox(char* ptxt,char*caption,unsigned int type){}
			virtual int CallbackFunc(int cbkID,int par1,int par2,int par3){return 0;}
			/// unused slot kept for binary compatibility.
			virtual float * unused0(int, int){return 0;}
			/// unused slot kept for binary compatibility.
			virtual float * unused1(int, int){return 0;}
			virtual int GetTickLength(){return 2048;}
			virtual int GetSamplingRate(){return 44100;}
			virtual int GetBPM(){return 125;}
			virtual int GetTPB(){return 4;}
			// Don't get fooled by the above return values.
			// You get a pointer to a subclass of this one that returns the correct ones.
		};

//////////////////////////////////////////////////////////////////////////
//	Base Machine structure class.
		class CMachineInterface
		{
		public:
			virtual inline ~CMachineInterface() {}
			virtual void Init() {}
			virtual void SequencerTick() {}
			virtual void ParameterTweak(int par, int val) {}

			/// Work function
			virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {}

			virtual void Stop() {}

			///\name Export / Import
			///\{
				virtual void PutData(void * pData) {}
				virtual void GetData(void * pData) {}
				virtual int GetDataSize() { return 0; }
			///\}

			virtual void Command() {}

			virtual void MuteTrack(int const i) {} /// Not used (yet?)
			virtual bool IsTrackMuted(int const i) const { return false; } 	// Not used (yet?)

			virtual void MidiNote(int const channel, int const value, int const velocity) {} /// Not used (yet?)
			virtual void Event(uint32 const data) {} /// Not used (yet?)

			virtual bool DescribeValue(char* txt,int const param, int const value) { return false; }

			virtual bool PlayWave(int const wave, int const note, float const volume) { return false; } /// Not used (prolly never)
			virtual void SeqTick(int channel, int note, int ins, int cmd, int val) {}

			virtual void StopWave() {} 	/// Not used (prolly never)

		public:
			/// initialize these members in the constructor
			int *Vals;

			/// Callback.
			/// this member is initialized by the
			/// engine right after it calls CreateMachine()
			/// don't touch it in the constructor
			CFxCallback * pCB;
		};

		#define PSYCLE__PLUGIN__INSTANCIATOR(typename, info) \
			extern "C" \
			{ \
				PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT ::CMachineInfo const * const PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION GetInfo() { return &info; } \
				PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT ::CMachineInterface *        PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION CreateMachine() { return new typename; } \
				PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT void                         PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION DeleteMachine(::CMachineInterface & plugin) { delete &plugin; } \
			}
		#if !defined _WIN32 && !defined _WIN64
			#define PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT
			#define PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION
		#elif defined _MSC_VER
			#define PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT __declspec(dllexport)
			#define PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION __cdecl
		#else
			#error please add definition for your compiler
		#endif
	}
}

// for plugins that aren't namespace-aware
using psycle::plugin_interface::MI_VERSION;
using psycle::plugin_interface::MAX_TRACKS;
using psycle::plugin_interface::NOTE_MAX;
using psycle::plugin_interface::NOTE_NO;
using psycle::plugin_interface::NOTE_OFF;
#if !defined PI
	using psycle::plugin_interface::PI;
#endif
using psycle::plugin_interface::MAX_BUFFER_LENGTH;
using psycle::plugin_interface::CMachineInfo;
using psycle::plugin_interface::GENERATOR;
using psycle::plugin_interface::EFFECT;
using psycle::plugin_interface::SEQUENCER;
using psycle::plugin_interface::CMachineInterface;
using psycle::plugin_interface::CMachineParameter;
using psycle::plugin_interface::MPF_LABEL;
using psycle::plugin_interface::MPF_STATE;
using psycle::plugin_interface::CFxCallback;
using psycle::plugin_interface::uint8;
using psycle::plugin_interface::uint16;
using psycle::plugin_interface::uint32;

#include <cstdio> // would be better if plugins that want it, included it themselves.
