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
		// Max number of samples per Instrument
		static const int MAX_INSTRUMENT_SAMPLES = 32;
		/// Size of the Instrument's note mapping.
		static const int NOTE_MAP_SIZE = 120; // C-0 .. B-9
		// A Note pair (note number=first, and sample number=second)
		typedef std::pair<short,short> NotePair;
		
		/// When a note starts to play in a channel, and there is still a note playing in it,
		/// do this on the currently playing note:
		enum NewNoteAction {
			STOP = 0x0,		///  [Note Cut]	(This one actually does a very fast fadeout)
			CONTINUE = 0x1,	///  [Ignore]
			NOTEOFF = 0x2,		///  [Note off]
			FADEOUT = 0x3		///  [Note fade]
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
			typedef std::pair<short,ValueType> PointValue;
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
				m_SustainBegin = INVALID;
				m_SustainEnd = INVALID;
				m_LoopStart = INVALID;
				m_LoopEnd = INVALID;
			}

			// property 
			/// If the envelope IsEnabled, it is used and triggered. Else, it is not.
			const bool IsEnabled(){ return m_Enabled;}
			void IsEnabled(const bool value){ m_Enabled = value;}

			
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
				m_WaveVolume = 0;
				m_WaveLoopStart = 0;
				m_WaveLoopEnd = 0;
				m_WaveTune = 0;
				m_WaveFineTune = 0;	
				m_WaveStereo = false;
				m_WaveLoopType = DO_NOT;
				memset(m_WaveName,0,sizeof(m_WaveName)*sizeof(TCHAR));
			};

			/// Destructor
			~WaveData(){
				DeleteWaveData();
			};
			
			const UINT WaveLength(){ return m_WaveLength;};
			void WaveLength (const UINT value){m_WaveLength = value;};

			const unsigned short WaveVolume(){ return m_WaveVolume;};
			void WaveVolume(const unsigned short value){m_WaveVolume = value;};
			
			const UINT WaveLoopStart(){ return m_WaveLoopStart;};
			void WaveLoopStart(const UINT value){m_WaveLoopStart = value;};

			const UINT WaveLoopEnd(){ return m_WaveLoopEnd;};
			void WaveLoopEnd(const UINT value){m_WaveLoopEnd = value;};

			const int WaveTune(){return m_WaveTune;};
			void WaveTune(const int value){m_WaveTune = value;};

			const int WaveFineTune(){return m_WaveFineTune;};
			void WaveFineTune(const int value){m_WaveFineTune = value;};

			const LoopType WaveLoopType(){ return m_WaveLoopType;};
			void WaveLoopType(const LoopType value){ m_WaveLoopType = value;};

			const bool IsWaveStereo(){ return m_WaveStereo;};
			void IsWaveStereo(const bool value){ m_WaveStereo = value;};

			const TCHAR * WaveName(){ return m_WaveName;};

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
				m_WaveVolume = source.m_WaveVolume;
				m_WaveLoopStart = source.m_WaveLoopStart;
				m_WaveLoopEnd = source.m_WaveLoopEnd;
				m_WaveTune = source.m_WaveTune;
				m_WaveFineTune = source.m_WaveFineTune;	
				m_WaveLoopType = source.m_WaveLoopType;
				m_WaveStereo = source.m_WaveStereo;
				_tcscpy(m_WaveName,source.m_WaveName);

				AllocWaveData(source.m_WaveLength,source.m_WaveStereo);
			
				memcpy(m_pWaveDataL,source.m_pWaveDataL,source.m_WaveLength * sizeof(short));
				if(source.m_WaveStereo){
					memcpy(m_pWaveDataR,source.m_pWaveDataR,source.m_WaveLength * sizeof(short));
				}
			};

		private:

			UINT m_WaveLength;
			unsigned short m_WaveVolume;
			UINT m_WaveLoopStart;
			UINT m_WaveLoopEnd;
			int m_WaveTune;
			int m_WaveFineTune;	
			LoopType m_WaveLoopType;
			bool m_WaveStereo;
			TCHAR m_WaveName[32];
			signed short *m_pWaveDataL;
			signed short *m_pWaveDataR;

		};// WaveData()


		//////////////////////////////////////////////////////////////////////////
		//  XMInstrument Class declaration
		XMInstrument();
		~XMInstrument();

		void Init();
		void DeleteLayer(int c);

		void Load(RiffFile& riffFile,const UINT version);
		void Save(RiffFile& riffFile,const UINT version);

		// Property //
		
		Envelope* const AmpEnvelope(){ return &m_AmpEnvelope;};
		Envelope* const FilterEnvelope(){ return &m_FilterEnvelope;};
		
		Envelope* const PanEnvelope(){return &m_PanEnvelope;};
		Envelope* const PitchEnvelope(){return &m_PitchEnvelope;};

		const int FilterCutoff(){ return m_FilterCutoff;};
		void FilterCutoff(const int value){m_FilterCutoff = value;};

		const int FilterResonance() { return m_FilterResonance;};
		void FilterResonance(const int value){m_FilterResonance = value;};

		const int FilterEnvAmount() { return m_FilterEnvAmount;};
		void FilterEnvAmount(const int value){ m_FilterEnvAmount = value;};
		
		const dsp::FilterType FilterType(){ return m_FilterType;};
		void FilterType(const dsp::FilterType value){ m_FilterType = value;};

		const NewNoteAction NNA() { return m_NNA;};
		void NNA(const NewNoteAction value){ m_NNA = value;};

		///\todo : Add a "Depth" Parameter to randompanning/cutoff/ressonance
		const bool IsRandomPanning(){return  m_RandomPanning;};///< Random Panning
		void IsRandomPanning(const bool value){m_RandomPanning = value;};

		const bool IsRandomCutoff(){return m_RandomCutoff;};///< Random CutOff
		void IsRandomCutoff(const bool value){m_RandomCutoff = value;};

		const bool IsRandomResonance(){return m_RandomResonance;};///< Random Resonance
		void IsRandomResonance(const bool value){m_RandomResonance = value;};
		
		const bool IsRandomSampleStart(){return m_RandomSampleStart;};///< Random Sample Start
		void IsRandomSampleStart(const bool value){m_RandomSampleStart = value;};

		const bool IsLoop(){ return m_Loop;};
		void IsLoop(const bool value){m_Loop = value;};

		const int Lines(){ return m_Lines;};
		void Lines(const int value){ m_Lines = value;};
	
		const float Pan() { return m_InitPan;};
		void Pan(const float pan) { m_InitPan = pan;};

		/// Get Volume Fade Enable or Disable
		const bool IsVolumeFade() { return m_bVolumeFade;};
		/// Set Volume Fade Enable or Disable
		void IsVolumeFade(const bool value){m_bVolumeFade = value;};

		/// Get Volume Fade Speed 
		const float VolumeFadeSpeed() { return m_VolumeFadeSpeed;};
		/// Set Volume Fade Speed
		void VolumeFadeSpeed(const float value){ m_VolumeFadeSpeed = value;};

		const int AutoVibratoType(){return m_AutoVibratoType;};
		const int AutoVibratoSweep(){return m_AutoVibratoSweep;};
		const int AutoVibratoDepth(){return m_AutoVibratoDepth;};
		const int AutoVibratoRate(){return m_AutoVibratoRate;};

		void AutoVibratoType(const int value){m_AutoVibratoType = value ;};
		void AutoVibratoSweep(const int value){m_AutoVibratoSweep = value ;};
		void AutoVibratoDepth(const int value){m_AutoVibratoDepth = value ;};
		void AutoVibratoRate(const int value){m_AutoVibratoRate = value ;};

		const bool IsAutoVibrato(){return m_AutoVibratoDepth && m_AutoVibratoRate;};

		const bool IsEnabled(){ return m_bEnabled;};
		void IsEnabled(const bool value){ m_bEnabled = value;};

		const NotePair NoteToSample(const int note){return m_AssignNoteToSample[note];};
		void NoteToSample(const int note,const NotePair npair){m_AssignNoteToSample[note] = npair;};

		std::string& Name(){return m_Name;};
		void Name(const std::string& name) { m_Name= name; }

		WaveData& rWaveLayer(const int index){ return m_WaveLayer[index];};

		void operator= (const XMInstrument & other)
		{
			int i;
			for(i = 0;i < MAX_INSTRUMENT_SAMPLES;i++)
			{
				m_WaveLayer[i] = other.m_WaveLayer[i];
			}

			m_Loop = other.m_Loop;
			m_Lines = other.m_Lines;

			m_NNA = other.m_NNA;
			
			m_AmpEnvelope = other.m_AmpEnvelope;
			m_FilterEnvelope = other.m_FilterEnvelope;
			m_FilterCutoff = other.m_FilterCutoff;
			m_FilterResonance = other.m_FilterResonance;
			m_FilterEnvAmount = other.m_FilterEnvAmount;
			m_FilterType = other.m_FilterType;

			// Paninng
			m_InitPan = other.m_InitPan;
			m_PanEnvelope = other.m_PanEnvelope;

			// Pitch Envelope
			m_PitchEnvelope = other.m_PitchEnvelope;

			// LFO
			m_RandomPanning = other.m_RandomPanning;
			m_RandomCutoff = other.m_RandomCutoff;
			m_RandomResonance = other.m_RandomResonance;
			m_RandomSampleStart = other.m_RandomSampleStart;

			m_Name = other.m_Name;

			m_bVolumeFade = other.m_bVolumeFade;
			m_VolumeFadeSpeed = other.m_VolumeFadeSpeed;

			// Auto Vibrato
			m_AutoVibratoType = other.m_AutoVibratoType;
			m_AutoVibratoSweep = other.m_AutoVibratoSweep;
			m_AutoVibratoDepth = other.m_AutoVibratoDepth;
			m_AutoVibratoRate = other.m_AutoVibratoRate;

			// 
			m_bEnabled = other.m_bEnabled;

			for(i=0;i<NOTE_MAP_SIZE;i++)
			{
				m_AssignNoteToSample[i]=other.m_AssignNoteToSample[i];
			}
		};

	private:
		WaveData m_WaveLayer[MAX_INSTRUMENT_SAMPLES];

		bool m_Loop;
		int m_Lines;

		NewNoteAction m_NNA;
		
		Envelope m_AmpEnvelope;
		Envelope m_FilterEnvelope;
		int m_FilterCutoff;		///< Cutoff Frequency [0-127]
		int m_FilterResonance;	///< Resonance [0-127]
		int m_FilterEnvAmount;	///< EnvAmount [-128,128]
		dsp::FilterType m_FilterType;		///< Filter Type [0-4]

		// Paninng
		float m_InitPan;
		Envelope m_PanEnvelope;
		// Pitch Envelope
		Envelope m_PitchEnvelope;

		// LFO
		bool m_RandomPanning;	///< Random Panning
		bool m_RandomCutoff;	///< Random CutOff
		bool m_RandomResonance;	///< Random Resonance
		bool m_RandomSampleStart;///< Random SampleStart

		std::string m_Name;

		bool m_bVolumeFade;
		float m_VolumeFadeSpeed;

		// Auto Vibrato
		int m_AutoVibratoType;
		int m_AutoVibratoSweep;
		int m_AutoVibratoDepth;
		int m_AutoVibratoRate;

		// 
		bool m_bEnabled;

		/// Table of mapped notes to samples
		// (note number=first, sample number=second)
		///\todo: Could it be interesting to map other things like volume,panning, cutoff...?
		NotePair m_AssignNoteToSample[NOTE_MAP_SIZE];
		
	};
	}
}
