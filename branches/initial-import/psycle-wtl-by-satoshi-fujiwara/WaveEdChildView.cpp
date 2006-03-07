/** @file 
 *  @brief
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <mmreg.h>
#include <math.h>
#include "MainFrm.h"
#include "WaveEdChildView.h"
#include "WaveEdFrame.h"
//#include "Song.h" // included  in "MainFrm.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/


CWaveEdChildView::CWaveEdChildView()
{
	m_PenLow.CreatePen(PS_SOLID,0,0xFF0000);
	m_PenMedium.CreatePen(PS_SOLID,0,0xCCCCCC);
	m_PenHigh.CreatePen(PS_SOLID,0,0x00FF00);

	m_bDrawWave=true;
	m_SelStart=0;
	
	m_bWdWave=false;
	m_WsInstrument=-1;
	m_WsWave=-1;

	m_SelX=0;m_SelX2=0;
}

CWaveEdChildView::~CWaveEdChildView()
{
	m_PenLow.DeleteObject();
	m_PenMedium.DeleteObject();
	m_PenHigh.DeleteObject();
}

void CWaveEdChildView::UpdateUICompo(){
	 m_pWaveEdFrame->UIEnable(ID_SELECTION_AMPLIFY,m_bWdWave && m_bBlSelection && m_BlLength > 1);
	 m_pWaveEdFrame->UIEnable(ID_SELECTION_REVERSE,m_bWdWave && m_bBlSelection && m_BlLength > 1);
	 m_pWaveEdFrame->UIEnable(ID_SELECTION_FADEIN, m_bWdWave && m_bBlSelection && m_BlLength > 1);
	 m_pWaveEdFrame->UIEnable(ID_SELECTION_FADEOUT, m_bWdWave && m_bBlSelection && m_BlLength > 1);
	 m_pWaveEdFrame->UIEnable(ID_SELECTION_NORMALIZE, m_bWdWave);
	 m_pWaveEdFrame->UIEnable(ID_SELECTION_REMOVEDC, m_bWdWave);
	 m_pWaveEdFrame->UIEnable(ID_SELECCION_ZOOM, m_bWdWave && m_bBlSelection);
	 m_pWaveEdFrame->UIEnable(ID_SELECCION_ZOOMOUT, m_bWdWave && (m_DiLength < m_WdLength));
	 m_pWaveEdFrame->UIEnable(ID_SELECTION_SHOWALL, m_bWdWave && (m_DiLength < m_WdLength));
	 m_pWaveEdFrame->UIEnable(ID_EDIT_COPY, m_bWdWave && m_bBlSelection && m_BlLength > 1);
	 m_pWaveEdFrame->UIEnable(ID_EDIT_CUT, m_bWdWave && m_bBlSelection && m_BlLength > 1);
	 m_pWaveEdFrame->UIEnable(ID_EDIT_PASTE,  ((m_bWdWave && m_bBlSelection && m_BlLength == 1) || !m_bWdWave) && ::IsClipboardFormatAvailable(CF_WAVE));
	 m_pWaveEdFrame->UIEnable(ID_EDIT_DELETE, m_bWdWave && m_bBlSelection && m_BlLength > 1);
	 m_pWaveEdFrame->UIEnable(ID_CONVERT_MONO, m_bWdWave && m_WdStereo);
	 m_pWaveEdFrame->UIEnable(ID_EDIT_UNDO, false);
	 m_pWaveEdFrame->UIEnable(ID_EDIT_SELECT_ALL, m_bWdWave);

}

/////////////////////////////////////////////////////////////////////////////
// CWaveEdChildView drawing

void CWaveEdChildView::OnPaint(HDC hdc)
{
	CPaintDC dc(m_hWnd);
	CPaintDC *pDC = &dc;

	int wrHeight = 0, wrHeight_R = 0, yLow = 0, yHi = 0, absBuf = 0, abs_yBuf = 0;
	double OffsetStep = 0;
	__int32 c, d;
//	LOGPEN _pen;
		
	if(m_bWdWave)
	{
		CRect rect;
		GetClientRect(&rect);
		
		int const nWidth = rect.Width();
		int const nHeight = rect.Height();
		int const my = nHeight / 2;
		
		if(m_WdStereo) 
			wrHeight = my / 2;
		else 
			wrHeight = my;
		
		if(m_bDrawWave)
		{
			// Draw preliminary stuff
			
			CPenHandle oldpen(pDC->SelectPen((HPEN)m_PenMedium));
			
			// Left channel 0 amplitude line
			pDC->MoveTo(0,wrHeight);
			pDC->LineTo(nWidth,wrHeight);
			
			int const wrHeight_R = my + wrHeight;
			
			if(m_WdStereo)
			{
				// Stereo channels separator line
				pDC->SelectPen((HPEN)m_PenLow);
				pDC->MoveTo(0,my);
				pDC->LineTo(nWidth,my);
				
				// Right channel 0 amplitude line
				pDC->SelectPen((HPEN)m_PenMedium);
				pDC->MoveTo(0,wrHeight_R);
				pDC->LineTo(nWidth,wrHeight_R);
			}
			
			// Draw samples in channels (Fideloop's)

			pDC->SelectPen((HPEN)m_PenHigh);
//			m_PenHigh.GetLogPen(&_pen);

			OffsetStep = (double) m_DiLength / nWidth;

			for(c = 0; c < nWidth; c++)
			{
				long const offset = m_DiStart + (long)(c * OffsetStep);

				yLow = 0, yHi = 0;

				for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
				{
					if (yLow > *(m_WdLeft + d)) yLow = *(m_WdLeft + d);
					if (yHi < *(m_WdLeft + d)) yHi = *(m_WdLeft + d);
				}

				int const ryLow = (wrHeight * yLow)/32768; // 32767...
				int const ryHi = (wrHeight * yHi)/32768;
				
				pDC->MoveTo(c,wrHeight - ryLow);
				pDC->LineTo(c,wrHeight - ryHi);
			}

			if(m_WdStereo)
			{
				for(c = 0; c < nWidth; c++)
				{
					long const offset = m_DiStart + (long)(c * OffsetStep);

					yLow = 0, yHi = 0;

					for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
					{
						if (yLow > *(m_WdRight + d)) yLow = *(m_WdRight + d);
						if (yHi < *(m_WdRight + d)) yHi = *(m_WdRight + d);
					}

					int const ryLow = (wrHeight * yLow)/32768; // 32767...
					int const ryHi = (wrHeight * yHi)/32768;
					
					pDC->MoveTo(c,wrHeight_R - ryLow);
					pDC->LineTo(c,wrHeight_R - ryHi);
				}
			}

			if ( m_bWdLoop )
			{
				pDC->SelectPen((HPEN)m_PenLow);
				if ( m_WdLoopS >= m_DiStart && m_WdLoopS < m_DiStart+m_DiLength)
				{
					int ls = ((m_WdLoopS-m_DiStart)*nWidth)/m_DiLength;
					pDC->MoveTo(ls,0);
					pDC->LineTo(ls,nHeight);
					pDC->TextOut(ls,0,_T("S"));
				}
				if ( m_WdLoopE >= m_DiStart && m_WdLoopE < m_DiStart+m_DiLength)
				{
					int le = ((m_WdLoopE-m_DiStart)*nWidth)/m_DiLength;
					pDC->MoveTo(le,0);
					pDC->LineTo(le,nHeight);
					pDC->TextOut(le-8,nHeight-16,_T("E"));
				}

			}
			pDC->SelectPen((HPEN)oldpen);
			
		}// Draw wave
		
		pDC->SetROP2(R2_NOT);
		
		if(!m_bDrawWave)
		{
			// Unmark selection
			pDC->Rectangle(m_SelX,0,m_SelX2,nHeight);
		}
		
		m_SelX=((m_BlStart-m_DiStart)*nWidth)/m_DiLength;
		m_SelX2=(((m_BlStart+m_BlLength)-m_DiStart)*nWidth)/m_DiLength;
		pDC->Rectangle(m_SelX,0,m_SelX2,nHeight);
	}
	else
	{
		pDC->TextOut(4,4,_T("No Wave Data"));
	}
	
	m_bDrawWave = true;
	SetMsgHandled(FALSE);
	// Do not call CWnd::OnPaint() for painting messages
}

/////////////////////////////////////////////////////////////////////////////
// CWaveEdChildView message handlers
/*
BOOL CWaveEdChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	LOGBRUSH LogB;
	HBRUSH	hBrush;

	LogB.lbColor = 0x00000000;
	LogB.lbStyle = BS_SOLID;

	hBrush = CreateBrushIndirect(&LogB);
	
	if (!CWnd::PreCreateWindow(cs)) return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
	::LoadCursor(NULL, IDC_ARROW), hBrush, NULL);

	DeleteObject(hBrush);

	return TRUE;
	
//	return CWnd::PreCreateWindow(cs);
}
*/

