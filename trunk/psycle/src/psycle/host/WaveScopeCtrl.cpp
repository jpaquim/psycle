#include <psycle/host/detail/project.private.hpp>
#include "WaveScopeCtrl.hpp"
namespace psycle { namespace host {

// CWaveScopeCtrl
CWaveScopeCtrl::CWaveScopeCtrl()
: m_pWave(NULL)
{
	cpen_lo.CreatePen(PS_SOLID,0,0xFF0000);
	cpen_med.CreatePen(PS_SOLID,0,0xCCCCCC);
	cpen_hi.CreatePen(PS_SOLID,0,0x00FF00);
	cpen_sus.CreatePen(PS_DOT,0,0xFF0000);
	resampler.quality(helpers::dsp::resampler::quality::spline);
}
CWaveScopeCtrl::~CWaveScopeCtrl(){
	cpen_lo.DeleteObject();
	cpen_med.DeleteObject();
	cpen_hi.DeleteObject();
	cpen_sus.DeleteObject();
}

void CWaveScopeCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if (m_pWave && lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
	{
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		int wrHeight = 0, yLow = 0, yHi = 0;
		double OffsetStep = 0;
		long c, d;

		CRect rect;
		GetClientRect(&rect);

		int const nWidth=rect.Width();
		int const nHeight=rect.Height();
		int const my=nHeight/2;
		if(rWave().IsWaveStereo()) wrHeight=my/2;
		else wrHeight=my;

		dc.FillSolidRect(&rect,RGB(255,255,255));
		dc.SetBkMode(TRANSPARENT);

		if(rWave().WaveLength())
		{

			// Draw preliminary stuff

			CPen *oldpen= dc.SelectObject(&cpen_med);

			// Left channel 0 amplitude line
			dc.MoveTo(0,wrHeight);
			dc.LineTo(nWidth,wrHeight);

			int const wrHeight_R = my + wrHeight;

			if(rWave().IsWaveStereo())
			{
				// Right channel 0 amplitude line
				dc.MoveTo(0,wrHeight_R);
				dc.LineTo(nWidth,wrHeight_R);

				// Stereo channels separator line
				dc.SelectObject(&cpen_lo);
				dc.MoveTo(0,my);
				dc.LineTo(nWidth,my);
			}

			dc.SelectObject(&cpen_hi);

			OffsetStep = (double) rWave().WaveLength() / nWidth;

			if ( OffsetStep > 4)
			{
				const std::int16_t * const pData = rWave().pWaveDataL();
				for(c = 0; c < nWidth; c++)
				{
					long const offset = (long)floorf(c * OffsetStep);
					yLow=0;yHi=0;
					// Alternate search. Doing the same than in the next "else if"
					// can be slow on big ( 100.000+ sample) samples
					for (d = offset; d < offset + OffsetStep; d+=(OffsetStep/4))
					{
						if (yLow > pData[d]) yLow = pData[d];
						if (yHi < pData[d]) yHi = pData[d];
					}
					int const ryLow = (wrHeight * yLow)/32768;
					int const ryHi = (wrHeight * yHi)/32768;
					dc.MoveTo(c,(wrHeight) - ryLow);
					dc.LineTo(c,(wrHeight) - ryHi);
				}
			}
			else if ( OffsetStep >1)
			{
				const std::int16_t * const pData = rWave().pWaveDataL();
				for(c = 0; c < nWidth; c++)
				{
					long const offset = (long)floorf(c * OffsetStep);
					yLow=0;yHi=0;
					for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
					{
						if (yLow > pData[d]) yLow = pData[d];
						if (yHi < pData[d]) yHi = pData[d];
					}
					int const ryLow = (wrHeight * yLow)/32768;
					int const ryHi = (wrHeight * yHi)/32768;
					dc.MoveTo(c,(wrHeight) - ryLow);
					dc.LineTo(c,(wrHeight) - ryHi);
				}
			}
			else
			{
				for(c = 0; c < nWidth; c++)
				{
					ULARGE_INTEGER posin;
					posin.QuadPart = c * OffsetStep* 4294967296.0f;
					yHi=0;
					yLow=resampler.work(rWave().pWaveDataL()+posin.HighPart,posin.HighPart,posin.LowPart,rWave().WaveLength(), NULL);

					int const ryLow = (wrHeight * yLow)/32768;
					int const ryHi = (wrHeight * yHi)/32768;
					dc.MoveTo(c,(wrHeight) - ryLow);
					dc.LineTo(c,(wrHeight) - ryHi);
				}
			}

			if(rWave().IsWaveStereo())
			{
				if ( OffsetStep > 4)
				{
					const std::int16_t * const pData = rWave().pWaveDataR();
					for(c = 0; c < nWidth; c++)
					{
						long const offset = (long)floorf(c * OffsetStep);
						yLow=0;yHi=0;
						for (d = offset; d < offset + OffsetStep; d+=(OffsetStep/4))
						{
							if (yLow > pData[d]) yLow = pData[d];
							if (yHi < pData[d]) yHi = pData[d];
						}
						int const ryLow = (wrHeight * yLow)/32768;
						int const ryHi = (wrHeight * yHi)/32768;
						dc.MoveTo(c,wrHeight_R - ryLow);
						dc.LineTo(c,wrHeight_R - ryHi);
					}
				}
				else if ( OffsetStep >1)
				{
					const std::int16_t * const pData = rWave().pWaveDataR();
					for(c = 0; c < nWidth; c++)
					{
						long const offset = (long)floorf(c * OffsetStep);
						yLow=0;yHi=0;
						for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
						{
							if (yLow > pData[d]) yLow = pData[d];
							if (yHi < pData[d]) yHi = pData[d];
						}
						int const ryLow = (wrHeight * yLow)/32768;
						int const ryHi = (wrHeight * yHi)/32768;
						dc.MoveTo(c,wrHeight_R - ryLow);
						dc.LineTo(c,wrHeight_R - ryHi);
					}
				}
				else
				{
					for(c = 0; c < nWidth; c++)
					{
						ULARGE_INTEGER posin;
						posin.QuadPart = c * OffsetStep* 4294967296.0f;
						yHi=0;
						yLow=resampler.work(rWave().pWaveDataR()+posin.HighPart,posin.HighPart,posin.LowPart,rWave().WaveLength(), NULL);

						int const ryLow = (wrHeight * yLow)/32768;
						int const ryHi = (wrHeight * yHi)/32768;
						dc.MoveTo(c,wrHeight_R - ryLow);
						dc.LineTo(c,wrHeight_R - ryHi);
					}
				}
			}
			if ( rWave().WaveLoopType() != XMInstrument::WaveData::LoopType::DO_NOT )
			{
				dc.SelectObject(&cpen_lo);
				int ls = (rWave().WaveLoopStart()* nWidth) /rWave().WaveLength();
				dc.MoveTo(ls,0);
				dc.LineTo(ls,nHeight);
				dc.TextOut(ls,12,"Start");
				int le = (rWave().WaveLoopEnd()* nWidth)/rWave().WaveLength();
				dc.MoveTo(le,0);
				dc.LineTo(le,nHeight);
				dc.TextOut(le-18,nHeight-24,"End");

			}
			if ( rWave().WaveSusLoopType() != XMInstrument::WaveData::LoopType::DO_NOT )
			{
				dc.SelectObject(&cpen_sus);
				int ls = (rWave().WaveSusLoopStart()* nWidth)/rWave().WaveLength();
				dc.MoveTo(ls,0);
				dc.LineTo(ls,nHeight);
				dc.TextOut(ls,0,"Start");
				int le = (rWave().WaveSusLoopEnd()* nWidth)/rWave().WaveLength();
				dc.MoveTo(le,0);
				dc.LineTo(le,nHeight);
				dc.TextOut(le-18,nHeight-12,"End");
			}
			dc.SelectObject(oldpen);
		}
		else
		{
			dc.TextOut(4,4,"No Wave Data");
		}
		dc.Detach();
	}
}
}}