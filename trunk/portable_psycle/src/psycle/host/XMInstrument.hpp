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
		enum DCType
			{
			 DCT_NONE=0x0,
			 DCT_NOTE,
			 DCT_SAMPLE,
			 DCT_INSTRUMENT
			};
		enum DCAction
			{
			DCA_STOP=0x0,
			DCA_NOTEOFF,
			DCA_FADEOUT
			};

		//////////////////////////////////////////////////////////////////////////
		//  XMInstrument::Envelope Class declaration
		 
		 /*@Envelope Point‚16points
		 *  Envelope Step: 1 Tick = 1/44100 sec 
		 *  Envelope Range: 0.0 .. 1.0f
		 */
		class Envelope {

		public:
			// Used as a Scale multiplier.
			typedef float ValueType;
			// Invalid point. Used to indicate that sustain/loop are disabled.
			static const short INVALID = -1;
			typedef std::pair<int,ValueType> PointValue;
			typedef std::vector< PointValue > Points;
			// constructor & destructor
			explicit Envelope()
			{	Init();
			}

			// copy Constructor
			Envelope(const Envelope& other)
			{	Init();
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
			}

			// property 
			/// If the envelope IsEnabled, it is used and triggered. Else, it is not.
			const bool IsEnabled(){ return m_Enabled;}
			void IsEnabled(const bool value){ m_Enabled = value;}

			const bool IsCarry(){ return m_Carry;}
			void IsCarry(const bool value){ m_Carry = value;}
			
			//////////////////////////////////////////////////////////////////////////
			// Point Functions. Helpers to get and set the values for them.

			// Gets the time at which the pointIndex point is located.
			const int GetTime(const int pointIndex)
			{	
				if(pointIndex >= 0 && pointIndex < (int)m_Points.size()){
					return m_Points[pointIndex].first;
				}
				return INVALID;
			}
			// Sets a new time for an existing pointIndex point.
			const int SetTime(const int pointIndex,const int pointTime)
			{ 
				ASSERT(pointIndex >= 0 && pointIndex < (int)m_Points.size());
				m_Points[pointIndex].first = pointTime;
				return SetTimeAndValue(pointIndex,pointTime,m_Points[pointIndex].second);
			}
			// Gets the value of the pointIndex point.
			const ValueType GetValue(const int pointIndex)
			{ 
				ASSERT(pointIndex >= 0 && pointIndex < (int)m_Points.size());
				return m_Points[pointIndex].second;
			}
			// Sets the value pointVal to pointIndex point.
			void SetValue(const int pointIndex,const ValueType pointVal)
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
			void SustainBegin(const int value){m_SustainBegin = value;};

			const int SustainEnd(){ return m_SustainEnd;};
			void SustainEnd(const int value){m_SustainEnd = value;};

			const int LoopStart(){return m_LoopStart;};
			void LoopStart(const int value){m_LoopStart = value;};

			const int LoopEnd(){return m_LoopEnd;};
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

		private:
			// Envelope is enabled or disabled
			bool m_Enabled;
			// ????
			bool m_Carry;
			//Array of Points of the envelope.
			// first : sample position since relative to envelope start. THIS HAS TO BE UPDATED IF CHANGED BPM and/or SampleRate.
			// second : 0 .. 1.0f .Use it as a multiplier.
			Points m_Points; 
			///< Sustain Start Point
			int m_SustainBegin;
			///< Sustain End Point
			int m_SustainEnd;
			///< Loop Start Point
			int m_LoopStart;
			///< Loop End Point
			int m_LoopEnd;
		};// class Envelope


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
				m_WaveLength = 0;
				m_WaveGlobVolume = 1.0f; // Global volume ( global multiplier )
				m_WaveDefVolume = 256; // Default volume ( volume at which it starts to play. corresponds to 0Cxx )
				m_WaveLoopStart = 0;
				m_WaveLoopEnd = 0;
				m_WaveLoopType = DO_NOT;
				m_WaveSusLoopStart = 0;
				m_WaveSusLoopEnd = 0;
				m_WaveSusLoopType = DO_NOT;
				m_WaveTune = 0;
				m_WaveFineTune = 0;	
				m_WaveStereo = false;
				m_PanFactor = 0.5f;
				m_PanEnabled = false;
				m_VibratoRate = 0;
				m_VibratoSweep = 0;
				m_VibratoDepth = 0;
				m_VibratoType = 0;
				m_WaveName= _T("");
			};

			/// Destructor
			~WaveData(){
				DeleteWaveData();
			};
			
			const UINT WaveLength(){ return m_WaveLength;};
			void WaveLength (const UINT value){m_WaveLength = value;};

			const float WaveGlobVolume(){ return m_WaveGlobVolume;};
			void WaveGlobVolume(const float value){m_WaveGlobVolume = value;};
			const unsigned short WaveVolume(){ return m_WaveDefVolume;};
			void WaveVolume(const unsigned short value){m_WaveDefVolume = value;};

			const float PanFactor(){ return m_PanFactor;};
			void PanFactor(const float value){m_PanFactor = value;};
			bool PanEnabled(){ return m_PanEnabled;};
			void PanEnabled(bool pan){ m_PanEnabled=pan;};

			const UINT WaveLoopStart(){ return m_WaveLoopStart;};
			void WaveLoopStart(const UINT value){m_WaveLoopStart = value;};
			const UINT WaveLoopEnd(){ return m_WaveLoopEnd;};
			void WaveLoopEnd(const UINT value){m_WaveLoopEnd = value;};
			const LoopType WaveLoopType(){ return m_WaveLoopType;};
			void WaveLoopType(const LoopType value){ m_WaveLoopType = value;};

			const UINT WaveSusLoopStart(){ return m_WaveSusLoopStart;};
			void WaveSusLoopStart(const UINT value){m_WaveSusLoopStart = value;};
			const UINT WaveSusLoopEnd(){ return m_WaveSusLoopEnd;};
			void WaveSusLoopEnd(const UINT value){m_WaveSusLoopEnd = value;};
			const LoopType WaveSusLoopType(){ return m_WaveSusLoopType;};
			void WaveSusLoopType(const LoopType value){ m_WaveSusLoopType = value;};

			const short WaveTune(){return m_WaveTune;};
			void WaveTune(const short value){m_WaveTune = value;};
			const short WaveFineTune(){return m_WaveFineTune;};
			void WaveFineTune(const short value){m_WaveFineTune = value;};

			const bool IsWaveStereo(){ return m_WaveStereo;};
			void IsWaveStereo(const bool value){ m_WaveStereo = value;};

			const unsigned char VibratoType(){return m_VibratoType;};
			const unsigned char VibratoSweep(){return m_VibratoSweep;};
			const unsigned char VibratoDepth(){return m_VibratoDepth;};
			const unsigned char VibratoRate(){return m_VibratoRate;};

			void VibratoType(const unsigned char value){m_VibratoType = value ;};
			void VibratoSweep(const unsigned char value){m_VibratoSweep = value ;};
			void VibratoDepth(const unsigned char value){m_VibratoDepth = value ;};
			void VibratoRate(const unsigned char value){m_VibratoRate = value ;};

			const bool IsAutoVibrato(){return m_VibratoDepth && m_VibratoRate;};

			const std::string WaveName(){ return m_WaveName;};

			const signed short * pWaveDataL(){ return m_pWaveDataL;};
			const signed short * pWaveDataR(){ return m_pWaveDataR;};
			
			signed short WaveDataL(const int index) const { return (*(m_pWaveDataL + index));};
			signed short WaveDataR(const int index) const { return (*(m_pWaveDataR + index));};
			
			void WaveDataL(const int index,const signed short value){ *(m_pWaveDataL + index) = value;};
			void WaveDataR(const int index,const signed short value){ *(m_pWaveDataR + index) = value;};

			void DeleteWaveData(){
				zapArray(m_pWaveDataL);
				zapArray(m_pWaveDataR);
			};

			void AllocWaveData(const int iLen,const bool bStereo)
			{
				DeleteWaveData();
				m_pWaveDataL = new signed short[iLen];
				m_pWaveDataR = bStereo?new signed short[iLen]:NULL;
				m_WaveStereo = bStereo;
				m_WaveLength  = iLen;
			};

			void Load(RiffFile& riffFile,const UINT version);
			void Save(RiffFile& riffFile,const UINT version);

			/// Wave Data Copy Operator
			void operator= (const WaveData& source)
			{
				Init();
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
				m_WaveName = source.m_WaveName;
				m_VibratoRate = source.m_VibratoRate;
				m_VibratoSweep = source.m_VibratoSweep;
				m_VibratoDepth = source.m_VibratoDepth;
				m_VibratoType = source.m_VibratoType;

				AllocWaveData(source.m_WaveLength,source.m_WaveStereo);
			
				memcpy(m_pWaveDataL,source.m_pWaveDataL,source.m_WaveLength * sizeof(short));
				if(source.m_WaveStereo){
					memcpy(m_pWaveDataR,source.m_pWaveDataR,source.m_WaveLength * sizeof(short));
				}
			};

		private:

			UINT m_WaveLength;
			float m_WaveGlobVolume;
			unsigned short m_WaveDefVolume;
			UINT m_WaveLoopStart;
			UINT m_WaveLoopEnd;
			LoopType m_WaveLoopType;
			UINT m_WaveSusLoopStart;
			UINT m_WaveSusLoopEnd;
			LoopType m_WaveSusLoopType;
			short m_WaveTune;
			short m_WaveFineTune;	// [ -256 .. 256] full range = -/+ 1 seminote
			bool m_WaveStereo;
			std::string m_WaveName;
			signed short *m_pWaveDataL;
			signed short *m_pWaveDataR;
			bool m_PanEnabled;
			float m_PanFactor; // Default position for panning ( 0..1 ) 0left 1 right
			unsigned char m_VibratoRate;
			unsigned char m_VibratoSweep;
			unsigned char m_VibratoDepth;
			unsigned char m_VibratoType;

		};// WaveData()


		//////////////////////////////////////////////////////////////////////////
		//  XMInstrument Class declaration
		XMInstrument();
		~XMInstrument();

		void Init();
