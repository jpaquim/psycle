#pragma once
// ExListBox.h : header file
// Code originary from ran wainstein:
// http://www.codeproject.com/combobox/cexlistboc.asp
//

/////////////////////////////////////////////////////////////////////////////
// CExListBox window

class CExListBox : public CListBox
{
// Construction
public:
	CExListBox();

// Attributes
public:
	int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	UINT ItemFromPoint2(CPoint pt, BOOL& bOutside) const;
	
	BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	// Operations
public:

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

/////////////////////////////////////////////////////////////////////////////
