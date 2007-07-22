#pragma once
#include "Constants.hpp"
#include "FileIO.hpp"
#include "Filter.hpp"

namespace psycle
{
	namespace host
	{
	class XMInstrument
	{
	public:
		/// Size of the Instrument's note mapping.
		static const int NOTE_MAP_SIZE = 120; // C-0 .. B-9
		/// A Note pair (note number=first, and sample number=second)
		typedef std::pair<unsigned char,unsigned char> NotePair;
		
		/// When a note starts to play in a channel, and there is still a note playing in it,
		/// do this on the currently playing note:
		enum NewNoteAction {
			STOP = 0x0,		///  [Note Cut]	(This one actually does a very fast fadeout)
			CONTINUE = 0x1,	///  [Ignore]
			NOTEOFF = 0x2,		///  [Note off]
			FADEOUT = 0x3		///  [Note fade]
			};
		enum DCType	{
			 DCT_NONE=0x0,
			 DCT_NOTE,
			 DCT_SAMPLE,
			 DCT_INSTRUMENT
			};
/*
		Using NewNoteAction so that we can convert easily from DCA to NNA.
		enum DCAction {
			DCA_STOP=0x0,
			DCA_NOTEOFF,
			DCA_FADEOUT
			};
*/

//////////////////////////////////////////////////////////////////////////
//  XMInstrument::WaveData Class declaration

		class WaveData {
		public:
			/** Wave Loop Type */
			enum LoopType {
				DO_NOT = 0x0,///< Do Nothing
				NORMAL = 0x1,///< normal Start --> End ,Start --> End ...
				BIDI = 0x2	 ///< bidirectional Start --> End, End --> Start ...
				};
			enum WaveForms {
				SINUS = 0x0,
				SQUARE = 0x1,
				SAWUP = 0x2,
				SAWDOWN = 0x3,
				RANDOM = 0x4
			};
			/// Constructor
			WaveData()
			{
				m_pWaveDataL = m_pWaveDataR = NULL;
				Init();
			};

			/// Initialize
			void Init(){
				DeleteWaveData();
				m_WaveName= _T("");
				m_WaveLength = 0;
				m_WaveGlobVolume = 1.0f; // Global volume ( global multiplier )
				m_WaveDefVolume = 128; // Default volume ( volume at which it starts to play. corresponds to 0Cxx/volume command )
				m_WaveLoopStart = 0;
				m_WaveLoopEnd = 0;
				m_WaveLoopType = DO_NOT;
				m_WaveSusLoopStart = 0;
				m_WaveSusLoopEnd = 0;
				m_WaveSusLoopType = DO_NOT;
				//todo: Add SampleRate functionality, and change WaveTune's one.
				// This means modifying the functions PeriodToSpeed (for linear slides) and NoteToPeriod (for amiga slides)
				m_WaveSampleRate = 8363;
				m_WaveTune = 0;
				m_WaveFineTune = 0;	
				m_WaveStereo = false;
				m_PanFactor = 0.5f;
				m_PanEnabled = false;
				m_VibratoAttack = 0;
				m_VibratoSpeed = 0;
				m_VibratoDepth = 0;
				m_VibratoType = 0;
			};

			/// Destructor
			~WaveData(){
				DeleteWaveData();
			};

			//	Object Functions
			void DeleteWaveData(){
				delete[] m_pWaveDataL;
				delete[] m_pWaveDataR;
				m_WaveLength = 0;
			};

			void AllocWaveData(const int iLen,const bool bStereo)
			{
				DeleteWaveData();
				m_pWaveDataL = new signed short[iLen];
				m_pWaveDataR = bStereo?new signed short[iLen]:NULL;
				m_WaveStereo = bStereo;
				m_WaveLength  = iLen;
			};

			bool Load(RiffFile& riffFile);
			void Save(RiffFile& riffFile);

