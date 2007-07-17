///\file
///\brief the original machine interface api
///\see plugin.hpp a more object-oriented api
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

		#if !defined(MAX_TRACKS)
		/// max number of pattern tracks
		int const MAX_TRACKS = 64;
		#endif

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

		double const pi = 
			#if defined M_PI // on some systems, #include <cmath> defines M_PI but this is not standard
				M_PI
			#else
				3.14159265358979323846
			#endif
		;

		//////////////////////////////////////////////////////////////////////////

		/// class to define the modificable parameters of the machine
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

		///\name CMachineParameter flags
		///\{
			/// shows a line with background
			int const MPF_NULL = 0;
			/// shows a line with the text in a centered label
			int const MPF_LABEL = 1;
			/// shows a tweakable knob and text
			int const MPF_STATE = 2;
		///\}

		//////////////////////////////////////////////////////////////////////////

		/// class defining the machine properties
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

		///\name CMachineInfo flags
		///\{
			int const EFFECT = 0;
			///\todo: unused
			int const SEQUENCER = 1;
			int const GENERATOR = 3;
			int const CUSTOM_GUI = 16;
		///\}

		//////////////////////////////////////////////////////////////////////////

		/// callback functions to let plugins communicate with the host.
		class CFxCallback
		{
		public:
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
			virtual inline ~CFxCallback() throw() {}
		};

		//////////////////////////////////////////////////////////////////////////
		
		/// base machine class
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

			/// callback.
			/// This member is initialized by the engine right after it calls CreateMachine().
			/// Don't touch it in the constructor.
			CFxCallback * pCB;
		};

		//////////////////////////////////////////////////////////////////////////

		// spelling INSTANCIATOR -> INSTANTIATOR
#define PSYCLE__PLUGIN__INSTANCIATOR(typename, info) \
	extern "C" \
	{ \
		__declspec(dllexport) ::CMachineInfo const * const cdecl GetInfo() { return &info; } \
		__declspec(dllexport) ::CMachineInterface * cdecl CreateMachine() { return new typename; } \
		__declspec(dllexport) void cdecl DeleteMachine(::CMachineInterface & plugin) { delete &plugin; } \
	}
	}
}

// for plugins that aren't namespace-aware
using psycle::plugin_interface::MI_VERSION;
#if !defined(MAX_TRACKS)
using psycle::plugin_interface::MAX_TRACKS;
#endif
using psycle::plugin_interface::NOTE_MAX;
using psycle::plugin_interface::NOTE_NO;
using psycle::plugin_interface::NOTE_OFF;
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
using psycle::plugin_interface::uint8; // deprecated anyway
using psycle::plugin_interface::uint16; // deprecated anyway
using psycle::plugin_interface::uint32; // deprecated anyway

#include <cstdio> // would be better if plugins that want it included it themselves.
