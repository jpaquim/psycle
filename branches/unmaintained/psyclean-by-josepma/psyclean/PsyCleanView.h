// PsyCleanView.h : interface of the CPsyCleanView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSYCLEANVIEW_H__57A36E51_F26B_4975_9199_42CFB52C51A6__INCLUDED_)
#define AFX_PSYCLEANVIEW_H__57A36E51_F26B_4975_9199_42CFB52C51A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPsyCleanView : public CView
{
protected: // create from serialization only
	CPsyCleanView();
	DECLARE_DYNCREATE(CPsyCleanView)

// Attributes
public:
	CPsyCleanDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPsyCleanView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPsyCleanView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPsyCleanView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in PsyCleanView.cpp
inline CPsyCleanDoc* CPsyCleanView::GetDocument()
   { return (CPsyCleanDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PSYCLEANVIEW_H__57A36E51_F26B_4975_9199_42CFB52C51A6__INCLUDED_)
