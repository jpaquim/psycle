// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
	*   Copyright (C) 2007 by Psycledelics     *
	*   psycle.sf.net   *
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#ifndef XMINSTRUMENT_H
#define XMINSTRUMENT_H

#include <utility>
#include <cstdint>
#include <cstdio>
#include <string>

namespace psy { namespace core {

/**
@author Psycledelics  
*/
class XMInstrument{
	public:
		/// Size of the Instrument's note mapping.
		static const int NOTE_MAP_SIZE = 120; // C-0 .. B-9
		/// A Note pair (note number=first, and sample number=second)
		typedef std::pair<unsigned char,unsigned char> NotePair;

		/// When a note starts to play in a channel, and there is still a note playing in it,
		/// do this on the currently playing note:
		enum NewNoteAction {
				STOP = 0x0,     ///< note cut. This one actually does a very fast fadeout.
				CONTINUE = 0x1, ///< ignore
				NOTEOFF = 0x2,  ///< note off
				FADEOUT = 0x3   ///< note fade
		};
		enum DCType {
				DCT_NONE=0x0,
				DCT_NOTE,
				DCT_SAMPLE,
				DCT_INSTRUMENT
		};

		/// a sample
		class WaveData {
			public:
				/// Wave Loop Type
				enum LoopType {
					DO_NOT = 0x0, ///< Do Nothing
					NORMAL = 0x1, ///< normal Start --> End ,Start --> End ...
					BIDI = 0x2    ///< bidirectional Start --> End, End --> Start ...
				};

				enum WaveForms {
					SINUS = 0x0,
					SQUARE = 0x1,
					SAWUP = 0x2,
					SAWDOWN = 0x3,
					RANDOM = 0x4
				};

				/// default constructor
				WaveData();

				/// constructor (with full OO semantics)
				WaveData(unsigned int samples, bool bStereo);

				/// copy constructor
				WaveData(WaveData const & source);

				/// copy operator
				WaveData const & operator=(WaveData const & source);

				/// reinitialisation (destroys the wave data)
				///\todo make it private?
				void Init();

				/// allocates memory for wave data
				void AllocWaveData(unsigned int samples, bool bStereo);

				/// deallocates the wave data
				void DeleteWaveData();

				/// destructor
				~WaveData();

				//void Load(DeSerializer* file);  // here is a change to main psycle
				//void Save(Serializer* file);

			///\name name of the wave
			///\{
				public:
					std::string const & WaveName() const { return m_WaveName; }
					void WaveName(std::string const & newname){ m_WaveName = newname; }
				private:
					std::string m_WaveName;
			///\}

			///\name length in samples
			///\{
				public:
					std::uint32_t  WaveLength() const { return m_WaveLength; }
					void WaveLength(std::uint32_t value){ m_WaveLength = value; }
				private:
					std::uint32_t m_WaveLength;
			///\}

			///\name global volume (global multiplier)
			///\{
				public:
					float WaveGlobVolume() const { return m_WaveGlobVolume; }
					void WaveGlobVolume(float value){ m_WaveGlobVolume = value; }
				private:
					float m_WaveGlobVolume;
			///\}

			///\name default volume.
			/// volume at which it starts to play. corresponds to 0Cxx/volume command
			///\{
				public:
					std::uint16_t WaveVolume() const { return m_WaveDefVolume; }
					void WaveVolume(std::uint16_t value) { m_WaveDefVolume = value; }
				private:
					std::uint16_t m_WaveDefVolume;
			///\}

			///\name default position for panning [0, 1].
			/// 0 for left, 1 for right, bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
			///\{
				public:
					float PanFactor() const { return m_PanFactor; }
					void PanFactor(float value) { m_PanFactor = value; }
				private:
					float m_PanFactor;
			///\}

			///\name  whether panning is enabled
			///\{
				public:
					bool PanEnabled() const { return m_PanEnabled; }
					void PanEnabled(bool pan) { m_PanEnabled=pan; }
				private:
					bool m_PanEnabled;
			///\}

			///\name loop start point
			///\{
				public:
					std::uint32_t WaveLoopStart() const { return m_WaveLoopStart; }
					void WaveLoopStart(std::uint32_t value) { m_WaveLoopStart = value; }
				private:
					std::uint32_t m_WaveLoopStart;
			///\}

			///\name loop end point
			///\{
				public:
					std::uint32_t WaveLoopEnd() const { return m_WaveLoopEnd; }
					void WaveLoopEnd(std::uint32_t value) { m_WaveLoopEnd = value; }
				private:
					std::uint32_t m_WaveLoopEnd;
			///\}

			///\name looping mode
			///\{
				public:
					LoopType WaveLoopType() const { return m_WaveLoopType; }
					void WaveLoopType(LoopType value) { m_WaveLoopType = value; }
				private:
					LoopType m_WaveLoopType;
			///\}