void CWaveEdChildView::GenerateAndShow()
{
	m_bBlSelection = false;
	UpdateWindow();
}

void  CWaveEdChildView::SetViewData(int ins, int wav)
{
	int wl=m_pSong->pInstrument(ins)->waveLength[wav];

	m_WsInstrument=ins;	// Do not put inside of "if(wl)". Pasting needs this.
	m_WsWave=wav;

	if(wl)
	{
		m_bWdWave=true;
			
		m_WdLength=wl;
		m_WdLeft=m_pSong->pInstrument(ins)->waveDataL[wav];
		m_WdRight=m_pSong->pInstrument(ins)->waveDataR[wav];
		m_WdStereo=m_pSong->pInstrument(ins)->waveStereo[wav];
		m_bWdLoop=m_pSong->pInstrument(ins)->waveLoopType[wav];
		m_WdLoopS=m_pSong->pInstrument(ins)->waveLoopStart[wav];
		m_WdLoopE=m_pSong->pInstrument(ins)->waveLoopEnd[wav];

		m_DiStart=0;
		m_DiLength=wl;
		m_BlStart=0;
		m_BlLength=0;
		Invalidate();
	}
	else
	{
		m_bWdWave = false;
		SetWindowText(_T("Wave Editor [No Data]"));
		Invalidate(true);
	}

	m_bBlSelection=false;
}

