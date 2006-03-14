/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */
#include "stdafx.h"
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
#include ".\newval.h"



/////////////////////////////////////////////////////////////////////////////
// CNewVal dialog


CNewVal::CNewVal()
{
	m_Value = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CNewVal message handlers

LRESULT CNewVal::OnEnUpdateEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  ���ꂪ RICHEDIT �R���g���[���̏ꍇ�A�܂��ACDialogImpl<CNewVal>::OnInitDialog() �֐����I�[�o�[���C�h���āA
	// OR ��Ԃ� ENM_CORRECTTEXT �t���O���}�X�N�ɓ���āA
	// OR ��Ԃ� ENM_CORRECTTEXT �t���O�� IParam �}�X�N�̂ɓ���āA
	// OR ��Ԃ� ENM_UPDATE �t���O�� lParam �}�X�N�ɓ���āA

	// TODO :  �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����Ă��������B
	// TODO: Add your control notification handler code here
	TCHAR buffer[256];
	m_value.GetWindowText(buffer,16);
	m_Value = _tstoi(buffer);

	if (m_Value < min)
	{
		m_Value = min;
		_stprintf(buffer,RES_STR(IDS_NEWVAL_1),paramindex,macindex,m_Value-min);
	}
	else if(m_Value > max)
	{
		m_Value = max;
		_stprintf(buffer,RES_STR(IDS_NEWVAL_2),paramindex,macindex,m_Value-min);
	}
	else
	{
		_stprintf(buffer,RES_STR(IDS_NEWVAL_3),paramindex,macindex,m_Value-min);
	}
	m_text.SetWindowText(buffer);
	return 0;
}

LRESULT CNewVal::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	// TODO: Add extra validation here
	TCHAR buffer[16];
	m_value.GetWindowText(buffer,16);
	m_Value=_tstoi(buffer);
	
	bHandled = FALSE;
	EndDialog(wID);
	return 0;
}

LRESULT CNewVal::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetWindowText(Title);
	m_value.Attach(GetDlgItem(IDC_EDIT1));
	m_text.Attach(GetDlgItem(IDC_TEXT));

	TCHAR buf[32];
	_stprintf(buf,_T("%d"),m_Value);
	m_value.SetWindowText(buf);
	m_value.SetSel(-1,-1,false);
	
	bHandled = FALSE;
	return TRUE;
}

LRESULT CNewVal::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

LRESULT CNewVal::OnEnSetfocusEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B

	return 0;
}