			///\name loop start point during sustain period
			///\{
				public:
					std::uint32_t WaveSusLoopStart() const { return m_WaveSusLoopStart; }
					void WaveSusLoopStart(std::uint32_t value){ m_WaveSusLoopStart = value; }
				private:
					std::uint32_t m_WaveSusLoopStart;
			///\}

			///\name loop end point during sustain period
			///\{
				public:
					std::uint32_t WaveSusLoopEnd() const { return m_WaveSusLoopEnd; }
					void WaveSusLoopEnd(std::uint32_t value) { m_WaveSusLoopEnd = value; }
				private:
					std::uint32_t m_WaveSusLoopEnd;
			///\}

			///\name looping mode during sustain period
			///\{
				public:
					LoopType WaveSusLoopType() const { return m_WaveSusLoopType; }
					void WaveSusLoopType(LoopType value) { m_WaveSusLoopType = value; }
				private:
					LoopType m_WaveSusLoopType;
			///\}

			///\name harsh tuning
			///\{
				public:
					std::int16_t WaveTune() const { return m_WaveTune; }
					void WaveTune(std::int16_t value);
				private:
					std::int16_t m_WaveTune;
			///\}

			///\name fine tuning
			/// [-256, +256] full range = -/+ 1 semitone
			///\{
				public:
					std::int16_t WaveFineTune() const { return m_WaveFineTune; }
					void WaveFineTune(std::int16_t value) { m_WaveFineTune = value; }
				private:
					std::int16_t m_WaveFineTune;
			///\}

			///\name sample rate
			///\todo Add SampleRate functionality, and change WaveTune's one.
			/// This means modifying the functions PeriodToSpeed (for linear slides) and NoteToPeriod (for amiga slides),
			/// and PeriodtoNote.
			///\{
				public:
					std::uint32_t WaveSampleRate() const { return m_WaveSampleRate; }
					void WaveSampleRate(std::uint32_t value) { m_WaveSampleRate = value; }
				private:
					std::uint32_t m_WaveSampleRate;
			///\}

			///\name whether the wave has stereo data (two channels)
			///\{
				public:
					bool IsWaveStereo() const { return m_WaveStereo; }
					void IsWaveStereo(bool value) { m_WaveStereo = value; }
				private:
					bool m_WaveStereo;
			///\}

			///\name vibrato type
			///\{
				public:
					std::uint8_t VibratoType() const { return m_VibratoType; }
					void VibratoType(std::uint8_t value) { m_VibratoType = value; }
				private:
					std::uint8_t m_VibratoType;
			///\}

			///\name vibrato speed
			///\{
				public:
					std::uint8_t VibratoSpeed() const { return m_VibratoSpeed; }
					void VibratoSpeed(std::uint8_t value) { m_VibratoSpeed = value; }
				private:
					std::uint8_t m_VibratoSpeed;
			///\}

			///\name vibrato depth
			///\{
				public:
					std::uint8_t VibratoDepth() const { return m_VibratoDepth; }
					void VibratoDepth(std::uint8_t value) { m_VibratoDepth = value; }
				private:
					std::uint8_t m_VibratoDepth;
			///\}

			///\name vibrato attack
			///\{
				public:
					std::uint8_t VibratoAttack() const { return m_VibratoAttack; }
					void VibratoAttack(std::uint8_t value) { m_VibratoAttack = value; }
				private:
					std::uint8_t m_VibratoAttack;
			///\}

			///\name whether there is a vibrato
			///\{
				public:
					bool IsAutoVibrato() const { return m_VibratoDepth && m_VibratoSpeed; } 
				private:
			///\}

			///\name left channel samples
			///\{
				public:
					std::int16_t const * pWaveDataL() { return m_pWaveDataL; }
					std::int16_t WaveDataL(std::uint32_t index) const { assert(index < m_WaveLength); return m_pWaveDataL[index]; }
					void WaveDataL(std::uint32_t index, std::int16_t value) { assert(index < m_WaveLength); m_pWaveDataL[index] = value; }
				private:
					std::int16_t * m_pWaveDataL;
			///\}

			///\name right channel samples
			///\{
				public:
					std::int16_t const * pWaveDataR() { return m_pWaveDataR; }
					std::int16_t WaveDataR(std::uint32_t index) const { assert(index < m_WaveLength); return m_pWaveDataR[index]; }
					void WaveDataR(std::uint32_t index, std::int16_t value) { assert(index < m_WaveLength); m_pWaveDataR[index] = value; }
				private:
					std::int16_t * m_pWaveDataR;
			///\}
		}; // class WaveData
}; // class XMInstrument
}}
#endif
