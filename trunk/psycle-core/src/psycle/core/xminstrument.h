// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__XM_INSTRUMENT__INCLUDED
#define PSYCLE__CORE__XM_INSTRUMENT__INCLUDED
#pragma once

#include "fileio.h"
#include <psycle/helpers/filter.hpp>
#include "constants.h"

#include <utility>
#include <string>
#include <cstring>
#include <cassert>
#include <cstdint>

namespace psy { namespace core {

class PSYCLE__CORE__DECL XMInstrument {
public:
	/// Size of the Instrument's note mapping.
	static const int NOTE_MAP_SIZE = 120; // C-0 .. B-9
	/// A Note pair (note number=first, and sample number=second)
	typedef std::pair<unsigned char,unsigned char> NotePair;

	/// When a new note comes to play in a channel, and there is still one playing in it,
	/// do this on the currently playing note:
	struct NewNoteAction {
		enum Type {
			STOP = 0,///< [Note Cut] (This one actually does a very fast fadeout)
			CONTINUE = 1, ///< [Ignore]
			NOTEOFF = 2, ///< [Note off]
			FADEOUT = 3 ///< [Note fade]
		};
	};

	/// In some cases, the default NNA is not adequate. This option lets choose one type of element
	/// that, if it is equal than the currently playing, will apply the DCAction intead.
	/// A common example is using NNA NOTEOFF, DCT_NOTE and DCA_STOP
	struct DCType {
		enum Type {
			DCT_NONE = 0,
			DCT_NOTE,
			DCT_SAMPLE,
			DCT_INSTRUMENT
		};
	};
/*
	Using NewNoteAction instead so that we can convert easily from DCA to NNA.
	struct DCAction {
		enum Type {
			DCA_STOP = 0,
			DCA_NOTEOFF,
			DCA_FADEOUT
		};
	};
*/

//////////////////////////////////////////////////////////////////////////
//  XMInstrument::WaveData Class declaration

	class WaveData {
	public:
		/// Wave Loop Types
		struct LoopType {
			enum Type {
				DO_NOT = 0, ///< Do Nothing
				NORMAL = 1, ///< normal Start --> End ,Start --> End ...
				BIDI = 2 ///< bidirectional Start --> End, End --> Start ...
			};
		};

		/// Wave Form Types
		struct WaveForms {
			enum Type {
				SINUS = 0,
				SQUARE = 1,
				SAWUP = 2,
				SAWDOWN = 3,
				RANDOM = 4
			};
		};

		/// Constructor
		WaveData() : m_pWaveDataL(0), m_pWaveDataR(0),m_WaveLength(0) {}

		/// Initialize
		void Init(){
			DeleteWaveData();
			m_WaveName= "";
			m_WaveLength = 0;
			m_WaveGlobVolume = 1.0f; // Global volume ( global multiplier )
			m_WaveDefVolume = 128; // Default volume ( volume at which it starts to play. corresponds to 0Cxx/volume command )
			m_WaveLoopStart = 0;
			m_WaveLoopEnd = 0;
			m_WaveLoopType = LoopType::DO_NOT;
			m_WaveSusLoopStart = 0;
			m_WaveSusLoopEnd = 0;
			m_WaveSusLoopType = LoopType::DO_NOT;
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
		}

		/// Destructor
		~WaveData(){
			DeleteWaveData();
		}

		// Object Functions

		void DeleteWaveData(){
			if ( m_pWaveDataL)
			{
				delete[] m_pWaveDataL;
				m_pWaveDataL=0;
				if (m_WaveStereo)
				{
					delete[] m_pWaveDataR;
					m_pWaveDataR=0;
				}
			}
			m_WaveLength = 0;
		}

		void AllocWaveData(const int iLen,const bool bStereo)
		{
			DeleteWaveData();
			m_pWaveDataL = new std::int16_t[iLen];
			m_pWaveDataR = bStereo?new std::int16_t[iLen]:NULL;
			m_WaveStereo = bStereo;
			m_WaveLength  = iLen;
		}

		int Load(RiffFile& riffFile);
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

			std::memcpy(m_pWaveDataL, source.m_pWaveDataL, source.m_WaveLength * sizeof *m_pWaveDataL);
			if(source.m_WaveStereo)
				std::memcpy(m_pWaveDataR, source.m_pWaveDataR, source.m_WaveLength * sizeof *m_pWaveDataR);
		}


