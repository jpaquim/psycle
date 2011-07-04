/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */

#include "stdafx.h"
#include "resource.h"
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
#include "ProgressDialog.h"




/////////////////////////////////////////////////////////////////////////////
// CProgressDialog dialog



CProgressDialog::CProgressDialog() 
{
	//Attach(GetActiveWindow());
	//m_Progress.Attach(GetDlgItem(IDC_PROGRESS1));
}



/////////////////////////////////////////////////////////////////////////////
// CProgressDialog message handlers

LRESULT CProgressDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
//	CDialogImpl::OnInitDialog();
	m_Progress.Attach(GetDlgItem(IDC_PROGRESS1));
	//m_Progress.Create();
	m_Progress.SetPos(0);
	m_Progress.SetRange(0,16384);
	m_Progress.SetPos(10000);
	CenterWindow(GetParent());
//	AfxGetApp()->DoWaitCursor(1);
	return TRUE;
}



void CProgressDialog::OnCancel()
{
	DestroyWindow();
	//AfxGetApp()->DoWaitCursor(-1); 
}