			/// Wave Data Copy Operator
			void operator= (const WaveData& source)
			{
				Init();
				m_WaveName = source.m_WaveName;
				m_WaveLength = source.m_WaveLength;
				m_WaveGlobVolume = source.m_WaveGlobVolume;
				m_WaveDefVolume = source.m_WaveDefVolume;
				m_WaveLoopStart = source.m_WaveLoopStart;
				m_WaveLoopEnd = source.m_WaveLoopEnd;
				m_WaveLoopType = source.m_WaveLoopType;
				m_WaveSusLoopStart = source.m_WaveSusLoopStart;
				m_WaveSusLoopEnd = source.m_WaveSusLoopEnd;
				m_WaveSusLoopType = source.m_WaveSusLoopType;
				m_WaveTune = source.m_WaveTune;
				m_WaveFineTune = source.m_WaveFineTune;	
				m_WaveStereo = source.m_WaveStereo;
				m_VibratoAttack = source.m_VibratoAttack;
				m_VibratoSpeed = source.m_VibratoSpeed;
				m_VibratoDepth = source.m_VibratoDepth;
				m_VibratoType = source.m_VibratoType;

				AllocWaveData(source.m_WaveLength,source.m_WaveStereo);

				memcpy(m_pWaveDataL,source.m_pWaveDataL,source.m_WaveLength * sizeof(short));
				if(source.m_WaveStereo){
					memcpy(m_pWaveDataR,source.m_pWaveDataR,source.m_WaveLength * sizeof(short));
				}
			};


			// Properties
			const std::string WaveName(){ return m_WaveName;};
			void WaveName(std::string newname){ m_WaveName = newname;};

			const compiler::uint32 WaveLength(){ return m_WaveLength;};
			void WaveLength (const compiler::uint32 value){m_WaveLength = value;};

			const float WaveGlobVolume()const{ return m_WaveGlobVolume;};
			void WaveGlobVolume(const float value){m_WaveGlobVolume = value;};
			const compiler::uint16 WaveVolume(){ return m_WaveDefVolume;};
			void WaveVolume(const compiler::uint16 value){m_WaveDefVolume = value;};

			const float PanFactor(){ return m_PanFactor;};// Default position for panning ( 0..1 ) 0left 1 right. Bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
			void PanFactor(const float value){m_PanFactor = value;};
			bool PanEnabled(){ return m_PanEnabled;};
			void PanEnabled(bool pan){ m_PanEnabled=pan;};

			const compiler::uint32 WaveLoopStart(){ return m_WaveLoopStart;};
			void WaveLoopStart(const compiler::uint32 value){m_WaveLoopStart = value;};
			const compiler::uint32 WaveLoopEnd(){ return m_WaveLoopEnd;};
			void WaveLoopEnd(const compiler::uint32 value){m_WaveLoopEnd = value;};
			const LoopType WaveLoopType(){ return m_WaveLoopType;};
			void WaveLoopType(const LoopType value){ m_WaveLoopType = value;};

			const compiler::uint32 WaveSusLoopStart(){ return m_WaveSusLoopStart;};
			void WaveSusLoopStart(const compiler::uint32 value){m_WaveSusLoopStart = value;};
			const compiler::uint32 WaveSusLoopEnd(){ return m_WaveSusLoopEnd;};
			void WaveSusLoopEnd(const compiler::uint32 value){m_WaveSusLoopEnd = value;};
			const LoopType WaveSusLoopType(){ return m_WaveSusLoopType;};
			void WaveSusLoopType(const LoopType value){ m_WaveSusLoopType = value;};

			const compiler::sint16 WaveTune(){return m_WaveTune;};
			void WaveTune(const compiler::sint16 value){m_WaveTune = value;};
			const compiler::sint16 WaveFineTune(){return m_WaveFineTune;};
			void WaveFineTune(const compiler::sint16 value){m_WaveFineTune = value;};
			const compiler::uint32 WaveSampleRate(){return m_WaveSampleRate;};
			void WaveSampleRate(const compiler::uint32 value){m_WaveSampleRate = value;};

			const bool IsWaveStereo(){ return m_WaveStereo;};
			void IsWaveStereo(const bool value){ m_WaveStereo = value;};

			const compiler::uint8 VibratoType(){return m_VibratoType;};
			const compiler::uint8 VibratoSpeed(){return m_VibratoSpeed;};
			const compiler::uint8 VibratoDepth(){return m_VibratoDepth;};
			const compiler::uint8 VibratoAttack(){return m_VibratoAttack;};