//		void DeleteLayer(int c);

		void Load(RiffFile& riffFile,const UINT version);
		void Save(RiffFile& riffFile,const UINT version);

		// Property //
		
		const bool IsEnabled(){ return m_bEnabled;};
		void IsEnabled(const bool value){ m_bEnabled = value;};

		std::string& Name(){return m_Name;};
		void Name(const std::string& name) { m_Name= name; }

		const bool IsLinesMode(){ return m_LinesMode;};
		void IsLinesMode(const bool value){m_LinesMode = value;};
		const int Lines(){ return m_Lines;};
		void Lines(const int value){ m_Lines = value;};

		Envelope* const AmpEnvelope(){ return &m_AmpEnvelope;};
		Envelope* const PanEnvelope(){return &m_PanEnvelope;};
		Envelope* const FilterEnvelope(){ return &m_FilterEnvelope;};
		Envelope* const PitchEnvelope(){return &m_PitchEnvelope;};

		const int FilterCutoff(){ return m_FilterCutoff;};
		void FilterCutoff(const int value){m_FilterCutoff = value;};
		const int FilterResonance() { return m_FilterResonance;};
		void FilterResonance(const int value){m_FilterResonance = value;};
		const int FilterEnvAmount() { return m_FilterEnvAmount;};
		void FilterEnvAmount(const int value){ m_FilterEnvAmount = value;};
		const dsp::FilterType FilterType(){ return m_FilterType;};
		void FilterType(const dsp::FilterType value){ m_FilterType = value;};

		const unsigned char RandomVolume(){return  m_RandomVolume;};///< Random Volume
		void RandomVolume(const unsigned char value){m_RandomVolume = value;};
		const unsigned char RandomPanning(){return  m_RandomPanning;};///< Random Panning
		void RandomPanning(const unsigned char value){m_RandomPanning = value;};
		const unsigned char RandomCutoff(){return m_RandomCutoff;};///< Random CutOff
		void RandomCutoff(const unsigned char value){m_RandomCutoff = value;};
		const unsigned char RandomResonance(){return m_RandomResonance;};///< Random Resonance
		void RandomResonance(const unsigned char value){m_RandomResonance = value;};
		const unsigned char RandomSampleStart(){return m_RandomSampleStart;};///< Random Sample Start
		void RandomSampleStart(const unsigned char value){m_RandomSampleStart = value;};

		const float Pan() { return m_InitPan;};
		void Pan(const float pan) { m_InitPan = pan;};
		const bool PanEnabled() { return m_PanEnabled;};
		void PanEnabled(const bool pan) { m_PanEnabled = pan;};
		const char PitchPanSep() { return m_PitchPanSep;};
		void PitchPanSep(const char pan) { m_PitchPanSep = pan;};
		const unsigned char PitchPanCenter() { return m_PitchPanCenter;};
		void PitchPanCenter(const unsigned char pan) { m_PitchPanCenter = pan;};

		const float GlobVol() { return m_GlobVol;};
		void GlobVol(const float value){m_GlobVol = value;};
		//const bool IsVolumeFade() { return m_bVolumeFade;};
		//void IsVolumeFade(const bool value){m_bVolumeFade = value;};
		const float VolumeFadeSpeed() { return m_VolumeFadeSpeed;};
		void VolumeFadeSpeed(const float value){ m_VolumeFadeSpeed = value;};