/*void CWaveEdChildView::FitWave()
{
	if (m_BlStart < 0) m_BlStart = 0;
	if ( (m_BlStart + m_BlLength) > m_WdLength) m_BlLength = m_WdLength - m_BlStart;
}*/

void CWaveEdChildView::OnSelectionZoom()
{
	if(m_bBlSelection && m_bWdWave)
	{
		CRect rect;
		GetClientRect(&rect);
		unsigned long const nWidth=rect.Width();

		if ( m_BlLength*8 < nWidth ) // Selection is too small, zoom to smallest possible
		{
			m_DiLength=(unsigned long)(nWidth*0.125f);
			if ( m_BlStart+m_DiLength > m_WdLength ) m_DiStart = m_WdLength-m_DiLength;
			else m_DiStart = m_BlStart;
		}
		else
		{
			m_DiStart= m_BlStart;
			m_DiLength = m_BlLength;
		}
		Invalidate();
	}
}

void CWaveEdChildView::OnSelectionZoomOut()
{
	if(m_DiLength<m_WdLength)
	{
		if ( m_DiLength >= m_DiStart )
			m_DiStart=0;
		else
			m_DiStart-=m_DiLength;
		
		m_DiLength=m_DiLength*3;
		if(m_DiLength+m_DiStart>m_WdLength) m_DiLength=m_WdLength-m_DiStart;
		
		Invalidate();
	}
}

void CWaveEdChildView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if(m_bWdWave)
	{
		int const x=point.x;

		if ( nFlags & MK_CONTROL )
		{
			m_pParent->m_view.AddMacViewUndo();
			m_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);

			CRect rect;
			GetClientRect(&rect);
			m_WdLoopE = m_DiStart+((x*m_DiLength)/rect.Width());
			m_pSong->pInstrument(m_WsInstrument)->waveLoopEnd[m_WsWave]=m_WdLoopE;
			if (m_pSong->pInstrument(m_WsInstrument)->waveLoopStart[m_WsWave]> m_WdLoopE )
			{
				m_pSong->pInstrument(m_WsInstrument)->waveLoopStart[m_WsWave]=m_WdLoopE;
			}
			if (!m_bWdLoop) 
			{
				m_bWdLoop=true;
				m_pSong->pInstrument(m_WsInstrument)->waveLoopType[m_WsWave]=true;
			}
			m_pSong->IsInvalided(false);
			m_bDrawWave=true;
			m_pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
			Invalidate();

		}
		else
		{
			if (m_bBlSelection) OnSelectionZoom();
			else OnSelectionZoomOut();
		}
	}
	SetMsgHandled(false);
	//CWnd::OnRButtonDown(nFlags, point);
}