			void VibratoType(const compiler::uint8 value){m_VibratoType = value ;};
			void VibratoSpeed(const compiler::uint8 value){m_VibratoSpeed = value ;};
			void VibratoDepth(const compiler::uint8 value){m_VibratoDepth = value ;};
			void VibratoAttack(const compiler::uint8 value){m_VibratoAttack = value ;};

			const bool IsAutoVibrato(){return m_VibratoDepth && m_VibratoSpeed;};

			const signed short * pWaveDataL(){ return m_pWaveDataL;};
			const signed short * pWaveDataR(){ return m_pWaveDataR;};
			
			signed short WaveDataL(const compiler::sint32 index) const { return (*(m_pWaveDataL + index));};
			signed short WaveDataR(const compiler::sint32 index) const { return (*(m_pWaveDataR + index));};
			
			void WaveDataL(const int index,const signed short value){ *(m_pWaveDataL + index) = value;};
			void WaveDataR(const int index,const signed short value){ *(m_pWaveDataR + index) = value;};

		private:

			std::string m_WaveName;
			compiler::uint32 m_WaveLength;		// Wave length in Samples.
			float m_WaveGlobVolume;
			compiler::uint16 m_WaveDefVolume;
			compiler::uint32 m_WaveLoopStart;
			compiler::uint32 m_WaveLoopEnd;
			LoopType m_WaveLoopType;
			compiler::uint32 m_WaveSusLoopStart;
			compiler::uint32 m_WaveSusLoopEnd;
			LoopType m_WaveSusLoopType;
			compiler::uint32 m_WaveSampleRate;
			compiler::sint16 m_WaveTune;
			compiler::sint16 m_WaveFineTune;	// [ -256 .. 256] full range = -/+ 1 seminote
			bool m_WaveStereo;
			signed short *m_pWaveDataL;
			signed short *m_pWaveDataR;
			bool m_PanEnabled;
			float m_PanFactor; // Default position for panning ( 0..1 ) 0left 1 right
			compiler::uint8 m_VibratoAttack;
			compiler::uint8 m_VibratoSpeed;
			compiler::uint8 m_VibratoDepth;
			compiler::uint8 m_VibratoType;

		};// WaveData()


//////////////////////////////////////////////////////////////////////////
//  XMInstrument::Envelope Class declaration

		class Envelope {
		public:
			// Invalid point. Used to indicate that sustain/normal loop is disabled.
			static const int INVALID = -1;
			// ValueType is a float value from  0 to 1.0  (or -1.0 1.0, or whatever else) which can be used as a multiplier.
			typedef float ValueType;
			//The meaning of the first value (int), is time, and the unit depends on the context.
			typedef std::pair<int,ValueType> PointValue;
			typedef std::vector< PointValue > Points;
			// constructor & destructor
			explicit Envelope()
			{	Init();
			}

			// copy Constructor
			Envelope(const Envelope& other)
			{
				operator=(other);
			}

			~Envelope(){;}

			// Init
			void Init()
			{	m_Enabled = false;
				m_Carry = false;
				m_SustainBegin = INVALID;
				m_SustainEnd = INVALID;
				m_LoopStart = INVALID;
				m_LoopEnd = INVALID;
				m_Points.clear();
			}

			// Object Functions.

			// Gets the time at which the pointIndex point is located.
			const int GetTime(const unsigned int pointIndex)
			{	
				if(pointIndex >= 0 && pointIndex < m_Points.size()) return m_Points[pointIndex].first;
				return INVALID;
			}
			// Sets a new time for an existing pointIndex point.
			const int SetTime(const unsigned int pointIndex,const int pointTime)
			{ 
				ASSERT(pointIndex >= 0 && pointIndex < (int)m_Points.size());
				m_Points[pointIndex].first = pointTime;
				return SetTimeAndValue(pointIndex,pointTime,m_Points[pointIndex].second);
			}
			// Gets the value of the pointIndex point.
			const ValueType GetValue(const unsigned int pointIndex)
			{ 
				ASSERT(pointIndex >= 0 && pointIndex < (int)m_Points.size());
				return m_Points[pointIndex].second;
			}
			// Sets the value pointVal to pointIndex point.
			void SetValue(const unsigned int pointIndex,const ValueType pointVal)
			{
				ASSERT(pointIndex >= 0 && pointIndex < (int)m_Points.size());
				m_Points[pointIndex].second = pointVal;
			}
			/// Appends a new point at the end of the array.
			/// Note: Be sure that the pointTime is the highest of the points, or use "Insert" instead.
			void Append(const int pointTime,const ValueType pointVal)
			{
				PointValue _value;
				_value.first = pointTime;
				_value.second = pointVal;
				m_Points.push_back(_value);
			};

