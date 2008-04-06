// -*- mode:c++; indent-tabs-mode:t -*-
namespace psy
{
	namespace core
	{
		class Instrument;

		/// plays a sample/instrument-- used for wav preview, wav-ed play function
		class InstPreview
		{
		public:
			InstPreview() : m_vol(0.5f), m_bPlaying(false) {}
			virtual ~InstPreview() {}
			/// process data for output
			void Work(float *pInSamplesL, float *pInSamplesR, int numSamples);
			/// begin playing
			void Play(unsigned long startPos=0);
			/// stop playback
			void Stop();
			/// disable looping if enabled
			void Release();
			/// whether or not we're already playing
			bool IsEnabled() {return m_bPlaying; }
			/// whether or not we're looping
			bool IsLooping() {return m_bLoop; }

			/// get Instrument to preview
			Instrument* GetInstrument() { return m_pInstrument; }
			/// set Instrument to preview
			void        SetInstrument(Instrument *pInstrument) { m_pInstrument=pInstrument; }

			/// get playback volume
			float GetVolume() { return m_vol; }
			/// set playback volume
			void  SetVolume(float vol) { m_vol = vol;}
			/// get current playback position
			unsigned long GetPosition() {return m_pos;}
		private:
			/// pointer to the instrument associated with the preview
			Instrument* m_pInstrument;
			/// current playback position in samples
			unsigned long m_pos;
			/// whether we're currently looping
			bool m_bLoop;
			/// playback volume
			float m_vol;
			/// whether we're currently playing
			bool m_bPlaying;
		};

}}
