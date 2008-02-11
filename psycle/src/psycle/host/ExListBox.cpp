/* -*- mode:c++, indent-tabs-mode:t -*- */
// ExListBox.cpp : implementation file
// Code originally from ran wainstein:
// http://www.codeproject.com/combobox/cexlistboc.asp
//

#include <psycle/project.private.hpp>
#include "ExListBox.h"
#include "psycle.hpp"
#include "MainFrm.hpp"
#include "Global.hpp"
#include "Song.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CExListBox
using namespace psycle::host;
CThisEdit::CThisEdit()
{
}
CThisEdit::~CThisEdit()
{
}
BEGIN_MESSAGE_MAP(CThisEdit, CEdit)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CThisEdit::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	///\todo: the "if" code doesn't work, because those keys never call this function :OO
	if ( nChar == VK_ESCAPE || nChar == VK_RETURN)
		GetParent()->SetFocus();
	else CEdit::OnKeyDown(nChar,nRepCnt,nFlags);
}


CExListBox::CExListBox()
{
}

CExListBox::~CExListBox()
{
}

BEGIN_MESSAGE_MAP(CExListBox, CListBox)
	ON_WM_RBUTTONDOWN()
	ON_EN_CHANGE(IDC_SEQEDITBOX, OnChangePatternName)
	ON_EN_KILLFOCUS(IDC_SEQEDITBOX,OnKillFocusPatternName)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CExListBox message handlers

void CExListBox::PreSubclassWindow() 
{
	CListBox::PreSubclassWindow();
	EnableToolTips(TRUE);
}
void CExListBox::OnRButtonDown( UINT nFlags, CPoint point )
{
	BOOL tmp = FALSE;
	int row = ItemFromPoint2(point,tmp);
	if ( row == -1)
		return;
	
	SetSel(row);
	HGDIOBJ hFont = GetStockObject( DEFAULT_GUI_FONT );
	CFont font;
	font.Attach( hFont );

	RECT cellrect;
	GetItemRect(row,&cellrect);

	myedit.DestroyWindow();
	myedit.Create(WS_CHILD|WS_BORDER,cellrect,this,IDC_SEQEDITBOX);
	myedit.SetFont(&font);
	myedit.SetWindowText(Global::_pSong->patternName[Global::_pSong->playOrder[row]]);
	myedit.ShowWindow(SW_SHOWNORMAL);
	myedit.SetFocus();
}
void CExListBox::OnKillFocusPatternName()
{
	((CMainFrame*)GetParentFrame())->UpdateSequencer();
	myedit.DestroyWindow();
}

void CExListBox::OnChangePatternName()
{
	CString string;
	myedit.GetWindowText(string);
	strncpy(Global::_pSong->patternName[Global::_pSong->playOrder[GetCurSel()]],string,32);
}
int CExListBox::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
	int row;
	RECT cellrect;   // cellrect		- to hold the bounding rect
	BOOL tmp = FALSE;
	row  = ItemFromPoint2(point,tmp);  //we call the ItemFromPoint function to determine the row,
	//note that in NT this function may fail  use the ItemFromPointNT member function

	if ( row == -1 ) 
		return -1;

	//set up the TOOLINFO structure. GetItemRect(row,&cellrect);
	GetItemRect(row,&cellrect);
	pTI->rect = cellrect;
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT)((row));   //The ‘uId’ is assigned a value according to the row value.
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	return pTI->uId;
}


//Define OnToolTipText(). This is the handler for the TTN_NEEDTEXT notification from 
//support ansi and unicode 
BOOL CExListBox::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	if (GetCount() == 0)
		return TRUE;
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText = Global::_pSong->patternName[Global::_pSong->playOrder[pNMHDR->idFrom]];
//	UINT nID = pNMHDR->idFrom;
//	GetText( nID ,strTipText);

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, strTipText, 80);
	else
		_mbstowcsz(pTTTW->szText, strTipText, 80);
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, 80);
	else
		lstrcpyn(pTTTW->szText, strTipText, 80);
#endif
	*pResult = 0;

	return TRUE;    
}

// CListBox::ItemFromPoint does not work on NT.
UINT CExListBox::ItemFromPoint2(CPoint pt, BOOL& bOutside) const
{
	int nFirstIndex, nLastIndex;
	//GetFirstAndLastIndex(nFirstIndex, nLastIndex);
	nFirstIndex = GetTopIndex();
	nLastIndex = nFirstIndex  + GetCount(); 
	
	bOutside = TRUE;
	
	CRect Rect;
	int nResult = -1;
	
	for (int i = nFirstIndex; nResult == -1 && i <= nLastIndex; i++)
	{
		if (GetItemRect(i, &Rect) != LB_ERR)
		{
			if (Rect.PtInRect(pt))
			{
				nResult  = i;
				bOutside = FALSE;
			}
		}
		
	}
	
	return nResult;
}