void CWaveEdChildView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	if(m_bWdWave)
	{
		int const x=point.x;

		if ( nFlags & MK_CONTROL )
		{
			m_pParent->m_view.AddMacViewUndo();
			m_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);

			CRect rect;
			GetClientRect(&rect);
			m_WdLoopS = m_DiStart+((x*m_DiLength)/rect.Width());
			m_pSong->pInstrument(m_WsInstrument)->waveLoopStart[m_WsWave]=m_WdLoopS;
			if (m_pSong->pInstrument(m_WsInstrument)->waveLoopEnd[m_WsWave] < m_WdLoopS )
			{
				m_pSong->pInstrument(m_WsInstrument)->waveLoopEnd[m_WsWave]=m_WdLoopS;
			}
			if (!m_bWdLoop) 
			{
				m_bWdLoop=true;
				m_pSong->pInstrument(m_WsInstrument)->waveLoopType[m_WsWave]=true;
			}
			m_pSong->IsInvalided(false);
			m_pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
			m_bDrawWave=true;
			Invalidate();
		}
		else if ( nFlags == 1 )
		{
			m_bBlSelection=false;
			
			CRect rect;
			GetClientRect(&rect);
			int const nWidth=rect.Width();
			
			m_BlStart=m_DiStart+((x*m_DiLength)/nWidth);
			m_BlLength=1;
			m_SelStart = x; //!!! new variable!
			
			m_bDrawWave=false;
		
			Invalidate(false);
		}
	}
	SetMsgHandled(FALSE);	
//	CWnd::OnLButtonDown(nFlags, point);
}

void CWaveEdChildView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	if(m_bBlSelection)
	{
		m_BlStart=m_DiStart;
		m_BlLength=m_DiLength;
		m_bDrawWave=false;
		Invalidate(false);
	}
}

void CWaveEdChildView::OnMouseMove(UINT nFlags, CPoint point) //Fideloop's
{
	if(nFlags == MK_LBUTTON && m_bWdWave)
	{
		int x=point.x;
		CRect rect;
		GetClientRect(&rect);
		int const nWidth=rect.Width();
		float diRatio = (float) m_DiLength/nWidth;
		
		if (x >= (long) m_SelStart)
		{
			if (x > nWidth)	{ x = nWidth; }
			m_BlStart = (long) (m_SelStart*diRatio + m_DiStart);
			m_BlLength = (long)(x*diRatio + m_DiStart - m_BlStart);

		}
						
		else
		{
			if (x < 0) { x = 0; }
			m_BlStart = (long) ( x*diRatio + m_DiStart);
			m_BlLength = (long) (m_SelStart*diRatio + m_DiStart - m_BlStart);
		}
		m_bBlSelection=true;
		m_bDrawWave=false;
		Invalidate(false);
	}
	
	
//	CWnd::OnMouseMove(nFlags, point);
	SetMsgHandled(FALSE);	

}

void CWaveEdChildView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
//	CWnd::OnLButtonUp(nFlags, point);
	SetMsgHandled(FALSE);	
}

void CWaveEdChildView::OnSelectionFadeIn()
{
	if(m_bBlSelection && m_bWdWave)
	{
		m_pParent->m_view.AddMacViewUndo();

		m_pSong->IsInvalided(true);
		Sleep(LOCK_LATENCY);

		double fpass=1.0/m_BlLength;
		double val=0.0f;
		
		for(unsigned long c=m_BlStart;c<m_BlStart+m_BlLength;c++)
		{
			float const vl=*(m_WdLeft+c);
			
			*(m_WdLeft+c)=signed short(vl*val);
			
			if(m_WdStereo)
			{
				float const vr=*(m_WdRight+c);
				*(m_WdRight+c)=signed short(vr*val);
			}
			
			val+=fpass;
		}
		
		Invalidate(true);
		m_pSong->IsInvalided(false);
	}
}


void CWaveEdChildView::OnSelectionFadeOut()
{
	if(m_bBlSelection && m_bWdWave)
	{
		m_pParent->m_view.AddMacViewUndo();

		m_pSong->IsInvalided(true);
		Sleep(LOCK_LATENCY);

		double fpass=1.0/m_BlLength;
		double val=1.0f;
		
		for(unsigned long c=m_BlStart;c<m_BlStart+m_BlLength;c++)
		{
			
			float const vl=*(m_WdLeft+c);
			
			*(m_WdLeft+c)=signed short(vl*val);
			
			if(m_WdStereo)
			{
				float const vr=*(m_WdRight+c);
				*(m_WdRight+c)=signed short(vr*val);
			}
			
			val-=fpass;
		}
		
		Invalidate(true);
		m_pSong->IsInvalided(false);
	}
}

