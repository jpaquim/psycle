// VstGui1.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "VstGui.h"
#include "Vst\AEffEditor.h"
#include "inputhandler.h"

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
	ON_WM_DESTROY()
	ON_WM_ENTERIDLE()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVstGui message handlers

void CVstGui::DoTheInit()
{
	effect->dispatcher(effect, effEditTop, 0, 0, NULL, 0.0f);
	SetTimer(0,25,NULL);
}

void CVstGui::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}

void CVstGui::OnDestroy()
{
	effect->dispatcher(effect,effEditClose,0,0,NULL,0.f);
	CFrameWnd::OnDestroy();
}

void CVstGui::OnEnterIdle(UINT nWhy, CWnd* pWho) 
{
	CFrameWnd::OnEnterIdle(nWhy, pWho);
	effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
}

void CVstGui::OnTimer(UINT nIDEvent)
{
	effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
	CFrameWnd::OnTimer(nIDEvent);
}

void CVstGui::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const BOOL bRepeat = nFlags&0x4000;
	CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
	if(!bRepeat && cmd.IsValid())
	{
		switch(cmd.GetType())
		{
		case CT_Note:
			{
				const int outnote = cmd.GetNote();
				Global::pInputHandler->PlayNote(outnote,127,true,_pMachine);
			}
			break;

		case CT_Immediate:
			Global::pInputHandler->PerformCmd(cmd,bRepeat);
			break;
		}
	}

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CVstGui::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);	
	const int outnote = cmd.GetNote();
	Global::pInputHandler->StopNote(outnote,true,_pMachine);
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}
