//\file
//\brief implementation file for psycle::host::InstPreview.

#include <psycle/host/detail/project.private.hpp>
#include "InstPreview.hpp"
#include "Instrument.hpp"
namespace psycle {
	namespace host {

		void InstPreview::Work(float *pInSamplesL, float *pInSamplesR, int numSamples)
		{
			if(m_pwave.WaveLength() == 0) return;

			float *pSamplesL = pInSamplesL;
			float *pSamplesR = pInSamplesR;
			--pSamplesL;
			--pSamplesR;
			
			short *wl = m_pwave.pWaveDataL();
			short *wr = m_pwave.pWaveDataR();
			float ld = 0;
			float rd = 0;
			
			do
			{
				ld=(*(wl+m_pos))*m_vol;
				if(m_pwave.IsWaveStereo())
					rd=(*(wr+m_pos))*m_vol;
				else
					rd=ld;
					
				*(++pSamplesL)+=ld;
				*(++pSamplesR)+=rd;
					
				if(++m_pos>=m_pwave.WaveLength())
				{
					Stop();
					return;
				}
				if(m_bLoop && m_pwave.WaveLoopEnd() == m_pos)
				{
					m_pos=m_pwave.WaveLoopStart();
				}
				
			} while(--numSamples);
		}

		void InstPreview::Play(unsigned long startPos/*=0*/)
		{
			if(m_pwave.WaveLength() == 0) return;

			m_bLoop = m_pwave.WaveLoopType() == XMInstrument::WaveData::LoopType::NORMAL;
			if(startPos < m_pwave.WaveLength())
			{
				m_bPlaying=true;
				m_pos=startPos;
			}
		}

		void InstPreview::Stop()
		{
			m_bPlaying=false;
		}
		
		void InstPreview::Release()
		{
			m_bLoop = false;
		}

	}   // namespace
}   // namespace

