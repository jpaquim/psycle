// GearScope.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearScope.h"
#include "ChildView.h"
#include "Machine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearScope dialog


CGearScope::CGearScope(CChildView* pParent /*=NULL*/)
	: CDialog(CGearScope::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CGearScope)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	doit=false;
}


void CGearScope::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearScope)
	DDX_Control(pDX, IDC_SLIDER, m_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearScope, CDialog)
	//{{AFX_MSG_MAP(CGearScope)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, OnCustomdrawSlider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearScope message handlers

BOOL CGearScope::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char buffer[64];
	sprintf(buffer,_pMachine->_editName);
	SetWindowText(buffer);
	SetTimer(2374,50,0);

	_pMachine->bCanDraw = TRUE;
	CClientDC dc(this);
	rc.top = 2;
	rc.left = 2;
	rc.bottom = 128+rc.top;
	rc.right = 256+rc.left;
	bmpDC = new CBitmap;
	bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);

	CDC bufDC;
	bufDC.CreateCompatibleDC(&dc);
	CBitmap* oldbmp;
	oldbmp = bufDC.SelectObject(bmpDC);

	bufDC.FillSolidRect(0,0,rc.right-rc.left,rc.bottom-rc.top,0);

	bufDC.SelectObject(oldbmp);
	bufDC.DeleteDC();

	m_slider.SetRange(1, 32);
	m_slider.SetPos(4);
	div = 4;

	return TRUE;
}


BOOL CGearScope::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearScope::OnCancel()
{
	_pMachine->bCanDraw = FALSE;
	m_pParent->ScopeMachineDialog = NULL;
	KillTimer(2374);
	DestroyWindow();
	bmpDC->DeleteObject();
	delete this;
}

inline int CGearScope::GetY(float f)
{
	f*=(64.0f/32768.0f);
	f+=64;
	if (f < 0) return 0;
	else if (f > 127) return 127;
	return f2i(f);
}

void CGearScope::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 2374 )
	{
		CClientDC dc(this);

		CDC bufDC;
		bufDC.CreateCompatibleDC(&dc);
		CBitmap* oldbmp;
		oldbmp = bufDC.SelectObject(bmpDC);

		bufDC.FillSolidRect(0,0,rc.right-rc.left,rc.bottom-rc.top,0);
		CPen linepen1( PS_SOLID, 2, 0x80c080);
		CPen linepen2( PS_SOLID, 2, 0xc08080);
		CPen linepen3( PS_SOLID, 4, 0x00404040);
		CPen linepen4( PS_SOLID, 8, 0x00202020);
		CPen *oldpen = bufDC.SelectObject(&linepen4);

		// now draw our scope

		_pMachine->bCanDraw = FALSE;
		bufDC.MoveTo(0,64);
		bufDC.LineTo(255,64);

		bufDC.SelectObject(&linepen3);
		bufDC.LineTo(0,64);

		bufDC.SelectObject(&linepen1);

		bufDC.MoveTo(0,GetY(_pMachine->bufL[0]));
		for (int x = 1; x < SCOPE_BUF_SIZE; x+=SCOPE_BUF_SIZE/64)
		{
			bufDC.LineTo(x/(SCOPE_BUF_SIZE/256),GetY(_pMachine->bufL[(x/div)]));
		}

		bufDC.SelectObject(&linepen2);
		bufDC.MoveTo(0,GetY(_pMachine->bufR[0]));
		for (x = 1; x < SCOPE_BUF_SIZE; x+=SCOPE_BUF_SIZE/64)
		{
			bufDC.LineTo(x/(SCOPE_BUF_SIZE/256),GetY(_pMachine->bufR[(x/div)]));
		}

		_pMachine->bCanDraw = TRUE;

		// and debuffer
		dc.BitBlt(rc.top,rc.left,rc.right-rc.left,rc.bottom-rc.top,&bufDC,0,0,SRCCOPY);

		bufDC.SelectObject(oldpen);
		linepen1.DeleteObject();
		linepen2.DeleteObject();
		linepen3.DeleteObject();
		linepen4.DeleteObject();

		bufDC.SelectObject(oldbmp);
		bufDC.DeleteDC();
//		m_pParent->SetFocus();
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CGearScope::OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	div = m_slider.GetPos();
	m_pParent->SetFocus();	
	*pResult = 0;
}
