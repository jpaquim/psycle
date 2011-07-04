// SubMarco.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "SubMarco.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSubMarco

IMPLEMENT_DYNCREATE(CSubMarco, CFrameWnd)

CSubMarco::CSubMarco()
{
	obx=0;
	obx2=0;
	dontdrawwave=false;
	
	dragging=false;
	wdWave=false;
	wsInstrument=-1;
	wsWave=-1;

}

CSubMarco::~CSubMarco()
{
}


BEGIN_MESSAGE_MAP(CSubMarco, CFrameWnd)
	//{{AFX_MSG_MAP(CSubMarco)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_SELECCION_ZOOM,OnSelectionZoom)
	ON_COMMAND(ID_SELECCION_ZOOMOUT,OnSelectionZoomOut)
	ON_COMMAND(ID_SELECTION_FADEIN,OnSelectionFadeIn)
	ON_COMMAND(ID_SELECTION_FADEOUT,OnSelectionFadeOut)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSubMarco message handlers

void CSubMarco::OnClose() 
{
	// Funcion "Override" para que no responda a los mensajes
	// OnClose, en vez de eso, se oculta la ventana

	ShowWindow(SW_HIDE);
}

void CSubMarco::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	//CFrameWnd::OnRButtonDown(nFlags, point);
}

int CSubMarco::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

CSubMarco::GenerateAndShow()
{
	blSelection=false;
	cpen_lo.CreatePen(PS_SOLID,0,0x00000001);
	cpen_me.CreatePen(PS_SOLID,0,0x00CCCCCC);
	cpen_hi.CreatePen(PS_SOLID,0,0x00FF0000);
	UpdateWindow();
}


CSubMarco::SetWave(short *pleft, short *pright, int numsamples, bool stereo)
{
	wdLength=numsamples;
	wdLeft=pleft;
	wdRight=pright;
	wdStereo=stereo;
	diStart=0;
	diLength=numsamples;
	blStart=0;
	blLength=0;
	dragging=false;
	Invalidate();

}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Painting handler

void CSubMarco::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	FitWave();
	
	if(wdWave)
	{
	CRect rect;
	GetClientRect(&rect);
	int const nWidth=rect.Width();
	int const nHeight=rect.Height();
	int const my=nHeight/2;
	
	int wrHeight;

	if(wdStereo)
		wrHeight=my/2;
	else
		wrHeight=my;

	if(!dontdrawwave)
	{
	// Draw preliminary stuff --------------------------------------------------------------------
	
	dc.SelectObject(cpen_me);
	
	// Left channel 0 amplitude line
	dc.MoveTo(0,wrHeight);
	dc.LineTo(nWidth,wrHeight);

	int const my2=my+wrHeight;

	if(wdStereo)
	{
		// Stereo channels separator line
		dc.SelectObject(cpen_lo);
		dc.MoveTo(0,my);
		dc.LineTo(nWidth,my);

		// Right channel 0 amplitude line
		dc.SelectObject(cpen_me);
		dc.MoveTo(0,my+wrHeight);
		dc.LineTo(nWidth,my2);
	}

	dc.SelectObject(cpen_hi);

	// Draw samples in channels
	for(int c=0;c<nWidth;c++)
	{
		int const offset=diStart+((c*diLength)/nWidth);

		int const y=*(wdLeft+offset);
		int const ry=(wrHeight*y)/32768;

		dc.MoveTo(c,wrHeight);
		dc.LineTo(c,wrHeight-ry);

		if(wdStereo)
		{
		int const y2=*(wdRight+offset);
		int const ry2=(wrHeight*y2)/32768;

		dc.MoveTo(c,my2);
		dc.LineTo(c,my2-ry2);
		}
	}
	}// DontDraw wave
	
	dc.SetROP2(R2_NOT);

	if(dontdrawwave)
	dc.Rectangle(obx,0,obx2,nHeight);
	
	int const selx=((blStart-diStart)*nWidth)/diLength;
	int const selx2=(((blStart+blLength)-diStart)*nWidth)/diLength;
	dc.Rectangle(selx,0,selx2,nHeight);
	obx=selx;
	obx2=selx2;

	}
	else
	{
		dc.TextOut(4,4,"No Wave Data");
	}

	dontdrawwave=false;
}

