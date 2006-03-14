// PsyCleanDoc.cpp : implementation of the CPsyCleanDoc class
//

#include "stdafx.h"
#include "PsyClean.h"

#include "PsyCleanDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanDoc

IMPLEMENT_DYNCREATE(CPsyCleanDoc, CDocument)

BEGIN_MESSAGE_MAP(CPsyCleanDoc, CDocument)
	//{{AFX_MSG_MAP(CPsyCleanDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanDoc construction/destruction

CPsyCleanDoc::CPsyCleanDoc()
{
	// TODO: add one-time construction code here

}

CPsyCleanDoc::~CPsyCleanDoc()
{
}

BOOL CPsyCleanDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CPsyCleanDoc serialization

void CPsyCleanDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanDoc diagnostics

#ifdef _DEBUG
void CPsyCleanDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPsyCleanDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPsyCleanDoc commands
