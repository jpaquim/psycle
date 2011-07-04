#pragma once
// ExListBox.h : header file
// Code originally from ran wainstein:
// http://www.codeproject.com/combobox/cexlistboc.asp
//
#include "Psycle.hpp"

namespace psycle { namespace host {

/////////////////////////////////////////////////////////////////////////////
// CExListBox window
class CExListBox : public CListBox
{
// Construction
public:
	CExListBox();
	virtual ~CExListBox();
	void ShowEditBox(bool isName);

// Attributes
public:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKillFocusPatternName();
	afx_msg void OnChangePatternName();
	INT_PTR OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	
	BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	// Operations
public:
	CEdit myedit;
	bool isName;
// Overrides
protected:
	virtual void PreSubclassWindow();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

}}
