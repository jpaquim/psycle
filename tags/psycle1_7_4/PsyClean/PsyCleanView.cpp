// PsyCleanView.cpp : implementation of the CPsyCleanView class
//

#include "stdafx.h"
#include "PsyClean.h"

#include "PsyCleanDoc.h"
#include "PsyCleanView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanView

IMPLEMENT_DYNCREATE(CPsyCleanView, CView)

BEGIN_MESSAGE_MAP(CPsyCleanView, CView)
	//{{AFX_MSG_MAP(CPsyCleanView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanView construction/destruction

CPsyCleanView::CPsyCleanView()
{
	// TODO: add construction code here

}

CPsyCleanView::~CPsyCleanView()
{
}

BOOL CPsyCleanView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanView drawing

void CPsyCleanView::OnDraw(CDC* pDC)
{
	CPsyCleanDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanView diagnostics

#ifdef _DEBUG
void CPsyCleanView::AssertValid() const
{
	CView::AssertValid();
}

void CPsyCleanView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPsyCleanDoc* CPsyCleanView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPsyCleanDoc)));
	return (CPsyCleanDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanView message handlers
