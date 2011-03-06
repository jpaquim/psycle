#pragma once
// ExListBox.h : header file
// Code originally from ran wainstein:
// http://www.codeproject.com/combobox/cexlistboc.asp
//
#include "Psycle.hpp"

namespace psycle { namespace host {

/////////////////////////////////////////////////////////////////////////////
// CExListBox window
class CThisEdit : public CEdit
{
public:
	CThisEdit();
	// Implementation
public:
	virtual ~CThisEdit();
	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

class CExListBox : public CListBox
{
// Construction
public:
	CExListBox();

// Attributes
public:
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnKillFocusPatternName();
	afx_msg void OnChangePatternName();
	INT_PTR OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	UINT ItemFromPoint2(CPoint pt, BOOL& bOutside) const;
	
	BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	// Operations
public:
	CThisEdit myedit;

// Overrides
	protected:
	virtual void PreSubclassWindow();

// Implementation
public:
	virtual ~CExListBox();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

}}