/*
		const int AutoVibratoType(){return m_AutoVibratoType;};
		const int AutoVibratoSweep(){return m_AutoVibratoSweep;};
		const int AutoVibratoDepth(){return m_AutoVibratoDepth;};
		const int AutoVibratoRate(){return m_AutoVibratoRate;};

		void AutoVibratoType(const int value){m_AutoVibratoType = value ;};
		void AutoVibratoSweep(const int value){m_AutoVibratoSweep = value ;};
		void AutoVibratoDepth(const int value){m_AutoVibratoDepth = value ;};
		void AutoVibratoRate(const int value){m_AutoVibratoRate = value ;};

		const bool IsAutoVibrato(){return m_AutoVibratoDepth && m_AutoVibratoRate;};
*/
		const char MidiChannel() { return m_MidiChannel;};
		void MidiChannel(const char value){ m_MidiChannel = value;};
		const char MidiProgram() { return m_MidiProgram;};
		void MidiProgram(const char value){ m_MidiProgram = value;};
		const char MidiBank() { return m_MidiBank;};
		void MidiBank(const char value){ m_MidiBank = value;};

		const NewNoteAction NNA() { return m_NNA;};
		void NNA(const NewNoteAction value){ m_NNA = value;};
		const DCType DCT() { return m_DCT;};
		void DCT(const DCType value){ m_DCT = value;};
		const DCAction DCA() { return m_DCA;};
		void DCA(const DCAction value){ m_DCA = value;};

		const NotePair NoteToSample(const int note){return m_AssignNoteToSample[note];};
		void NoteToSample(const int note,const NotePair npair){m_AssignNoteToSample[note] = npair;};