void CWaveEdChildView::OnSelectionNormalize() // (Fideloop's)
{
	signed short maxL = 0, maxR = 0, absBuf;
	double ratio = 0, buf;
	unsigned long c = 0;

	if (m_bWdWave)
	{
		m_pParent->m_view.AddMacViewUndo();

		m_pSong->IsInvalided(true);
		Sleep(LOCK_LATENCY);

		for (c = 0 ; c < m_WdLength ; c++)
		{

			if (*(m_WdLeft+c) < 0)
			{
				if (!( *(m_WdLeft+c) == -32768) )	absBuf = - *(m_WdLeft + c);
				else absBuf = 32767;
			}
			else absBuf = *(m_WdLeft + c);
			if (maxL < absBuf) maxL = absBuf;
		}

		if (m_WdStereo)

		{
			for (c = 0; c< m_WdLength; c++)
			{
				if (*(m_WdRight+c) < 0)
				{
					if (!( *(m_WdRight+c) == -32768) )	absBuf = - *(m_WdRight + c);
					else absBuf = 32767;
				}
				else absBuf = *(m_WdRight + c);
				if (maxR < absBuf) maxR = absBuf;
			}
		}

		if ( (maxL < maxR) && (m_WdStereo) ) maxL = maxR;
		
		if (maxL) ratio = (double) 32767 / maxL;
		
		if (ratio != 1)
		{
			for (c=0; c<m_WdLength; c++)
			{
				buf = *(m_WdLeft + c);
				*(m_WdLeft + c) = (short)(buf*ratio);

				if (m_WdStereo)
				{
					buf = *(m_WdRight + c);
					*(m_WdRight + c) = (short)(buf*ratio);
				}
			}
		}

		Invalidate(true);
		m_pSong->IsInvalided(false);
	}
}

void CWaveEdChildView::OnSelectionRemoveDC() // (Fideloop's)
{
	double meanL = 0, meanR = 0;
	unsigned long c = 0;
	signed short buf;

	if (m_bWdWave)
	{
		m_pParent->m_view.AddMacViewUndo();

		m_pSong->IsInvalided(true);
		Sleep(LOCK_LATENCY);

		for (c=0; c<m_WdLength; c++)
		{
			meanL = meanL + ( (double) *(m_WdLeft+c) / m_WdLength);

			if (m_WdStereo) meanR = (double) meanR + ((double) *(m_WdRight+c) / m_WdLength);
		}

		for (c=0; c<m_WdLength; c++)
		{
			buf = *(m_WdLeft+c);
			if (meanL > 0)
			{
				if ((double)(buf - meanL) < (-32768))	*(m_WdLeft+c) = -32768;
				else	*(m_WdLeft+c) = (short)(buf - meanL);
			}
			else
			{
				if (meanL < 0)
				{
					if ((double)(buf - meanL) > 32767) *(m_WdLeft+c) = 32767;
				}
				else *(m_WdLeft + c) = (short)(buf - meanL);
			}

		}
	
		if (m_WdStereo)
		{
			for (c=0; c<m_WdLength; c++)
			{
				buf = *(m_WdRight+c);
				if (meanR > 0)
				{
					if ((double)(buf - meanR) < (-32768))	*(m_WdRight + c) = -32768;
					else	*(m_WdRight+c) = (short)(buf - meanR);
				}
				else
				{
					if (meanR < 0)
					{
						if ((double)(buf - meanR) > 32767) *(m_WdRight+c) = 32767;
					}
					else *(m_WdRight + c) = (short)(buf - meanR);
				}
			}
		}
		m_pSong->IsInvalided(false);
		Invalidate(true);
	}
}

void CWaveEdChildView::OnSelectionAmplify()
{
	short buf = 0;
	double ratio = 1;
	unsigned int c = 0;
	int pos = 0;

	if (m_bWdWave && m_bBlSelection)
	{
		m_pParent->m_view.AddMacViewUndo();

		pos = m_AmpDialog.DoModal();
		if (pos != AMP_DIALOG_CANCEL)
		{
			m_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);
			ratio = pow(10.0, (double) pos / (double) 2000);
		
			for (c=m_BlStart; c<m_BlStart+m_BlLength; c++)
			{
				buf = *(m_WdLeft + c);
				if (buf < 0)
				{
					if ( (double) (buf*ratio) < -32768 ) *(m_WdLeft+c) = -32768;
					else *(m_WdLeft + c) = (short)(buf*ratio);
				}
				else
				{
					if ( (double) (buf*ratio) > 32767 ) *(m_WdLeft+c) = 32767;
					else *(m_WdLeft + c) = (short)(buf*ratio);
				}


				if (m_WdStereo)
				{
					buf = *(m_WdRight + c);
					if (buf < 0)
					{	
						if ( (double) (buf*ratio) < -32768 ) *(m_WdRight+c) = -32768;
						else *(m_WdRight + c) = (short)(buf*ratio);
					}
					else
					{
						if ( (double) (buf*ratio) > 32767 ) *(m_WdRight+c) = 32767;
						else *(m_WdRight + c) = (short)(buf*ratio);
					}
				}
			}
			m_pSong->IsInvalided(false);
			Invalidate(true);
		}
	}
}