		// Properties

		const std::string WaveName()const { return m_WaveName;}
		void WaveName(const std::string newname){ m_WaveName = newname;}

		std::uint32_t WaveLength() const { return m_WaveLength;}
		void WaveLength (const std::uint32_t value){m_WaveLength = value;}

		float WaveGlobVolume() const { return m_WaveGlobVolume;}
		void WaveGlobVolume(const float value) {m_WaveGlobVolume = value;}
		std::uint16_t WaveVolume() const { return m_WaveDefVolume;}
		void WaveVolume(const std::uint16_t value){m_WaveDefVolume = value;}

		/// Default position for panning ( 0..1 ) 0left 1 right. Bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
		float PanFactor() const { return m_PanFactor;}
		void PanFactor(const float value){m_PanFactor = value;}
		bool PanEnabled() const { return m_PanEnabled;}
		void PanEnabled(const bool pan){ m_PanEnabled=pan;}

		std::uint32_t WaveLoopStart() const { return m_WaveLoopStart;}
		void WaveLoopStart(const std::uint32_t value){m_WaveLoopStart = value;}
		std::uint32_t WaveLoopEnd() const { return m_WaveLoopEnd;}
		void WaveLoopEnd(const std::uint32_t value){m_WaveLoopEnd = value;}
		LoopType::Type WaveLoopType() const { return m_WaveLoopType;}
		void WaveLoopType(const LoopType::Type value){ m_WaveLoopType = value;}

		std::uint32_t WaveSusLoopStart() const { return m_WaveSusLoopStart;}
		void WaveSusLoopStart(const std::uint32_t value){m_WaveSusLoopStart = value;}
		std::uint32_t WaveSusLoopEnd() const { return m_WaveSusLoopEnd;}
		void WaveSusLoopEnd(const std::uint32_t value){m_WaveSusLoopEnd = value;}
		LoopType::Type WaveSusLoopType() const { return m_WaveSusLoopType;}
		void WaveSusLoopType(const LoopType::Type value){ m_WaveSusLoopType = value;}

		std::int16_t WaveTune() const {return m_WaveTune;}
		void WaveTune(const std::int16_t value){m_WaveTune = value;}
		std::int16_t WaveFineTune() const {return m_WaveFineTune;}
		void WaveFineTune(const std::int16_t value){m_WaveFineTune = value;}
		std::uint32_t WaveSampleRate() const {return m_WaveSampleRate;}
		void WaveSampleRate(const std::uint32_t value);

		bool IsWaveStereo() const { return m_WaveStereo;}
		void IsWaveStereo(const bool value){ m_WaveStereo = value;}

		std::uint8_t VibratoType() const {return m_VibratoType;}
		std::uint8_t VibratoSpeed() const {return m_VibratoSpeed;}
		std::uint8_t VibratoDepth() const {return m_VibratoDepth;}
		std::uint8_t VibratoAttack() const {return m_VibratoAttack;}

		void VibratoType(const std::uint8_t value){m_VibratoType = value ;}
		void VibratoSpeed(const std::uint8_t value){m_VibratoSpeed = value ;}
		void VibratoDepth(const std::uint8_t value){m_VibratoDepth = value ;}
		void VibratoAttack(const std::uint8_t value){m_VibratoAttack = value ;}

		bool IsAutoVibrato() const {return m_VibratoDepth && m_VibratoSpeed;}

		std::int16_t * const pWaveDataL() const { return m_pWaveDataL;}
		std::int16_t * const pWaveDataR() const { return m_pWaveDataR;}

		std::int16_t WaveDataL(const std::int32_t index) const { return *(m_pWaveDataL + index); }
		std::int16_t WaveDataR(const std::int32_t index) const { return *(m_pWaveDataR + index); }

		void WaveDataL(const int index,const std::int16_t value){ *(m_pWaveDataL + index) = value;}
		void WaveDataR(const int index,const std::int16_t value){ *(m_pWaveDataR + index) = value;}

	private:

		std::string m_WaveName;
		/// Wave length in Samples.
		std::uint32_t m_WaveLength;
		float m_WaveGlobVolume;
		std::uint16_t m_WaveDefVolume;
		std::uint32_t m_WaveLoopStart;
		std::uint32_t m_WaveLoopEnd;
		LoopType::Type m_WaveLoopType;
		std::uint32_t m_WaveSusLoopStart;
		std::uint32_t m_WaveSusLoopEnd;
		LoopType::Type m_WaveSusLoopType;
		std::uint32_t m_WaveSampleRate;
		std::int16_t m_WaveTune;
		/// [ -256 .. 256] full range = -/+ 1 seminote
		std::int16_t m_WaveFineTune;
		bool m_WaveStereo;
		std::int16_t *m_pWaveDataL;
		std::int16_t *m_pWaveDataR;
		bool m_PanEnabled;
		/// Default position for panning ( 0..1 ) 0left 1 right
		float m_PanFactor;
		std::uint8_t m_VibratoAttack;
		std::uint8_t m_VibratoSpeed;
		std::uint8_t m_VibratoDepth;
		std::uint8_t m_VibratoType;

	};// WaveData


//////////////////////////////////////////////////////////////////////////
//  XMInstrument::Envelope Class declaration

	class Envelope {
	public:
		/// Invalid point. Used to indicate that sustain/normal loop is disabled.
		static const int INVALID = -1;

		/// ValueType is a float value from  0 to 1.0  (or -1.0 1.0, or whatever else) which can be used as a multiplier.
		typedef float ValueType;

		/// The meaning of the first value (int), is time, and the unit depends on the context.
		typedef std::pair<int,ValueType> PointValue;

		/// ?
		typedef std::vector< PointValue > Points;

		/// constructor
		explicit Envelope() {
			Init();
		}

		/// copy Constructor
		Envelope(const Envelope& other)
		{
			operator=(other);
		}

		~Envelope(){}

		/// Init
		void Init() {
			m_Enabled = false;
			m_Carry = false;
			m_SustainBegin = INVALID;
			m_SustainEnd = INVALID;
			m_LoopStart = INVALID;
			m_LoopEnd = INVALID;
			m_Points.clear();
		}

		// Object Functions.

		/// Gets the time at which the pointIndex point is located.
		int GetTime(const unsigned int pointIndex) const
		{
			if(pointIndex >= 0 && pointIndex < m_Points.size()) return m_Points[pointIndex].first;
			return INVALID;
		}
		/// Sets a new time for an existing pointIndex point.
		int SetTime(const unsigned int pointIndex,const int pointTime)
		{
			assert(pointIndex >= 0 && pointIndex < (int)m_Points.size());
			m_Points[pointIndex].first = pointTime;
			return SetTimeAndValue(pointIndex,pointTime,m_Points[pointIndex].second);
		}
		/// Gets the value of the pointIndex point.
		ValueType GetValue(const unsigned int pointIndex) const
		{
			assert(pointIndex >= 0 && pointIndex < (int)m_Points.size());
			return m_Points[pointIndex].second;
		}
		/// Sets the value pointVal to pointIndex point.
		void SetValue(const unsigned int pointIndex,const ValueType pointVal)
		{
			assert(pointIndex >= 0 && pointIndex < (int)m_Points.size());
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
		}

		/// Helper to set a new time for an existing index.
		int SetTimeAndValue(const unsigned int pointIndex,const int pointTime,const ValueType pointVal);

		/// Inserts a new point to the points Array.
		unsigned int Insert(const int pointIndex,const ValueType pointVal);

		/// Removes a point from the points Array.
		void Delete(const unsigned int pointIndex);

		/// Clears the points Array
		void Clear()
		{
			m_Points.clear();
		}
		/// Set or Get the point Index for Sustain and Loop.
		int SustainBegin() const { return m_SustainBegin;}
		/// value has to be an existing point!
		void SustainBegin(const unsigned int value){m_SustainBegin = value;}

		int SustainEnd() const { return m_SustainEnd;}
		/// value has to be an existing point!
		void SustainEnd(const unsigned int value){m_SustainEnd = value;}

		int LoopStart() const {return m_LoopStart;}
		/// value has to be an existing point!
		void LoopStart(const unsigned int value){m_LoopStart = value;}

		int LoopEnd() const {return m_LoopEnd;}
		/// value has to be an existing point!
		void LoopEnd(const unsigned int value){m_LoopEnd = value;}

		unsigned int NumOfPoints() const { return m_Points.size();}

		void Load(RiffFile& riffFile,const std::uint32_t version);
		void Save(RiffFile& riffFile,const std::uint32_t version);