void CSubMarco::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	if(wdWave)
	{
	blSelection=true;
	int const x=point.x;

	CRect rect;
	GetClientRect(&rect);
	int const nWidth=rect.Width();
	
	blStart=diStart+((x*diLength)/nWidth);
	blLength=1;

	dragging=true;

	dontdrawwave=true;

	Invalidate(false);
	}

	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CSubMarco::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	if(blSelection)
	{
		blStart=diStart;
		blLength=diLength;
		FitWave();
		dontdrawwave=true;
		Invalidate(false);
	}
}

void CSubMarco::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(dragging && wdWave)
	{

		int const x=point.x;
	
		CRect rect;
		GetClientRect(&rect);
		int const nWidth=rect.Width();
	
		blLength=(diStart+((x*diLength)/nWidth))-blStart;

		if(blLength<1)
		{
			blStart+=blLength;
			blLength=1;
		}
		
		FitWave();

		dontdrawwave=true;
		Invalidate(false);
	}


	CFrameWnd::OnMouseMove(nFlags, point);
}

void CSubMarco::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	dragging=false;
	CFrameWnd::OnLButtonUp(nFlags, point);
}

CSubMarco::AdquireFromSong(int ins, int wav, bool show)
{
	int si=songRef->instSelected;
	int sw=songRef->waveSelected;
	int wl=songRef->waveLength[si][sw];

	if(wl)
	{
	wsInstrument=ins;
	wsWave=wav;
	char buffer[1024];
	sprintf(buffer,"Wave Editor [%s Instrument:%d Layer:%d]",songRef->waveName[si][sw],ins,wav);
	wdWave=true;
	SetWindowText(buffer);

	SetWave(
	songRef->waveDataL[si][sw],
	songRef->waveDataR[si][sw],
	wl,
	songRef->waveStereo[si][sw]);
	}
	else
	{
	wdWave=false;
	blSelection=false;

	SetWindowText("Wave Editor [No Data]");
	Invalidate(true);
	}

	if(show)
	{
	ShowWindow(SW_SHOWNORMAL);
	SetActiveWindow();
	UpdateWindow();
	FitWave();
	}
}

void CSubMarco::OnSelectionZoom()
{
	if(blSelection && wdWave && blLength>8)
	{
		diStart=blStart;
		diLength=blLength;
		Invalidate();
		FitWave();
	}

}

void CSubMarco::OnSelectionZoomOut()
{
	if(diLength<wdLength)
	{
		diStart-=diLength;
		
		if(diStart<0)
		diStart=0;

		diLength=diLength*3;
		if(diLength+diStart>wdLength)diLength=wdLength-diStart;
		
		Invalidate();
		FitWave();
	}
}

void CSubMarco::OnSelectionFadeIn()
{
	if(blSelection && wdWave)
	{
		double fpass=1.0/blLength;
		double val=0.0f;

		for(int c=blStart;c<blStart+blLength;c++)
		{
			float const vl=*(wdLeft+c);

			*(wdLeft+c)=signed short(vl*val);
			
			if(wdStereo)
			{
				float const vr=*(wdRight+c);
				*(wdRight+c)=signed short(vl*val);
			}

		val+=fpass;
		}

	Invalidate(true);
	}		
}


void CSubMarco::OnSelectionFadeOut()
{
	if(blSelection && wdWave)
	{
		double fpass=1.0/blLength;
		double val=1.0f;

		for(int c=blStart;c<blStart+blLength;c++)
		{
		
			float const vl=*(wdLeft+c);

			*(wdLeft+c)=signed short(vl*val);
			
			if(wdStereo)
			{
				float const vr=*(wdRight+c);
				*(wdRight+c)=signed short(vl*val);
			}

			val-=fpass;
		}

	Invalidate(true);
	}		
}

CSubMarco::FitWave()
{
	if(blStart<0)blStart=0;
	if((blStart+blLength)>wdLength)blLength=wdLength-blStart;
}