void CWaveEdChildView::OnSelectionReverse() 
{
	short buf = 0;
	int c, halved = 0;

	if (m_bWdWave && m_bBlSelection)
	{
		m_pParent->m_view.AddMacViewUndo();

		m_pSong->IsInvalided(true);
		Sleep(LOCK_LATENCY);

		halved = (int) floor((double)m_BlLength / 2.0);

		for (c = 0; c < halved; c++)
		{
			buf = *(m_WdLeft+m_BlStart+m_BlLength - c);
			*(m_WdLeft+m_BlStart+m_BlLength - c) = *(m_WdLeft+m_BlStart + c);
			*(m_WdLeft+m_BlStart + c) = buf;

			if (m_WdStereo)
			{
				buf = *(m_WdRight+m_BlStart+m_BlLength - c);
				*(m_WdRight+m_BlStart+m_BlLength - c) = *(m_WdRight+m_BlStart + c);
				*(m_WdRight+m_BlStart + c) = buf;
			}

		}
		Invalidate(true);
		m_pSong->IsInvalided(false);
	}
}

void CWaveEdChildView::OnSelectionShowall() 
{
	m_DiStart = 0;
	m_DiLength = m_WdLength;
	Invalidate(true);
}

void CWaveEdChildView::OnConvertMono() 
{
	if (m_bWdWave && m_WdStereo)
	{
		m_pParent->m_view.AddMacViewUndo();

		m_pSong->IsInvalided(true);
		Sleep(LOCK_LATENCY);

//		SetUndo(4, m_WdLeft, m_WdRight, m_WdLength); 
		for (unsigned int c = 0; c < m_WdLength; c++)
		{
			*(m_WdLeft + c) = ( *(m_WdLeft + c) + *(m_WdRight + c) ) / 2;
		}

		m_pSong->pInstrument(m_WsInstrument)->waveStereo[m_WsWave] = false;
		m_WdStereo = false;
		delete m_pSong->pInstrument(m_WsInstrument)->waveDataR[m_WsWave];
		Invalidate(true);
		m_pSong->IsInvalided(false);
	}
}

//Clipboard
void CWaveEdChildView::OnEditDelete()
{
	short* pTmp = 0, *pTmpR = 0;
	long datalen = 0;

	if (m_bWdWave && m_bBlSelection)
	{
		m_pParent->m_view.AddMacViewUndo();

		m_pSong->IsInvalided(true);
		Sleep(LOCK_LATENCY);

		datalen = (m_WdLength - m_BlLength);
		if (datalen)
		{
			pTmp = new signed short[datalen];
			
			if (m_WdStereo)
			{
				pTmpR= new signed short[datalen];
				CopyMemory(pTmpR, m_WdRight, m_BlStart*sizeof(short));
				CopyMemory( (pTmpR+m_BlStart), (m_WdRight + m_BlStart + m_BlLength), (m_WdLength - m_BlStart - m_BlLength)*sizeof(short) );
				delete m_pSong->pInstrument(m_WsInstrument)->waveDataR[m_WsWave];
				m_pSong->pInstrument(m_WsInstrument)->waveDataR[m_WsWave] = pTmpR;
				m_WdRight = pTmpR;
			}

			CopyMemory( pTmp, m_WdLeft, m_BlStart*sizeof(short) );
			CopyMemory( (pTmp+m_BlStart), (m_WdLeft + m_BlStart + m_BlLength), (m_WdLength - m_BlStart - m_BlLength)*sizeof(short) );
			delete m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave];
			
			m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave] = pTmp;
			m_WdLeft = pTmp;
			m_pSong->pInstrument(m_WsInstrument)->waveLength[m_WsWave] = datalen;
			m_WdLength = datalen;
		}
		else
		{
			m_pSong->DeleteLayer(m_WsInstrument, m_WsWave);
			m_WdLength = 0;
			m_bWdWave   = false;
		}
	
		//Validate display
		if ( (m_DiStart + m_DiLength) > m_WdLength )
		{
			long newlen = m_WdLength - m_DiLength;

			if ( newlen < 0 )
				this->OnSelectionShowall();
			else
				m_DiStart = (unsigned)newlen;
		}
		
		m_bBlSelection = false;
		m_BlLength  = 0;
		m_BlStart   = 0;
		m_pParent->ChangeIns(m_WsInstrument); // This causes an update of the Instrument Editor.

		Invalidate(true);
		m_pSong->IsInvalided(false);
	}
}
/*
//Menu update handlers.
void CWaveEdChildView::OnUpdateConvertMono(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bWdWave && m_WdStereo);
}
*/
/* Clipboard functions */