		/// overloaded copy function
		Envelope& operator=(const Envelope& other)
		{
			if(this == &other) return *this;

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

		//// If the envelope IsEnabled, it is used and triggered. Else, it is not.
		bool IsEnabled() const { return m_Enabled;}
		void IsEnabled(const bool value){ m_Enabled = value;}

		/// if IsCarry() and a new note enters, the envelope position is set to
		/// that of the previous note *on the same channel*
		/// \todo implement carry
		bool IsCarry() const { return m_Carry;}
		void IsCarry(const bool value){ m_Carry = value;}

	private:
		/// Envelope is enabled or disabled
		bool m_Enabled;
		/// ????
		bool m_Carry;
		/// Array of Points of the envelope.
		/// first : time at which to set the value. Unit can be different things depending on the context.
		/// second : 0 .. 1.0f . (or -1.0 1.0 or whatever else) Use it as a multiplier.
		Points m_Points;
		/// Loop Start Point
		int m_LoopStart;
		/// Loop End Point
		int m_LoopEnd;
		/// Sustain Start Point
		int m_SustainBegin;
		/// Sustain End Point
		int m_SustainEnd;
	};// class Envelope


//////////////////////////////////////////////////////////////////////////
//  XMInstrument Class declaration
	XMInstrument();
	~XMInstrument();

	void Init();

	int Load(RiffFile& riffFile);
	void Save(RiffFile& riffFile);

	void operator= (const XMInstrument & other)
	{
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
		for(int i=0;i<NOTE_MAP_SIZE;i++)
		{
			m_AssignNoteToSample[i]=other.m_AssignNoteToSample[i];
		}
	}

	// Properties

	/// IsEnabled() is used on Saving, to not store unused instruments
	bool IsEnabled() const { return m_bEnabled;}
	void IsEnabled(const bool value){ m_bEnabled = value;}

	const std::string& Name() const {return m_Name;}
	void Name(const std::string& name) { m_Name= name; }

	std::uint16_t Lines() const { return m_Lines;}
	void Lines(const std::uint16_t value){ m_Lines = value;}

	const Envelope* AmpEnvelope() const { return &m_AmpEnvelope;}
	const Envelope* PanEnvelope() const {return &m_PanEnvelope;}
	const Envelope* FilterEnvelope() const { return &m_FilterEnvelope;}
	const Envelope* PitchEnvelope() const {return &m_PitchEnvelope;}

	Envelope* AmpEnvelope() { return &m_AmpEnvelope;}
	Envelope* PanEnvelope() {return &m_PanEnvelope;}
	Envelope* FilterEnvelope() { return &m_FilterEnvelope;}
	Envelope* PitchEnvelope() {return &m_PitchEnvelope;}

	float GlobVol() const { return m_GlobVol;}
	void GlobVol(const float value){m_GlobVol = value;}
	float VolumeFadeSpeed() const { return m_VolumeFadeSpeed;}
	void VolumeFadeSpeed(const float value){ m_VolumeFadeSpeed = value;}

	/// Default position for panning ( 0..1 ) 0left 1 right. Bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
	float Pan() const { return m_InitPan;}
	void Pan(const float pan) { m_InitPan = pan;}
	bool PanEnabled() const { return m_PanEnabled;}
	void PanEnabled(const bool pan) { m_PanEnabled = pan;}
	std::uint8_t NoteModPanCenter() const { return m_NoteModPanCenter;}
	void NoteModPanCenter(const std::uint8_t pan) { m_NoteModPanCenter = pan;}
	std::int8_t NoteModPanSep() const { return m_NoteModPanSep;}
	void NoteModPanSep(const std::int8_t pan) { m_NoteModPanSep = pan;}

	std::uint8_t FilterCutoff() const { return m_FilterCutoff;}
	void FilterCutoff(const std::uint8_t value){m_FilterCutoff = value;}
	std::uint8_t FilterResonance() const { return m_FilterResonance;}
	void FilterResonance(const std::uint8_t value){m_FilterResonance = value;}
	std::int16_t FilterEnvAmount() const { return m_FilterEnvAmount;}
	void FilterEnvAmount(const std::int16_t value){ m_FilterEnvAmount = value;}
	psycle::helpers::dsp::FilterType FilterType() const { return m_FilterType;}
	void FilterType(const psycle::helpers::dsp::FilterType value){ m_FilterType = value;}

