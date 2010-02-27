#pragma once
#include "Psycle.hpp"

namespace psycle { namespace host {

/// CExListBox window
/// ExListBox.h : header file
/// Code originally from ran wainstein:
/// http://www.codeproject.com/combobox/cexlistboc.asp
class CThisEdit : public CEdit {
	public:
		afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );

	// Generated message map functions
	protected:
		DECLARE_MESSAGE_MAP()
};

class CExListBox : public CListBox {
	// Attributes
	public:
		afx_msg void OnRButtonDown( UINT nFlags, CPoint point);
		afx_msg void OnKillFocusPatternName();
		afx_msg void OnChangePatternName();
		int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
		UINT ItemFromPoint2(CPoint pt, BOOL& bOutside) const;
		
		BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult);

	// Operations
	public:
		CThisEdit myedit;

	// Overrides
	protected:
		virtual void PreSubclassWindow();

	// Implementation
	public:
		virtual ~CExListBox() {}

	// Generated message map functions
	protected:
		DECLARE_MESSAGE_MAP()
};

}}