//		WaveData& rWaveLayer(const int index){ return m_WaveLayer[index];};

		void operator= (const XMInstrument & other)
		{
			int i;
			m_bEnabled = other.m_bEnabled;

			m_Name = other.m_Name;

			m_LinesMode = other.m_LinesMode;
			m_Lines = other.m_Lines;

			m_AmpEnvelope = other.m_AmpEnvelope;
			m_FilterEnvelope = other.m_FilterEnvelope;
			m_FilterCutoff = other.m_FilterCutoff;
			m_FilterResonance = other.m_FilterResonance;
			m_FilterEnvAmount = other.m_FilterEnvAmount;
			m_FilterType = other.m_FilterType;

			// Paninng
			m_InitPan = other.m_InitPan;
			m_PanEnabled=other.m_PanEnabled;
			m_PanEnvelope = other.m_PanEnvelope;
			m_PitchPanCenter=other.m_PitchPanCenter;
			m_PitchPanSep=other.m_PitchPanSep;

			// Pitch Envelope
			m_PitchEnvelope = other.m_PitchEnvelope;

			m_RandomVolume = other.m_RandomVolume;
			m_RandomPanning = other.m_RandomPanning;
			m_RandomCutoff = other.m_RandomCutoff;
			m_RandomResonance = other.m_RandomResonance;
			m_RandomSampleStart = other.m_RandomSampleStart;

			m_GlobVol = other.m_GlobVol;
			//m_bVolumeFade = other.m_bVolumeFade;
			m_VolumeFadeSpeed = other.m_VolumeFadeSpeed;

/*			// Auto Vibrato
			m_AutoVibratoType = other.m_AutoVibratoType;
			m_AutoVibratoSweep = other.m_AutoVibratoSweep;
			m_AutoVibratoDepth = other.m_AutoVibratoDepth;
			m_AutoVibratoRate = other.m_AutoVibratoRate;

*/			// 
			m_MidiChannel=other.m_MidiChannel;
			m_MidiProgram=other.m_MidiProgram;
			m_MidiBank=other.m_MidiBank;
			
			m_NNA = other.m_NNA;
			m_DCT = other.m_DCT;
			m_DCA = other.m_DCA;
			for(i=0;i<NOTE_MAP_SIZE;i++)
			{
				m_AssignNoteToSample[i]=other.m_AssignNoteToSample[i];
			}
		};

	private:
		// 
		bool m_bEnabled;

		std::string m_Name;

		bool m_LinesMode;
		unsigned char m_Lines;

		Envelope m_AmpEnvelope;
		Envelope m_FilterEnvelope;
		unsigned short m_FilterCutoff;		///< Cutoff Frequency [0-127]
		unsigned short m_FilterResonance;	///< Resonance [0-127]
		short m_FilterEnvAmount;	///< EnvAmount [-128,128]
		dsp::FilterType m_FilterType;		///< Filter Type [0-4]

		// Paninng
		float m_InitPan;
		bool m_PanEnabled;
		short m_PitchPanCenter;
		short m_PitchPanSep;

		Envelope m_PanEnvelope;
		// Pitch Envelope
		Envelope m_PitchEnvelope;

		// LFO
		unsigned char m_RandomVolume;	///< Random Volume % [ 0 -> No randomize. 100 = randomize full scale.]
		unsigned char m_RandomPanning;	///< Random Panning
		unsigned char m_RandomCutoff;	///< Random CutOff
		unsigned char m_RandomResonance;	///< Random Resonance
		unsigned char m_RandomSampleStart;///< Random SampleStart

		float m_GlobVol;
//		bool m_bVolumeFade;
		float m_VolumeFadeSpeed;

		// Auto Vibrato
/*		int m_AutoVibratoType;
		int m_AutoVibratoSweep;
		int m_AutoVibratoDepth;
		int m_AutoVibratoRate;
*/
		char m_MidiChannel;
		char m_MidiProgram;
		short m_MidiBank;

		NewNoteAction m_NNA;
		DCType m_DCT;
		DCAction m_DCA;

		/// Table of mapped notes to samples
		// (note number=first, sample number=second)
		///\todo: Could it be interesting to map other things like volume,panning, cutoff...?
		NotePair m_AssignNoteToSample[NOTE_MAP_SIZE];
		
	};
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
