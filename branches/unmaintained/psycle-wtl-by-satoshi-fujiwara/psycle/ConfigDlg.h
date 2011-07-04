#if !defined(AFX_CONFIGDLG_H__0F38D780_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
#define AFX_CONFIGDLG_H__0F38D780_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigDlg.h : header file
//
#include "configuration.h"
#include "DirectoryDlg.h"
#include "SkinDlg.h"
#include "OutputDlg.h"
#include "MidiInputDlg.h"
#include "KeyConfigDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CConfigDlg


class CConfigDlg : public CPropertySheetImpl<CConfigDlg> 
{
	//DECLARE_DYNAMIC(CConfigDlg)

// Construction
public:
	CConfigDlg( ATL::_U_STRINGorID title = (LPCTSTR) NULL, 
                        UINT uStartPage = 0, HWND hWndParent = NULL );
//	CConfigDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
//	CConfigDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CDirectoryDlg _dirDlg;
	CSkinDlg _skinDlg;
	COutputDlg _outputDlg;
	CMidiInputDlg _midiDlg;
	CKeyConfigDlg _keyDlg;

// Operations
public:
	void Init(configuration* pConfig,int dlgnum=0);

// Overrides
	public:
	virtual int DoModal();

// Implementation
public:
	virtual ~CConfigDlg();

	// Generated message map functions
    BEGIN_MSG_MAP_EX(CConfigDlg)
		COMMAND_HANDLER(IDOK,BN_CLICKED,OnOK);
        CHAIN_MSG_MAP(CPropertySheetImpl<CConfigDlg>)
    END_MSG_MAP()
protected:
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	configuration* _pConfig;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__0F38D780_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
