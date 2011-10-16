///\file
///\brief interface file for psycle::host::Sampler.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "Machine.hpp"
#include <psycle/helpers/filter.hpp>
namespace psycle
{
	namespace host
	{
		class CGearTracker; // forward declaration

		#define SAMPLER_MAX_POLYPHONY		16
		#define SAMPLER_DEFAULT_POLYPHONY	8

		#define SAMPLER_CMD_NONE			0x00
		#define SAMPLER_CMD_PORTAUP			0x01
		#define SAMPLER_CMD_PORTADOWN		0x02
		#define SAMPLER_CMD_PORTA2NOTE		0x03
		#define SAMPLER_CMD_PANNING			0x08
		#define SAMPLER_CMD_OFFSET			0x09
		#define SAMPLER_CMD_VOLUME			0x0c
		#define SAMPLER_CMD_RETRIG			0x15
		#define SAMPLER_CMD_EXTENDED		0x0e
		#define SAMPLER_CMD_EXT_NOTEOFF		0xc0
		#define SAMPLER_CMD_EXT_NOTEDELAY	0xd0

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
			ULARGE_INTEGER _pos;
			signed __int64 _speed;
			bool _loop;
			ULONG _loopStart;
			ULONG _loopEnd;
			ULONG _length;
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
			friend CGearTracker;
		public:
			Sampler(int index);
			virtual void Init(void);
			virtual void Tick();
			virtual void Tick(int channel, PatternEntry* pData);
			virtual void Stop(void);
			virtual int GenerateAudioInTicks(int startSample,  int numSamples);
			virtual bool Load(RiffFile* pFile);
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
			virtual void SaveSpecificChunk(RiffFile* pFile);
			virtual char* GetName(void) { return _psName; }
			virtual float GetAudioRange() const { return 32768.0f; }
			virtual void SetSampleRate(int sr);
			virtual bool NeedsAuxColumn() { return true; }
			virtual const char* AuxColumnName(int idx);
			virtual int NumAuxColumnIndexes() { return MAX_INSTRUMENTS-1; } // last instrument is the preview instrument
			virtual bool playsTrack(const int track) const;

			void StopInstrument(int insIdx);
			void DefaultC4(bool correct) {
				baseC = correct? 60 : 48;
			}
			bool isDefaultC4() {
				return baseC == 60;
			}

		protected:
			static inline int alteRand(int x)
			{
				return (x*rand())/32768;
			}
			void PerformFx(int voice);
			void VoiceWork(int numsamples, int voice);
			void NoteOff(int voice);
			void NoteOffFast(int voice);
			int VoiceTick(int channel, PatternEntry* pData);
			inline void TickEnvelope(int voice);
			inline void TickFilterEnvelope(int voice);

			static char* _psName;
			unsigned char lastInstrument[MAX_TRACKS];
			int _numVoices;
			Voice _voices[SAMPLER_MAX_POLYPHONY];
			psycle::helpers::dsp::cubic_resampler _resampler;
			int baseC;
		};
	}
}
