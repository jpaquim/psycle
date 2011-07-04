// VstGui1.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "VstGui1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVstGui

IMPLEMENT_DYNCREATE(CVstGui, CFrameWnd)

CVstGui::CVstGui()
{
}

CVstGui::~CVstGui()
{
}


BEGIN_MESSAGE_MAP(CVstGui, CFrameWnd)
	//{{AFX_MSG_MAP(CVstGui)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_ENTERIDLE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVstGui message handlers

CVstGui::DoTheInit()
{
	effect->dispatcher(effect, effEditOpen, 0, 0, m_hWnd, 0.0f);

	ERect * er;
	
	effect->dispatcher(effect, effEditGetRect, 0, 0, &er, 0.0f);
	
	int const w =	(er->right - er->left) + GetSystemMetrics(SM_CXEDGE) * 2; // Borders
	int const h =	(er->bottom - er->top) + GetSystemMetrics(SM_CYCAPTION)
			+ GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE); // Bar + Menu
	
	SetWindowPos(&wndTop,0,0,w+6,h+9,SWP_NOMOVE);
	SetWindowText(cWndTitle);
	effect->dispatcher(effect, effEditTop, 0, 0, NULL, 0.0f);
	SetTimer(0,30,NULL);
}

void CVstGui::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}

void CVstGui::OnClose()
{
	effect->dispatcher(effect,effEditClose,0,0,NULL,0.f);
	*editorOpen=false;
	DestroyWindow();
}

void CVstGui::OnEnterIdle(UINT nWhy, CWnd* pWho) 
{
	CFrameWnd::OnEnterIdle(nWhy, pWho);
	effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
}

void CVstGui::OnTimer(UINT nIDEvent)
{
effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
}