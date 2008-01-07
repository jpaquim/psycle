///\file
///\brief interface file for psy::core::Sampler. based on psyclemfc revision 5903
#pragma once

#include "cstdint.h"
#include "dsp.h"
#include "filter.h"
#include "instrument.h"
#include "instpreview.h"
#include "machine.h"

namespace psy {
	namespace core {

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

		typedef enum
		{
			ENV_OFF = 0,
			ENV_ATTACK = 1,
			ENV_DECAY = 2,
			ENV_SUSTAIN = 3,
			ENV_RELEASE = 4,
			ENV_FASTRELEASE = 5
		}
		EnvelopeStage;

		class WaveData
		{
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

		class Envelope
		{
		public:
			EnvelopeStage _stage;
			float _value;
			float _step;
			float _attack;
			float _decay;
			float _sustain;
			float _release;
		};

		class Voice
		{
		public:
			Envelope _filterEnv;
			Envelope _envelope;
			int _sampleCounter;
			int _triggerNoteOff;
			int _triggerNoteDelay;
			int _instrument;
			WaveData _wave;
			dsp::Filter _filter;
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
		class Sampler : public Machine
		{
		public:
			void Tick( );
			Sampler(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song);
			virtual void Init();
			virtual int GenerateAudioInTicks( int startSample, int numSamples );
			virtual void SetSampleRate(int sr)
			{
				Machine::SetSampleRate(sr);
				for (int i=0; i<_numVoices; i++)
				{
					_voices[i]._envelope._stage = ENV_OFF;
					_voices[i]._envelope._sustain = 0;
					_voices[i]._filterEnv._stage = ENV_OFF;
					_voices[i]._filterEnv._sustain = 0;
					_voices[i]._filter.Init(sr);
					_voices[i]._triggerNoteOff = 0;
					_voices[i]._triggerNoteDelay = 0;
				}
			}
			virtual void Stop();
			virtual void Tick(int channel, const PatternEvent & data );
			virtual std::string GetName() const { return _psName; }
			/// Loader for psycle fileformat version 2.
			virtual bool LoadPsy2FileFormat(std::string const & plugin_path, RiffFile* pFile);
			inline virtual bool LoadSpecificChunk(RiffFile* pFile, int version)
			{
				std::uint32_t size;
				pFile->Read(size);
				if (size)
				{
					if (version > CURRENT_FILE_VERSION_MACD)
					{
						// data is from a newer format of psycle, it might be unsafe to load.
						pFile->Skip(size);
						return false;
					}
					else
					{
						std::int32_t temp;
						pFile->Read(temp); // numSubtracks
						_numVoices=temp;
						pFile->Read(temp); // quality

						switch (temp)
						{
						case 2:
							_resampler.SetQuality(dsp::R_SPLINE);
							break;
						case 3:
							_resampler.SetQuality(dsp::R_BANDLIM);
							break;
						case 0:
							_resampler.SetQuality(dsp::R_NONE);
							break;
						default:
						case 1:
							_resampler.SetQuality(dsp::R_LINEAR);
							break;
						}
					}
				}
				return true;
			}

			inline virtual void SaveSpecificChunk(RiffFile* pFile) const
			{
				std::int32_t temp;
				std::uint32_t size = 2 * sizeof temp;
				pFile->Write(size);
				temp = _numVoices;
				pFile->Write(temp); // numSubtracks
				switch (_resampler.GetQuality())
				{
					case dsp::R_NONE:
						temp = 0;
						break;
					case dsp::R_LINEAR:
						temp = 1;
						break;
					case dsp::R_SPLINE:
						temp = 2;
						break;
					case dsp::R_BANDLIM:
						temp = 3;
						break;
				}
				pFile->Write(temp); // quality
			}

			void Update();

			///\name wave file previewing
			///\todo shouldn't belong to the song class. Move it to Sampler.
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
			int _numVoices;
			Voice _voices[SAMPLER_MAX_POLYPHONY];
			dsp::Cubic _resampler;

			void PerformFx(int voice);
			void VoiceWork( int startSample, int numsamples, int voice );
			void NoteOff( int voice );
			void NoteOffFast(int voice );
			int VoiceTick( int channel, const PatternEvent & pEntry );
			inline void TickEnvelope( int voice );
			inline void TickFilterEnvelope( int voice );
			Instrument::id_type lastInstrument[MAX_TRACKS];
			static inline int alteRand(int x)
			{
				return (x*rand())/RAND_MAX;
			}
		};
	}
}