			/// Helper to set a new time for an existing index.
			const int SetTimeAndValue(const int pointIndex,const int pointTime,const ValueType pointVal);

			/// Inserts a new point to the points Array.
			const int Insert(const int pointIndex,const ValueType pointVal);

			/// Removes a point from the points Array.
			void Delete(const int pointIndex);

			/// Clears the points Array
			void Clear()
			{
				m_Points.clear();
			};
			// Set or Get the point Index for Sustain and Loop.
			const int SustainBegin(){ return m_SustainBegin;};
			// value has to be an existing point!
			void SustainBegin(const int value){m_SustainBegin = value;};

			const int SustainEnd(){ return m_SustainEnd;};
			// value has to be an existing point!
			void SustainEnd(const int value){m_SustainEnd = value;};

			const int LoopStart(){return m_LoopStart;};
			// value has to be an existing point!
			void LoopStart(const int value){m_LoopStart = value;};

			const int LoopEnd(){return m_LoopEnd;};
			// value has to be an existing point!
			void LoopEnd(const int value){m_LoopEnd = value;};

			const int NumOfPoints(){ return m_Points.size();};

			void Load(RiffFile& riffFile,const UINT version);
			void Save(RiffFile& riffFile,const UINT version);

			// overloaded copy function
			Envelope& operator=(const Envelope& other)
			{
				if(this == &other){ return *this;};

				m_Enabled = other.m_Enabled;
				m_Carry = other.m_Carry;

				m_Points.clear();
				for(Points::const_iterator it = other.m_Points.begin();it != other.m_Points.end();it++)
				{
					m_Points.push_back(*it);
				}

				m_SustainBegin = other.m_SustainBegin;
				m_SustainEnd = other.m_SustainEnd;
				m_LoopStart = other.m_LoopStart;
				m_LoopEnd = other.m_LoopEnd;

				return *this;
			}
			// Properties
			/// If the envelope IsEnabled, it is used and triggered. Else, it is not.
			const bool IsEnabled(){ return m_Enabled;}
			void IsEnabled(const bool value){ m_Enabled = value;}

			//  if IsCarry() and a new note enters, the envelope position is set to
			//  that of the previous note *on the same channel*
			//\ todo: implement carry
			const bool IsCarry(){ return m_Carry;}
			void IsCarry(const bool value){ m_Carry = value;}

		private:
			// Envelope is enabled or disabled
			bool m_Enabled;
			// ????
			bool m_Carry;
			//Array of Points of the envelope.
			// first : time at which to set the value. Unit can be different things depending on the context.
			// second : 0 .. 1.0f . (or -1.0 1.0 or whatever else) Use it as a multiplier.
			Points m_Points;
			// Loop Start Point
			int m_LoopStart;
			// Loop End Point
			int m_LoopEnd; 
			// Sustain Start Point
			int m_SustainBegin;
			// Sustain End Point
			int m_SustainEnd;
		};// class Envelope


//////////////////////////////////////////////////////////////////////////
//  XMInstrument Class declaration
		XMInstrument();
		~XMInstrument();

		void Init();

		bool Load(RiffFile& riffFile);
		void Save(RiffFile& riffFile);

