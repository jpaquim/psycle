#if !defined(AFX_NEWMACHINE_H__53B7A919_8301_4220_8487_445916AD32E8__INCLUDED_)
#define AFX_NEWMACHINE_H__53B7A919_8301_4220_8487_445916AD32E8__INCLUDED_

/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewMachine.h : header file
//
#include "resource.h"
#include "machine.h"
#include "ProgressDialog.h"

//#include <afxcoll.h>

#define MAX_BROWSER_NODES 64
#define MAX_BROWSER_PLUGINS 2048

class PluginInfo
{
public:
	MachineMode mode;
	MachineType type;
	TCHAR *dllname;
	TCHAR name[64];
	TCHAR desc[64];
	TCHAR version[16];
	FILETIME FileTime;
	bool allow;
	int error_type;

	PluginInfo()
	{
		dllname = NULL;
	}
	~PluginInfo()
	{
		if ( dllname != NULL ) delete dllname;
	}
/*	void operator=(PluginInfo& newinfo)
	{
		mode=newinfo.mode;
		type=newinfo.type;
		strcpy(version,newinfo.version);
		strcpy(name,newinfo.name);
		strcpy(desc,newinfo.desc);
		if (dllname != NULL ) delete dllname;
		dllname = new char[sizeof(newinfo.dllname)+1];
		strcpy(dllname,newinfo.dllname);
	}
	friend bool operator!=(PluginInfo& info1,PluginInfo& info2)
	{
		if ((info1.type != info2.type) ||
			(info1.mode != info2.mode) ||
			(strcmp(info1.version,info2.version) != 0 ) ||
			(strcmp(info1.desc,info2.desc) != 0 ) ||
			(strcmp(info1.name,info2.name) != 0 ) ||
			(strcmp(info1.dllname,info2.dllname) != 0)) return true;
		else return false;
	}
*/
};


/////////////////////////////////////////////////////////////////////////////
// CNewMachine dialog

typedef ATL::CAtlMap<CString, CString> CMapStringToString;

class CNewMachine : public CDialogImpl<CNewMachine> , public CWinDataExchange<CNewMachine>
{
// Construction
public:
	CImageList imgList;

	HTREEITEM tHand;

	int Outputmachine;
	TCHAR* psOutputDll;
	int OutBus;

	static int pluginOrder;
	static bool pluginName;
	static CMapStringToString dllNames;
	static void DestroyPluginInfo();
	static void LoadPluginInfo(void);
	static int LastType0;
	static int LastType1;

	CNewMachine();   // standard constructor
	~CNewMachine();

	static bool TestFilename(TCHAR* name);

	enum { IDD = IDD_NEWMACHINE };
	CButton	m_Allow;
	CStatic	m_nameLabel;
	WTL::CTreeViewCtrl	m_browser;
	CStatic	m_versionLabel;
	CStatic	m_descLabel;
	int		m_orderby;
	CStatic	m_dllnameLabel;
	int		m_showdllName;

	BEGIN_DDX_MAP(CNewMachine)
		DDX_RADIO(IDC_BYTYPE, m_orderby);
		DDX_RADIO(IDC_SHOWDLLNAME, m_showdllName);
	END_DDX_MAP()


// Implementation
protected:

	bool bAllowChanged;
	HTREEITEM hNodes[MAX_BROWSER_NODES];
	HTREEITEM hInt[3];
	HTREEITEM hPlug[MAX_BROWSER_PLUGINS];

	/*
	// Generated message map functions
	//{{AFX_MSG(CNewMachine)
	afx_msg void OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	afx_msg void OnByclass();
	afx_msg void OnBytype();
	virtual void OnOK();
	afx_msg void OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnShowdllname();
	afx_msg void OnShoweffname();
	afx_msg void OnCheckAllow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
		//{{AFX_MSG_MAP(CNewMachine)
		ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER, OnSelchangedBrowser)
		ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
		ON_BN_CLICKED(IDC_BYCLASS, OnByclass)
		ON_BN_CLICKED(IDC_BYTYPE, OnBytype)
		ON_NOTIFY(NM_DBLCLK, IDC_BROWSER, OnDblclkBrowser)
		ON_WM_DESTROY()
		ON_BN_CLICKED(IDC_SHOWDLLNAME, OnShowdllname)
		ON_BN_CLICKED(IDC_SHOWEFFNAME, OnShoweffname)
		ON_BN_CLICKED(IDC_CHECK_ALLOW, OnCheckAllow)
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()
	
	*/
private:
	static int _numPlugins;
	static PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
	static int _numDirs;

	static void FindPluginsInDir(int& currentPlugsCount,int &currentBadPlugsCount,CString findDir,MachineType type,CProgressDialog * pProgress = NULL);
	static bool LoadCacheFile(int& currentPlugsCount, int &currentBadPlugsCount);
	static bool SaveCacheFile();
	void UpdateList(bool bInit = false);
public:
	BEGIN_MSG_MAP(CNewMachine)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		COMMAND_HANDLER(IDC_SHOWDLLNAME, BN_CLICKED, OnBnClickedShowdllname)
		COMMAND_HANDLER(IDC_SHOWEFFNAME, BN_CLICKED, OnBnClickedShoweffname)
		NOTIFY_HANDLER(IDC_BROWSER, TVN_SELCHANGED, OnTvnSelchangedBrowser)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, OnBnClickedRefresh)
		COMMAND_HANDLER(IDC_BYTYPE, BN_CLICKED, OnBnClickedBytype)
		COMMAND_HANDLER(IDC_BYCLASS, BN_CLICKED, OnBnClickedByclass)
		COMMAND_HANDLER(IDC_CHECK_ALLOW, BN_CLICKED, OnBnClickedCheckAllow)
		NOTIFY_HANDLER(IDC_BROWSER, NM_DBLCLK, OnNMDblclkBrowser)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedShowdllname(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedShoweffname(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTvnSelchangedBrowser(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBytype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedByclass(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheckAllow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMDblclkBrowser(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWMACHINE_H__53B7A919_8301_4220_8487_445916AD32E8__INCLUDED_)
