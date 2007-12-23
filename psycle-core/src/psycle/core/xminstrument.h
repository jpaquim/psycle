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
		XMInstrument();

		~XMInstrument();

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

		///////////////////////////////////////////////////////////////////////////
		//  XMInstrument::WaveData Class declaration

		class WaveData {
		public:
			/** Wave Loop Type */
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

				WaveData();

				/// Initialize
        void Init();

				/// Destructor
				~WaveData();

				//  Object Functions
				void DeleteWaveData();
				void AllocWaveData(const int iLen,const bool bStereo);
//        void Load(DeSerializer* file);  // here is a change to main psycle
//        void Save(Serializer* file);

				/// Wave Data Copy Operator
				void operator= (const WaveData& source);

			// Properties
        const std::string WaveName();
        void WaveName(std::string newname);

        uint32_t  WaveLength();
        void WaveLength (const uint32_t value);

        const float WaveGlobVolume()const;
        void WaveGlobVolume(const float value);
        const uint16_t WaveVolume();
        void WaveVolume(const uint16_t value);

        const float PanFactor();
        void PanFactor(const float value);
        bool PanEnabled();
        void PanEnabled(bool pan);

        const uint32_t WaveLoopStart();
        void WaveLoopStart(const uint32_t value);
        const uint32_t WaveLoopEnd();
        void WaveLoopEnd(const uint32_t value);
        const LoopType WaveLoopType();
        void WaveLoopType(const LoopType value);

        const uint32_t WaveSusLoopStart();
        void WaveSusLoopStart(const uint32_t value);
        const uint32_t WaveSusLoopEnd();
        void WaveSusLoopEnd(const uint32_t value);
        const LoopType WaveSusLoopType();
        void WaveSusLoopType(const LoopType value);

        const int16_t WaveTune();
        void WaveTune(const int16_t value);
        const int16_t WaveFineTune();
        void WaveFineTune(const int16_t value);
        const uint32_t WaveSampleRate();
        void WaveSampleRate(const uint32_t value);

        const bool IsWaveStereo();
        void IsWaveStereo(const bool value);

        const uint8_t VibratoType();
        const uint8_t VibratoSpeed();
        const uint8_t VibratoDepth();
        const uint8_t VibratoAttack();

        void VibratoType(const uint8_t value);
        void VibratoSpeed(const uint8_t value);
        void VibratoDepth(const uint8_t value);
        void VibratoAttack(const uint8_t value);

        const bool IsAutoVibrato();

        const signed short * pWaveDataL();
        const signed short * pWaveDataR();

        signed short WaveDataL(const uint32_t index) const ;
        signed short WaveDataR(const uint32_t index) const ;

        void WaveDataL(const uint32_t index,const signed short value);
        void WaveDataR(const uint32_t index,const signed short value);

			private:

				std::string m_WaveName;
				/// Wave length in Samples.
				uint32_t m_WaveLength;
				float m_WaveGlobVolume;
				uint16_t m_WaveDefVolume;
				uint32_t m_WaveLoopStart;
				uint32_t m_WaveLoopEnd;
				LoopType m_WaveLoopType;
				uint32_t m_WaveSusLoopStart;
				uint32_t m_WaveSusLoopEnd;
				LoopType m_WaveSusLoopType;
				/// SampleRate of the sample. \\todo . We will have to rework NotetoPeriod,PeriodtoNote and PeriodtoSpeed.
				uint32_t m_WaveSampleRate;
				int16_t m_WaveTune;
				/// [ -256 .. 256] full range = -/+ 1 seminote
				int16_t m_WaveFineTune;
				bool m_WaveStereo;
				signed short *m_pWaveDataL;
				signed short *m_pWaveDataR;
				bool m_PanEnabled;
				/// Default position for panning ( 0..1 ) 0left 1 right. Bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
				float m_PanFactor;
				uint8_t m_VibratoAttack;
				uint8_t m_VibratoSpeed;
				uint8_t m_VibratoDepth;
				uint8_t m_VibratoType;

			};// WaveData()

};


}}
#endif