void CWaveEdChildView::OnEditCopy() 
{
	 unsigned long c = 0;
		
	struct fullheader
	{
		DWORD	head;
		DWORD	size;
		DWORD	head2;
		DWORD	fmthead;
		DWORD	fmtsize;
		WAVEFORMATEX	fmtcontent;
		DWORD datahead;
		DWORD datasize;
	} wavheader;

	OpenClipboard();
	EmptyClipboard();
	m_hClipboardData = GlobalAlloc(GMEM_MOVEABLE, ( m_WdStereo ? m_BlLength*4 + sizeof(fullheader) : m_BlLength*2 + sizeof(fullheader)));
	
	wavheader.head = 'FFIR';
	wavheader.size = m_WdStereo ? (m_BlLength*4 + sizeof(fullheader) - 8) : (m_BlLength*2 + sizeof(fullheader) - 8);
	wavheader.head2= 'EVAW';
	wavheader.fmthead = ' tmf';
	wavheader.fmtsize = sizeof(WAVEFORMATEX) + 2; // !!!!!!!!!!!!!!!!????????? - works...
	wavheader.fmtcontent.wFormatTag = WAVE_FORMAT_PCM;
	wavheader.fmtcontent.nChannels = m_WdStereo ? 2 : 1;
	wavheader.fmtcontent.nSamplesPerSec = 44100;
	wavheader.fmtcontent.wBitsPerSample = 16;
	wavheader.fmtcontent.nAvgBytesPerSec = wavheader.fmtcontent.wBitsPerSample/8*wavheader.fmtcontent.nChannels*wavheader.fmtcontent.nSamplesPerSec;
	wavheader.fmtcontent.nBlockAlign = m_WdStereo ? 4 : 2 ;
	wavheader.fmtcontent.cbSize = 0;
	wavheader.datahead = 'atad';
	wavheader.datasize = m_WdStereo ? m_BlLength*4 : m_BlLength*2;

	m_pClipboardData = (char*) GlobalLock(m_hClipboardData);
	
	CopyMemory(m_pClipboardData, &wavheader, sizeof(fullheader) );
	if (m_WdStereo)
	{
		m_pClipboardData += sizeof(fullheader);
		for (c = 0; c < m_BlLength*2; c += 2)
		{
			*((signed short*)m_pClipboardData + c) = *(m_WdLeft + m_BlStart + (long)(c*0.5));
			*((signed short*)m_pClipboardData + c + 1) = *(m_WdRight + m_BlStart + (long)(c*0.5));
		}
	}
	else
	{
		CopyMemory(m_pClipboardData + sizeof(fullheader), (m_WdLeft + m_BlStart), m_BlLength*2);
	}

	GlobalUnlock(m_hClipboardData);
	SetClipboardData(CF_WAVE, m_hClipboardData);
	CloseClipboard();
	Invalidate(true);
}

void CWaveEdChildView::OnEditCut() 
{
	OnEditCopy();
	OnEditDelete();
}