		void operator= (const XMInstrument & other)
		{
			int i;
			m_bEnabled = other.m_bEnabled;

			m_Name = other.m_Name;

			m_Lines = other.m_Lines;

			// Volume
			m_AmpEnvelope = other.m_AmpEnvelope;
			m_GlobVol = other.m_GlobVol;
			m_VolumeFadeSpeed = other.m_VolumeFadeSpeed;

			// Paninng
			m_PanEnvelope = other.m_PanEnvelope;
			m_InitPan = other.m_InitPan;
			m_PanEnabled=other.m_PanEnabled;
			m_NoteModPanCenter=other.m_NoteModPanCenter;
			m_NoteModPanSep=other.m_NoteModPanSep;

			// Pitch/Filter Envelope
			m_PitchEnvelope = other.m_PitchEnvelope;
			m_FilterEnvelope = other.m_FilterEnvelope;
			m_FilterCutoff = other.m_FilterCutoff;
			m_FilterResonance = other.m_FilterResonance;
			m_FilterEnvAmount = other.m_FilterEnvAmount;
			m_FilterType = other.m_FilterType;

			m_RandomVolume = other.m_RandomVolume;
			m_RandomPanning = other.m_RandomPanning;
			m_RandomCutoff = other.m_RandomCutoff;
			m_RandomResonance = other.m_RandomResonance;

			m_NNA = other.m_NNA;
			m_DCT = other.m_DCT;
			m_DCA = other.m_DCA;
			for(i=0;i<NOTE_MAP_SIZE;i++)
			{
				m_AssignNoteToSample[i]=other.m_AssignNoteToSample[i];
			}
		};

		// Properties
		
		// IsEnabled() is used on Saving, to not store unused instruments
		const bool IsEnabled(){ return m_bEnabled;};
		void IsEnabled(const bool value){ m_bEnabled = value;};

		std::string& Name(){return m_Name;};
		void Name(const std::string& name) { m_Name= name; }

		const compiler::uint16 Lines(){ return m_Lines;};
		void Lines(const compiler::uint16 value){ m_Lines = value;};

		Envelope* const AmpEnvelope(){ return &m_AmpEnvelope;};
		Envelope* const PanEnvelope(){return &m_PanEnvelope;};
		Envelope* const FilterEnvelope(){ return &m_FilterEnvelope;};
		Envelope* const PitchEnvelope(){return &m_PitchEnvelope;};

		const float GlobVol() { return m_GlobVol;};
		void GlobVol(const float value){m_GlobVol = value;};
		const float VolumeFadeSpeed() { return m_VolumeFadeSpeed;};
		void VolumeFadeSpeed(const float value){ m_VolumeFadeSpeed = value;};

		const float Pan() { return m_InitPan;};// Default position for panning ( 0..1 ) 0left 1 right. Bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
		void Pan(const float pan) { m_InitPan = pan;};
		const bool PanEnabled() { return m_PanEnabled;};
		void PanEnabled(const bool pan) { m_PanEnabled = pan;};
		const compiler::uint8 NoteModPanCenter() { return m_NoteModPanCenter;};
		void NoteModPanCenter(const compiler::uint8 pan) { m_NoteModPanCenter = pan;};
		const compiler::sint8 NoteModPanSep() { return m_NoteModPanSep;};
		void NoteModPanSep(const compiler::sint8 pan) { m_NoteModPanSep = pan;};

		const compiler::uint8 FilterCutoff(){ return m_FilterCutoff;};
		void FilterCutoff(const compiler::uint8 value){m_FilterCutoff = value;};
		const compiler::uint8 FilterResonance() { return m_FilterResonance;};
		void FilterResonance(const compiler::uint8 value){m_FilterResonance = value;};
		const compiler::sint16 FilterEnvAmount() { return m_FilterEnvAmount;};
		void FilterEnvAmount(const compiler::sint16 value){ m_FilterEnvAmount = value;};
		const dsp::FilterType FilterType(){ return m_FilterType;};
		void FilterType(const dsp::FilterType value){ m_FilterType = value;};

		const float RandomVolume(){return  m_RandomVolume;};
		void RandomVolume(const float value){m_RandomVolume = value;};
		const float RandomPanning(){return  m_RandomPanning;};
		void RandomPanning(const float value){m_RandomPanning = value;};
		const float RandomCutoff(){return m_RandomCutoff;};
		void RandomCutoff(const float value){m_RandomCutoff = value;};
		const float RandomResonance(){return m_RandomResonance;};
		void RandomResonance(const float value){m_RandomResonance = value;};

		const NewNoteAction NNA() { return m_NNA;};
		void NNA(const NewNoteAction value){ m_NNA = value;};
		const DCType DCT() { return m_DCT;};
		void DCT(const DCType value){ m_DCT = value;};
		const NewNoteAction DCA() { return m_DCA;};
		void DCA(const NewNoteAction value){ m_DCA = value;};

