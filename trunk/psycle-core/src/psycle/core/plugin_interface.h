




///////// TODO This copy of the interface is not up-to-date
///////// TODO Should use the file from psycle-plugins/src/psycle/plugin_interface.hpp








///\file
///\brief the original machine interface api
///\see plugin.hpp another, more object-oriented, interface api
#pragma once
#include <cstdio> // would be better if plugins that wants it included it themselves.

//namespace psy
//{
	int const MI_VERSION = 11;
	int const SEQUENCER = 1;
	int const EFFECT = 0;
	int const GENERATOR = 3;
	int const NOTE_MAX = 119;
	int const NOTE_NO = 120;
	int const NOTE_OFF = 255;

	#if !defined MAX_TRACKS
		#define MAX_TRACKS 64
	#endif

	// <Sartorius> Druttis's plugins
	#ifndef PI
		double const PI = 3.14159265358979323846;
	#endif

	/// in number of samples (per channel).
	int const MAX_BUFFER_LENGTH = 256;

	/// CMachineParameter flags.
	int const MPF_LABEL = 0;

	/// CMachineParameter flags.
	int const MPF_STATE = 2;

	///\todo use #include <cstdint> for that!
	typedef /* std::uint8_t  */ unsigned char      uint8;
	typedef /* std::uint16_t */ unsigned short int uint16;
	typedef /* std::uint32_t */ unsigned long int  uint32;
//}

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

class CFxCallback
{
public:
	virtual void MessBox(char * /*ptxt*/, char * /*caption*/, unsigned int /*type*/) {}
	virtual int CallbackFunc(int /*cbkID*/, int /*par1*/, int /*par2*/, int /*par3*/){ return 0; }
	virtual float *GetWaveLData(int /*inst*/, int /*wave*/){ return 0; } ///\todo USELESS if you cannot get the length!
	virtual float *GetWaveRData(int /*inst*/, int /*wave*/){ return 0; } ///\todo USELESS if you cannot get the length!
	virtual int GetTickLength(){ return 2048; }
	virtual int GetSamplingRate(){ return 44100; }
	virtual int GetBPM(){ return 125; }
	virtual int GetTPB(){ return 4; }
	// Don't get fooled by the above return values.
	// You get a pointer to a subclass of this one that returns the correct ones.
	virtual inline ~CFxCallback() throw() {}
};

class CMachineInterface
{
public:
	virtual ~CMachineInterface() {}
	virtual void Init() {}
	virtual void SequencerTick() {}
	virtual void ParameterTweak(int /*par*/, int /*val*/) {}

	/// Work function
	virtual void Work(float * /*psamplesleft*/, float * /*psamplesright*/, int /*numsamples*/, int /*tracks*/) {}

	virtual void Stop() {}

	///\name Export / Import
	///\{
		virtual void PutData(void * /*pData*/) {}
		virtual void GetData(void * /*pData*/) {}
		virtual int GetDataSize() { return 0; }
	///\}

	virtual void Command() {}

	/// Not used (yet?)
	virtual void MuteTrack(int const /*track*/) {}
	/// Not used (yet?)
	virtual bool IsTrackMuted(int const /*track*/) const { return false; }

	/// Not used (yet?)
	virtual void MidiNote(int const /*channel*/, int const /*value*/, int const /*velocity*/) {}
	virtual void Event(uint32 const /*data*/) {} /// Not used (yet?)

	virtual bool DescribeValue(char * /*txt*/, int const /*param*/, int const /*value*/) { return false; }

	/// Not used (prolly never)
	virtual bool PlayWave(int const /*wave*/, int const /*note*/, float const /*volume*/) { return false; }
	virtual void SeqTick(int /*channel*/, int /*note*/, int /*ins*/, int /*cmd*/, int /*val*/) {}

	/// Not used (prolly never)
	virtual void StopWave() {}

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
#if !defined _WIN64 && !defined _WIN32 && !defined __CYGWIN__ && !defined __MSYS__ && !defined _UWIN
	#define PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT
	#define PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION
#else
	#define PSYCLE__PLUGIN__DETAIL__DYNAMIC_LINK__EXPORT __declspec(dllexport)
	#define PSYCLE__PLUGIN__DETAIL__CALLING_CONVENTION __cdecl
#endif
