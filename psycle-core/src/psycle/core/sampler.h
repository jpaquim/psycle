// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psy::core::Sampler.
#ifndef PSYCLE__CORE__SAMPLER__INCLUDED
#define PSYCLE__CORE__SAMPLER__INCLUDED
#pragma once

#include <psycle/helpers/dsp.hpp>
#include <psycle/helpers/filter.hpp>
#include "instrument.h"
#include "instpreview.h"
#include "machine.h"
#include <cstdint>

namespace psy { namespace core {

	#define SAMPLER_MAX_POLYPHONY          16
	#define SAMPLER_DEFAULT_POLYPHONY      8

	#define SAMPLER_CMD_NONE               0x00
	#define SAMPLER_CMD_PORTAUP            0x01
	#define SAMPLER_CMD_PORTADOWN          0x02
	#define SAMPLER_CMD_PORTA2NOTE         0x03
	#define SAMPLER_CMD_PANNING            0x08
	#define SAMPLER_CMD_OFFSET             0x09
	#define SAMPLER_CMD_VOLUME             0x0c
	#define SAMPLER_CMD_RETRIG             0x15
	#define SAMPLER_CMD_EXTENDED           0x0e
	#define SAMPLER_CMD_EXT_NOTEOFF        0xc0
	#define SAMPLER_CMD_EXT_NOTEDELAY      0xd0

	enum EnvelopeStage {
		ENV_OFF = 0,
		ENV_ATTACK = 1,
		ENV_DECAY = 2,
		ENV_SUSTAIN = 3,
		ENV_RELEASE = 4,
		ENV_FASTRELEASE = 5
	};

	class WaveData {
		public:
			short* _pL;
			short* _pR;
			bool _stereo;
			std::int64_t _pos;
			std::int64_t _speed;
			bool _loop;
			std::uint32_t _loopStart;
			std::uint32_t _loopEnd;
			std::uint32_t _length;
			float _vol;
			float _lVolDest;
			float _rVolDest;
			float _lVolCurr;
			float _rVolCurr;
	};

	class Envelope {
		public:
			EnvelopeStage _stage;
			float _value;
			float _step;
			float _attack;
			float _decay;
			float _sustain;
			float _release;
	};

	class Voice {
		public:
			Envelope _filterEnv;
			Envelope _envelope;
			int _sampleCounter;
			int _triggerNoteOff;
			int _triggerNoteDelay;
			int _instrument;
			WaveData _wave;
			psycle::helpers::dsp::Filter _filter;
			int _cutoff;
			float _coModify;
			int _channel;
			int effVal;
			//int effPortaNote;
			int effCmd;
			int effretMode;
			int effretTicks;
			float effretVol;
			int effOld;
	};

	/// sampler.
	class PSYCLE__CORE__DECL Sampler : public Machine {
		protected:
			Sampler(MachineCallbacks* callbacks, Machine::id_type id); friend class InternalHost;

		public:
			void Tick( );

			virtual void Init();
			virtual int GenerateAudioInTicks( int startSample, int numSamples );
			virtual void SetSampleRate(int sr);
			virtual void Stop();
			virtual void Tick(int channel, const PatternEvent & data );
			virtual MachineKey getMachineKey() const { return MachineKey::sampler(); }
			virtual std::string GetName() const { return _psName; }
			/// Loader for psycle fileformat version 2.
			virtual bool LoadPsy2FileFormat(RiffFile* pFile);
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
			virtual void SaveSpecificChunk(RiffFile* pFile) const;
			void Update();

		///\name wave file previewing
		///\{
			public:
				//todo these ought to be dynamically allocated
				/// Wave preview.
				static InstPreview wavprev;
				/// Wave editor playback.
				///\todo: two previews???
				static InstPreview waved;
				/// runs the wave previewing.
				static void DoPreviews(int amount, float* pLeft, float* pRight);
		///\}

		protected:
			static std::string _psName;

		public:
			int _numVoices;
			Voice _voices[SAMPLER_MAX_POLYPHONY];
			psycle::helpers::dsp::Cubic _resampler;

			void PerformFx(int voice);
			void VoiceWork( int startSample, int numsamples, int voice );
			void NoteOff( int voice );
			void NoteOffFast(int voice );
			int VoiceTick( int channel, const PatternEvent & pEntry );
			inline void TickEnvelope( int voice );
			inline void TickFilterEnvelope( int voice );
			Instrument::id_type lastInstrument[MAX_TRACKS];
	};

}}
#endif