		const NotePair NoteToSample(const int note){return m_AssignNoteToSample[note];};
		void NoteToSample(const int note,const NotePair npair){m_AssignNoteToSample[note] = npair;};

	private:
		bool m_bEnabled;

		std::string m_Name;

		compiler::uint16 m_Lines;			// If m_Lines > 0 use samplelen/(tickduration*m_Lines) to determine the wave speed instead of the note.

		Envelope m_AmpEnvelope;				// envelope range = [0.0f..1.0f]
		Envelope m_PanEnvelope;				// envelope range = [-1.0f..1.0f]
		Envelope m_PitchEnvelope;			// envelope range = [-1.0f..1.0f]
		Envelope m_FilterEnvelope;			// envelope range = [0.0f..1.0f]

		float m_GlobVol;					// [0..1.0f] Global volume affecting all samples of the instrument.
		float m_VolumeFadeSpeed;			// [0..1.0f] Fadeout speed. Decreasing amount for each tracker tick.

		// Paninng
		bool m_PanEnabled;
		float m_InitPan;					// Initial panFactor (if enabled) [-1..1]
		compiler::uint8 m_NoteModPanCenter;	// Note number for center pan position
		compiler::sint8 m_NoteModPanSep;		// -32..32. 1/256th of panFactor change per seminote.

		compiler::uint8 m_FilterCutoff;		// Cutoff Frequency [0..127]
		compiler::uint8 m_FilterResonance;	// Resonance [0..127]
		compiler::sint16 m_FilterEnvAmount;	// EnvAmount [-128..128]
		dsp::FilterType m_FilterType;		// Filter Type [0..4]

		// Randomness. Applies on new notes.
		float m_RandomVolume;		// Random Volume % [ 0.0 -> No randomize. 1.0 = randomize full scale.]
		float m_RandomPanning;		// Random Panning  (same)
		float m_RandomCutoff;		// Random CutOff	(same)
		float m_RandomResonance;	// Random Resonance	(same)

		NewNoteAction m_NNA; // Action to take on the playing voice when any new note comes in the same channel.
		DCType m_DCT;
		NewNoteAction m_DCA; // Action to take on the playing voice when a new note comes in the same channel 
							 // and the element defined by m_DCT is the same. (like the same note value).

		/// Table of mapped notes to samples
		// (note number=first, sample number=second)
		///\todo: Could it be interesting to map other things like volume,panning, cutoff...?
		NotePair m_AssignNoteToSample[NOTE_MAP_SIZE];
	};



//\todo : implement the following for inter-XMSampler sharing of instruments.
	class SampleList{
	public:
		SampleList(){top=0;};
		~SampleList(){};
		int AddSample(XMInstrument::WaveData &wave)
		{
			if ( top+1<MAX_INSTRUMENTS)
			{
				m_waves[top++]=wave;
				return top-1;
			}
			else return -1;
		}
		int SetSample(XMInstrument::WaveData &wave,int pos)
		{
			ASSERT(pos<MAX_INSTRUMENTS);
			m_waves[pos]=wave;
			return pos;
		}
		XMInstrument::WaveData &operator[](int pos)
		{
			ASSERT(pos<MAX_INSTRUMENTS);
			return m_waves[pos];
		}
	private:
		XMInstrument::WaveData m_waves[MAX_INSTRUMENTS];
		int top;
	};
	class InstrumentList {
	public:
		InstrumentList(){top=0;}
		~InstrumentList(){};
		int AddIns(XMInstrument &ins)
		{
			if ( top+1<MAX_INSTRUMENTS)
			{
				m_inst[top++]=ins;
				return top-1;
			}
			else return -1;
		}
		int SetInst(XMInstrument &inst,int pos)
		{
			ASSERT(pos<MAX_INSTRUMENTS);
			m_inst[pos]=inst;
			return pos;
		}
		XMInstrument &operator[](int pos){
			ASSERT(pos<MAX_INSTRUMENTS);
			return m_inst[pos];
		}
	private:
		XMInstrument m_inst[MAX_INSTRUMENTS];
		int top;
	};
	}
}
