/** @file 
 *  @brief implementation file
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

#include "Vst\AEffEditor.h"
#include "inputhandler.h"
#include "MainFrm.h"
#include "Machine.h"
#include ".\vstgui.h"

//extern CPsycleApp theApp;
//IMPLEMENT_DYNCREATE(CVstGui, CFrameWnd)

/////////////////////////////////////////////////////////////////////////////
// CVstGui

//IMPLEMENT_DYNCREATE(CVstGui, CFrameWnd)


CVstGui::CVstGui()
{
}

CVstGui::~CVstGui()
{
}




/////////////////////////////////////////////////////////////////////////////
// CVstGui message handlers

void CVstGui::DoTheInit()
{
	m_pEffect->dispatcher(m_pEffect, effEditTop, 0, 0, NULL, 0.0f);
	SetTimer(0,25,NULL);
}


LRESULT CVstGui::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CPaintDC dc((HWND)(*this)); // device context for painting
	bHandled = FALSE;
	return 0;
}

LRESULT CVstGui::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	m_pEffect->dispatcher(m_pEffect,effEditClose,0,0,NULL,0.f);
	bHandled = FALSE;
	return 0;
}

LRESULT CVstGui::OnEnterIdle(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_pEffect->dispatcher(m_pEffect, effEditIdle, 0, 0, NULL, 0.0f);

	return 0;
}

LRESULT CVstGui::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_pEffect->dispatcher(m_pEffect, effEditIdle, 0, 0, NULL, 0.0f);

	return 0;
}

LRESULT CVstGui::OnKeyDown(UINT /*uMsg*/, WPARAM nChar, LPARAM nFlags, BOOL& /*bHandled*/)
{
	const BOOL bRepeat = nFlags & 0x4000;
	CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
	if(!bRepeat && cmd.IsValid())
	{
		switch(cmd.GetType())
		{
		case CT_Note:
			{
				const int outnote = cmd.GetNote();
				if ( m_pMachine->_mode == MACHMODE_GENERATOR || Global::pConfig->_notesToEffects)
				{
					Global::pInputHandler->PlayNote(outnote,127,true,m_pMachine);
				}
				else Global::pInputHandler->PlayNote(outnote,127,true);
			}
			break;

		case CT_Immediate:
			Global::pInputHandler->PerformCmd(cmd,bRepeat);
			break;
		}
	}
	SendMessage(CMainFrame::GetInstance().m_hWndClient,WM_KEYDOWN,nChar,nFlags);
	return 0;
}

LRESULT CVstGui::OnKeyUp(UINT /*uMsg*/, WPARAM nChar, LPARAM nFlags, BOOL& /*bHandled*/)
{
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);	
	const int outnote = cmd.GetNote();
	if ( m_pMachine->_mode == MACHMODE_GENERATOR ||Global::pConfig->_notesToEffects)
	{
		Global::pInputHandler->StopNote(outnote,true,m_pMachine);
	}
	else Global::pInputHandler->StopNote(outnote,true);

	SendMessage(CMainFrame::GetInstance().m_hWndClient,WM_KEYUP,nChar,nFlags);
	return 0;
}

LRESULT CVstGui::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	this->SetFocus();
	return 0;
}

void CVstGui::OnFinalMessage(HWND hWnd)
{
	// TODO : ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	CWindowImpl<CVstGui,IVstGui>::OnFinalMessage(hWnd);
}
