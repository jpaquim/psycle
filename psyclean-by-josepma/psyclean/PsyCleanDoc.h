// PsyCleanDoc.h : interface of the CPsyCleanDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSYCLEANDOC_H__9894A2F6_E7D5_4C11_9A2A_0B0EA6033ECD__INCLUDED_)
#define AFX_PSYCLEANDOC_H__9894A2F6_E7D5_4C11_9A2A_0B0EA6033ECD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPsyCleanDoc : public CDocument
{
protected: // create from serialization only
	CPsyCleanDoc();
	DECLARE_DYNCREATE(CPsyCleanDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPsyCleanDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPsyCleanDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPsyCleanDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PSYCLEANDOC_H__9894A2F6_E7D5_4C11_9A2A_0B0EA6033ECD__INCLUDED_)
