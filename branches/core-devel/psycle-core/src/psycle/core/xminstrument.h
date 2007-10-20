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
#if !defined XMINSTRUMENT_H && 0
#define XMINSTRUMENT_H

#include <utility>
#include <inttypes.h>
#include <cassert>

namespace psy { namespace core {

template<typename object_array> inline object_array * zapArray(object_array *& pointer, object_array * const new_value = 0)
{
	if(pointer) delete [] pointer;
	return pointer = new_value;
}



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
				}

				/// Destructor
				~WaveData(){
						DeleteWaveData();
				};

				//  Object Functions
				void DeleteWaveData(){
					zapArray(m_pWaveDataL);
					zapArray(m_pWaveDataR);
				}

				void AllocWaveData(const int iLen,const bool bStereo)
				{
					DeleteWaveData();
					m_pWaveDataL = new signed short[iLen];
					m_pWaveDataR = bStereo?new signed short[iLen]:NULL;
					m_WaveStereo = bStereo;
					m_WaveLength  = iLen;
				}

//        void Load(DeSerializer* file);  // here is a change to main psycle
//        void Save(Serializer* file);

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
				}


			// Properties
			const std::string WaveName(){ return m_WaveName;};
			void WaveName(std::string newname){ m_WaveName = newname;};

			uint32_t  WaveLength(){ return m_WaveLength;};
			void WaveLength (const uint32_t value){m_WaveLength = value;};

			const float WaveGlobVolume()const{ return m_WaveGlobVolume;};
			void WaveGlobVolume(const float value){m_WaveGlobVolume = value;};
			const uint16_t WaveVolume(){ return m_WaveDefVolume;};
			void WaveVolume(const uint16_t value){m_WaveDefVolume = value;};

			const float PanFactor(){ return m_PanFactor;};// Default position for panning ( 0..1 ) 0left 1 right. Bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
			void PanFactor(const float value){m_PanFactor = value;};
			bool PanEnabled(){ return m_PanEnabled;};
			void PanEnabled(bool pan){ m_PanEnabled=pan;};

			const uint32_t WaveLoopStart(){ return m_WaveLoopStart;};
			void WaveLoopStart(const uint32_t value){m_WaveLoopStart = value;};
			const uint32_t WaveLoopEnd(){ return m_WaveLoopEnd;};
			void WaveLoopEnd(const uint32_t value){m_WaveLoopEnd = value;};
			const LoopType WaveLoopType(){ return m_WaveLoopType;};
			void WaveLoopType(const LoopType value){ m_WaveLoopType = value;};

			const uint32_t WaveSusLoopStart(){ return m_WaveSusLoopStart;};
			void WaveSusLoopStart(const uint32_t value){m_WaveSusLoopStart = value;};
			const uint32_t WaveSusLoopEnd(){ return m_WaveSusLoopEnd;};
			void WaveSusLoopEnd(const uint32_t value){m_WaveSusLoopEnd = value;};
			const LoopType WaveSusLoopType(){ return m_WaveSusLoopType;};
			void WaveSusLoopType(const LoopType value){ m_WaveSusLoopType = value;};

			const int16_t WaveTune(){return m_WaveTune;};
			void WaveTune(const int16_t value){m_WaveTune = value;};
			const int16_t WaveFineTune(){return m_WaveFineTune;};
			void WaveFineTune(const int16_t value){m_WaveFineTune = value;};
			const uint32_t WaveSampleRate(){return m_WaveSampleRate;};
			void WaveSampleRate(const uint32_t value){m_WaveSampleRate = value;};

			const bool IsWaveStereo(){ return m_WaveStereo;};
			void IsWaveStereo(const bool value){ m_WaveStereo = value;};

			const uint8_t VibratoType(){return m_VibratoType;};
			const uint8_t VibratoSpeed(){return m_VibratoSpeed;};
			const uint8_t VibratoDepth(){return m_VibratoDepth;};
			const uint8_t VibratoAttack(){return m_VibratoAttack;};

			void VibratoType(const uint8_t value){m_VibratoType = value ;};
			void VibratoSpeed(const uint8_t value){m_VibratoSpeed = value ;};
			void VibratoDepth(const uint8_t value){m_VibratoDepth = value ;};
			void VibratoAttack(const uint8_t value){m_VibratoAttack = value ;};

			const bool IsAutoVibrato(){return m_VibratoDepth && m_VibratoSpeed;};

			const signed short * pWaveDataL(){ return m_pWaveDataL;};
			const signed short * pWaveDataR(){ return m_pWaveDataR;};

			signed short WaveDataL(const uint32_t index) const { assert(index<m_WaveLength); return (*(m_pWaveDataL + index));};
			signed short WaveDataR(const uint32_t index) const { assert(index<m_WaveLength); return (*(m_pWaveDataR + index));};

			void WaveDataL(const uint32_t index,const signed short value){ assert(index<m_WaveLength); *(m_pWaveDataL + index) = value;};
			void WaveDataR(const uint32_t index,const signed short value){ assert(index<m_WaveLength); *(m_pWaveDataR + index) = value;};

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
