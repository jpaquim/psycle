//\file
//\brief interface file for psycle::host::InstPreview
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "XMSampler.hpp"
namespace psycle {
	namespace host {

		//plays a sample/instrument-- used for wav preview, wave bank tweaking and wav-editor play function
		class InstPreview
		{
		public:
			InstPreview() : m_vol(1.0f), m_pwave(&m_prevwave) { resampler.quality(dsp::resampler::quality::spline); }
			virtual ~InstPreview() {}
			//process data for output
			void Work(float *pInSamplesL, float *pInSamplesR, int numSamples);
			//begin playing
			void Play(int note=60,unsigned long startPos=0);
			//stop playback
			void Stop(bool deallocate=false);
			//disable looping if enabled  (sustain loop!)
			void Release();
			//whether or not we're already playing
			bool IsEnabled() const { return controller.Playing(); }
			//whether or not we're looping (sustain loop!)
			bool IsLooping() const { return m_bLoop; }

			//set Instrument to preview
			void UseWave(XMInstrument::WaveData<>*  wave) {
				m_pwave = wave; 
				if(wave != NULL) {m_vol = wave->WaveGlobVolume();}
			}
			XMInstrument::WaveData<> & UsePreviewWave() {
				m_pwave = &m_prevwave;
				return m_prevwave;
			}

			//get playback volume
			float	GetVolume()		const	{ return m_vol; }
			//set playback volume
			void	SetVolume(float vol)	{ m_vol= vol;}
			//get current playback position
			unsigned long GetPosition()	const	{return controller.Position();}
		private:
			double NoteToSpeed(int note);
			psycle::helpers::dsp::cubic_resampler resampler;
			XMSampler::WaveDataController<> controller;
			//(copy of) the instrument associated with the preview
			XMInstrument::WaveData<> * m_pwave;
			XMInstrument::WaveData<> m_prevwave;
			//whether we're currently looping
			bool m_bLoop;
			float m_vol;
		};

	}   // namespace
}   // namespace