void CWaveEdChildView::OnEditPaste() 
{
	unsigned long c = 0;

	m_pParent->m_view.AddMacViewUndo();

	char *pData;
	DWORD lFmt, lData;
	
	WAVEFORMATEX* pFmt;
	short* pPasteData;
	short* pTmp = 0, *pTmpR = 0;

	OpenClipboard();
	m_hPasteData = GetClipboardData(CF_WAVE);
	pPasteData = (short*)GlobalLock(m_hPasteData);

	if ((*(DWORD*)pPasteData != 'FFIR') && (*((DWORD*)pPasteData + 2)!='EVAW')) return;
	lFmt= *(DWORD*)((char*)pPasteData + 16);
	pFmt = (WAVEFORMATEX*)((char*)pPasteData + 20); //'RIFF' + len. +'WAVE' + 'fmt ' + len. = 20 bytes.

	lData = *(DWORD*)((char*)pPasteData + 20 + lFmt + 4);
	pData = (char*)pPasteData + 20 + lFmt + 8;

	m_pSong->IsInvalided(true);
	Sleep(LOCK_LATENCY);

	if (!m_bWdWave)
	{
		if (pFmt->wBitsPerSample == 16)
		{
			m_pSong->WavAlloc(m_WsInstrument, m_WsWave, (pFmt->nChannels==2) ? true : false, (pFmt->nChannels==2) ? (DWORD)(lData*0.25) : (DWORD)(lData*0.5), _T("Clipboard"));
			if (pFmt->nChannels == 1)
			{
				memcpy(m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave], pData, lData);
				m_WdLength = (DWORD)(lData*0.5);
				m_WdLeft  = m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave];
				m_WdStereo = false;
			}
			else if (pFmt->nChannels == 2)
			{
				for (c = 0; c < lData*0.5; c += 2)
				{
					*(m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave] + (long)(c*0.5)) = *((signed short*)pData + c);
					*(m_pSong->pInstrument(m_WsInstrument)->waveDataR[m_WsWave] + (long)(c*0.5)) = *((signed short*)pData + c + 1);
				}
				m_WdLength = (DWORD)(lData *0.25);
				m_WdLeft = m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave];
				m_WdRight = m_pSong->pInstrument(m_WsInstrument)->waveDataR[m_WsWave];
				m_WdStereo = true;
			}
			m_bWdWave = true;
		}
	}
	else
	{
		if (pFmt->wBitsPerSample == 16)
		{
			if ( (pFmt->nChannels == 1) && (m_WdStereo == false) )
			{ 
				pTmp = new signed short[(DWORD)(lData*0.5) + m_WdLength];
				memcpy(pTmp, m_WdLeft, m_BlStart*2);
				memcpy(pTmp + m_BlStart, pData, lData);
				memcpy((BYTE*)pTmp + m_BlStart*2 + lData, m_WdLeft + m_BlStart, 2*(m_WdLength - m_BlStart));

				delete m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave];
				m_WdLeft = m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave] = pTmp;
				m_pSong->pInstrument(m_WsInstrument)->waveLength[m_WsWave] = m_WdLength + (DWORD)(lData*0.5);
				m_WdLength = m_WdLength + (DWORD)(lData*0.5);
			}
			else if ( (pFmt->nChannels == 2) && (m_WdStereo == true) )
			{
				unsigned long c;
				pTmp = new signed short[(DWORD)(lData*0.25) + m_WdLength];
				pTmpR= new signed short[(DWORD)(lData*0.25) + m_WdLength];
				memcpy(pTmp, m_WdLeft, m_BlStart*2);
				memcpy(pTmpR,m_WdRight,m_BlStart*2);
				for (c = 0; c < (unsigned long)(lData*0.25); c++)
				{
					*(pTmp + m_BlStart + c) = *(short*)(pData + c*4);
					*(pTmpR+ m_BlStart + c) = *(short*)(pData + c*4 + 2);
				}
				memcpy((BYTE*)pTmp + m_BlStart*2 + (unsigned long)(lData*0.5), m_WdLeft + m_BlStart, 2*(m_WdLength - m_BlStart));
				memcpy((BYTE*)pTmpR+ m_BlStart*2 + (unsigned long)(lData*0.5), m_WdRight + m_BlStart, 2*(m_WdLength - m_BlStart));

				delete m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave];
				m_WdLeft = m_pSong->pInstrument(m_WsInstrument)->waveDataL[m_WsWave] = pTmp;
				delete m_pSong->pInstrument(m_WsInstrument)->waveDataR[m_WsWave];
				m_WdRight = m_pSong->pInstrument(m_WsInstrument)->waveDataR[m_WsWave] = pTmpR;
				m_pSong->pInstrument(m_WsInstrument)->waveLength[m_WsWave] = m_WdLength + (DWORD)(lData*0.25);
				m_WdLength = m_WdLength + (DWORD)(lData*0.25);
			}
		}
	}

	GlobalUnlock(m_hPasteData);
	CloseClipboard();
	OnSelectionShowall();

	m_pParent->ChangeIns(m_WsInstrument); // This causes an update of the Instrument Editor.
	Invalidate(true);
	m_pSong->IsInvalided(false);
}

void CWaveEdChildView::OnEditSelectAll() 
{
	m_DiStart = 0;
	m_BlStart = 0;
	m_DiLength = m_WdLength;
	m_BlLength = m_WdLength;
	m_bBlSelection = true;
	Invalidate(true);
}

void CWaveEdChildView::OnDestroyClipboard() 
{
//	CWnd::OnDestroyClipboard();
	GlobalFree(m_hClipboardData);
	SetMsgHandled(FALSE);
}

void CWaveEdChildView::SetSong(Song* _sng)
{
	m_pSong = _sng;
}
void CWaveEdChildView::SetParent(CMainFrame* parent)
{
	m_pParent = parent;
}