#if !defined(AFX_NEWMACHINE_H__53B7A919_8301_4220_8487_445916AD32E8__INCLUDED_)
#define AFX_NEWMACHINE_H__53B7A919_8301_4220_8487_445916AD32E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewMachine.h : header file
//
#include "resource.h"
#include "machine.h"
#include <afxcoll.h>

#define MAX_BROWSER_NODES 72
#define MAX_BROWSER_PLUGINS 512

class PluginInfo
{
public:
	MachineMode mode;
	MachineType type;
	char *dllname;
	char name[64];
	char desc[64];
	char version[16];

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

class CNewMachine : public CDialog
{
// Construction
public:
	CImageList imgList;

	int Outputmachine;
	char* psOutputDll;
	int OutBus;

	static int pluginOrder;
	static bool pluginName;
	static CMapStringToString dllNames;
	static void DestroyPluginInfo();
	static void LoadPluginInfo(void);

	CNewMachine(CWnd* pParent = NULL);   // standard constructor
	~CNewMachine();

// Dialog Data
	//{{AFX_DATA(CNewMachine)
	enum { IDD = IDD_NEWMACHINE };
	CStatic	m_nameLabel;
	CTreeCtrl	m_browser;
	CStatic	m_versionLabel;
	CStatic	m_descLabel;
	int		m_orderby;
	CStatic	m_dllnameLabel;
	int		m_showdllName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewMachine)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	HTREEITEM hNodes[MAX_BROWSER_NODES];
	HTREEITEM hInt[9];
	HTREEITEM hPlug[MAX_BROWSER_PLUGINS];



	// Generated message map functions
	//{{AFX_MSG(CNewMachine)
	afx_msg void OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	afx_msg void OnByclass();
	afx_msg void OnBytype();
	afx_msg void OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnShowdllname();
	afx_msg void OnShoweffname();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static int _numPlugins;
	static PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
	static int _numDirs;
	static char *CNewMachine::_dirArray[MAX_BROWSER_NODES];

	static void FindPluginsInDir(int& currentPlugsCount,CString findDir,MachineType type);
	static bool LoadCacheFile(int& currentPlugsCount);
	static bool SaveCacheFile(int numPlugins);
	void UpdateList(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWMACHINE_H__53B7A919_8301_4220_8487_445916AD32E8__INCLUDED_)
