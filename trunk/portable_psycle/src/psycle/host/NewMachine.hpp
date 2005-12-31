///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include "resources/resources.hpp"
#include "machine.hpp"
#include "ProgressDialog.hpp"
#include <afxcoll.h>
#include <iostream>
#include <typeinfo>
#include <map>
#include "afxwin.h"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		const int MAX_BROWSER_NODES = 64;
		const int MAX_BROWSER_PLUGINS = 2048;

		class PluginInfo
		{
		public:
			PluginInfo()
				: mode(MACHMODE_UNDEFINED)
				, type(MACH_UNDEFINED)
				, allow(true)
			{
				std::memset(&FileTime, 0, sizeof FileTime);
			}
			~PluginInfo() throw()
			{
			}
			std::string dllname;
			std::string error;
			MachineMode mode;
			MachineType type;
			std::string name;
			std::string desc;
			std::string version;
			FILETIME FileTime;
			bool allow;
			std::string category;
			/*
			void operator=(PluginInfo& newinfo)
			{
				mode=newinfo.mode;
				type=newinfo.type;
				strcpy(version,newinfo.version);
				strcpy(name,newinfo.name);
				strcpy(desc,newinfo.desc);
				zapArray(dllname,new char[sizeof(newinfo.dllname)+1]);
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
		
		/// new machine dialog window.
		class CNewMachine : public CDialog
		{
		public:
			CNewMachine(CWnd* pParent = 0);
			~CNewMachine();
			CImageList imgList;
			HTREEITEM tHand;
			int Outputmachine;
			std::string psOutputDll;
			int OutBus;
			static int pluginOrder;
			static bool pluginName;
			static void learnDllName(const std::string & fullpath);
			static bool lookupDllName(const std::string &, std::string & result);
			static void DestroyPluginInfo();
			static void LoadPluginInfo();
			static int LastType0;
			static int LastType1;
			static bool TestFilename(const std::string & name);
			
		// Dialog Data
			//{{AFX_DATA(CNewMachine)
			enum { IDD = IDD_NEWMACHINE };
			CButton	m_Allow;
			CStatic	m_nameLabel;
			CTreeCtrl	m_browser;
			CStatic	m_versionLabel;
			CStatic	m_descLabel;
			CStatic	m_dllnameLabel;
			CComboBox comboListStyle;
			CComboBox comboNameStyle;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CNewMachine)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			static std::map<std::string,std::string> dllNames;
			static std::map<CString, int> CustomFolders;
			bool bAllowChanged;
			bool bCategoriesChanged;
			HTREEITEM hNodes[MAX_BROWSER_NODES];
			HTREEITEM hInt[4];
			HTREEITEM hPlug[MAX_BROWSER_PLUGINS];
			HTREEITEM m_hItemDrag;
			// Generated message map functions
			//{{AFX_MSG(CNewMachine)
			afx_msg void OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult);
			virtual BOOL OnInitDialog();
			afx_msg void OnRefresh();
			virtual void OnOK();
			afx_msg void OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnDestroy();
			afx_msg void OnCheckAllow();
			afx_msg void OnCbnSelendokListstyle();
			afx_msg void OnCbnSelendokNamestyle();
			afx_msg void BeginDrag(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void BeginRDrag(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
			afx_msg void NMPOPUP_AddSubFolder();
			afx_msg void NMPOPUP_AddFolderSameLevel();
			afx_msg void NMPOPUP_RenameFolder();
			afx_msg void NMPOPUP_DeleteMoveToParent();
			afx_msg void NMPOPUP_DeleteMoveUncat();
			afx_msg void NMPOPUP_ExpandAll();
			afx_msg void NMPOPUP_CollapseAll();
			afx_msg void BeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void EndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void AlterItemCaption(NMHDR *pNMHDR, LRESULT *pResult);

			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnCancelMode();
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		private:
			static int _numPlugins;
			static PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
			static int _numDirs;
			static void FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress = 0);
			static bool LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount);
			static bool SaveCacheFile();
			void UpdateList(bool bInit = false);
			HTREEITEM CategoryExists (HTREEITEM hParent, CString category);
			void SortList() ;
			

		public:
			//afx_msg void AddNewCustomFolder();
			//CEdit txtNewFolderName;

			void FinishDragging(BOOL bDraggingImageList);
			void OnEndDrag(UINT nFlags, CPoint point);
			
			UINT nFlags;
			CPoint point;
			bool bEditing;

			afx_msg void BrowserKeyDown(NMHDR *pNMHDR, LRESULT *pResult);		
			HTREEITEM MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo, HTREEITEM hItemPos = TVI_SORT, bool bAllowReplace = false);
			afx_msg void NMPOPUP_MoveToTopLevel();
			void SetPluginCategories(HTREEITEM hItem, CString Category);

			HTREEITEM hCategory;

			afx_msg void OnBnClickedCancel();
			static int CALLBACK CNewMachine::NodeCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};




		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	NAMESPACE__END
NAMESPACE__END