	float RandomVolume() const {return  m_RandomVolume;}
	void RandomVolume(const float value){m_RandomVolume = value;}
	float RandomPanning() const {return  m_RandomPanning;}
	void RandomPanning(const float value){m_RandomPanning = value;}
	float RandomCutoff() const {return m_RandomCutoff;}
	void RandomCutoff(const float value){m_RandomCutoff = value;}
	float RandomResonance() const {return m_RandomResonance;}
	void RandomResonance(const float value){m_RandomResonance = value;}

	NewNoteAction::Type NNA() const { return m_NNA;}
	void NNA(const NewNoteAction::Type value){ m_NNA = value;}
	DCType::Type DCT() const { return m_DCT;}
	void DCT(const DCType::Type value){ m_DCT = value;}
	NewNoteAction::Type DCA() const { return m_DCA;}
	void DCA(const NewNoteAction::Type value){ m_DCA = value;}

	const NotePair NoteToSample(const int note) const {return m_AssignNoteToSample[note];}
	void NoteToSample(const int note,const NotePair npair){m_AssignNoteToSample[note] = npair;}

private:
	bool m_bEnabled;

	std::string m_Name;

	/// If m_Lines > 0 use samplelen/(tickduration*m_Lines) to determine the wave speed instead of the note.
	std::uint16_t m_Lines;

	/// envelope range = [0.0f..1.0f]
	Envelope m_AmpEnvelope;
	/// envelope range = [-1.0f..1.0f]
	Envelope m_PanEnvelope;
	/// envelope range = [-1.0f..1.0f]
	Envelope m_PitchEnvelope;
	/// envelope range = [0.0f..1.0f]
	Envelope m_FilterEnvelope;

	/// [0..1.0f] Global volume affecting all samples of the instrument.
	float m_GlobVol;
	/// [0..1.0f] Fadeout speed. Decreasing amount for each tracker tick.
	float m_VolumeFadeSpeed;

	// Paninng

	bool m_PanEnabled;
	/// Initial panFactor (if enabled) [-1..1]
	float m_InitPan;
	/// Note number for center pan position
	std::uint8_t m_NoteModPanCenter;
	/// -32..32. 1/256th of panFactor change per seminote.
	std::int8_t m_NoteModPanSep;

	/// Cutoff Frequency [0..127]
	std::uint8_t m_FilterCutoff;
	/// Resonance [0..127]
	std::uint8_t m_FilterResonance;
	/// EnvAmount [-128..128]
	std::int16_t m_FilterEnvAmount;
	/// Filter Type [0..4]
	psycle::helpers::dsp::FilterType m_FilterType;

	// Randomness. Applies on new notes.

	/// Random Volume % [ 0.0 -> No randomize. 1.0 = randomize full scale.]
	float m_RandomVolume;
	/// Random Panning (same)
	float m_RandomPanning;
	/// Random CutOff (same)
	float m_RandomCutoff;
	/// Random Resonance (same)
	float m_RandomResonance;

	/// Action to take on the playing voice when any new note comes in the same channel.
	NewNoteAction::Type m_NNA;
	/// ?
	DCType::Type m_DCT;
	/// Action to take on the playing voice when a new note comes in the same channel
	/// and the element defined by m_DCT is the same. (like the same note value).
	NewNoteAction::Type m_DCA;

	/// Table of mapped notes to samples
	/// (note number=first, sample number=second)
	///\todo Could it be interesting to map other things like volume,panning, cutoff...?
	NotePair m_AssignNoteToSample[NOTE_MAP_SIZE];
};

///\todo : implement the following for inter-XMSampler sharing of instruments.
class SampleList{
	public:
		SampleList(){top=0;}
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
			assert(pos<MAX_INSTRUMENTS);
			m_waves[pos]=wave;
			return pos;
		}
		XMInstrument::WaveData &operator[](int pos)
		{
			assert(pos<MAX_INSTRUMENTS);
			return m_waves[pos];
		}
	private:
		XMInstrument::WaveData m_waves[MAX_INSTRUMENTS];
		int top;
};

///\todo : implement the following for inter-XMSampler sharing of instruments.
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
			assert(pos<MAX_INSTRUMENTS);
			m_inst[pos]=inst;
			return pos;
		}
		XMInstrument &operator[](int pos){
			assert(pos<MAX_INSTRUMENTS);
			return m_inst[pos];
		}
	private:
		XMInstrument m_inst[MAX_INSTRUMENTS];
		int top;
};

}}
#endif
