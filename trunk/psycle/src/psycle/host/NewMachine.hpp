///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include "resources/resources.hpp"
#include "machine.hpp"
#include "ProgressDialog.hpp"
#include <afxwin.h>
#include <afxcoll.h>
#include <iostream>
#include <typeinfo>
#include <map>
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		const int MAX_BROWSER_NODES = 64;
		const int MAX_BROWSER_PLUGINS = 2048;

		enum selectionclasses
		{
			internal=0,
			native=1,
			vstmac=2
		};
		enum selectionmodes
		{
			modegen=0,
			modefx
		};
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
			virtual ~PluginInfo() throw()
			{
			}
			std::string dllname;
			long identifier;
			std::string error;
			MachineMode mode;
			MachineType type;
			std::string name;
			std::string vendor;
			std::string desc;
			std::string version;
			std::string APIversion;
			FILETIME FileTime;
			bool allow;
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
			virtual ~CNewMachine();

			enum { IDD = IDD_NEWMACHINE };
			CButton	m_Allow;
			CStatic	m_nameLabel;
			CTreeCtrl	m_browser;
			CStatic	m_versionLabel;
			CStatic	m_descLabel;
			int		m_orderby;
			CStatic	m_dllnameLabel;
			int		m_showdllName;
			CStatic m_APIversionLabel;
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		public:
			CImageList imgList;
			HTREEITEM tHand;
			int Outputmachine;
			std::string psOutputDll;
			int shellIdx;
			int OutBus;
			static int pluginOrder;
			static bool pluginName;

			static void learnDllName(const std::string & fullpath, MachineType type);
			static bool lookupDllName(const std::string, std::string & result, MachineType tye,int& shellIdx);
			static void DestroyPluginInfo();
			static void LoadPluginInfo(bool verify=true);
			static int selectedClass;
			static int selectedMode;
			static bool TestFilename(const std::string & name,int shellIdx);
		protected:
			static std::map<std::string,std::string> NativeNames;
			static std::map<std::string,std::string> VstNames;
			bool bAllowChanged;
			HTREEITEM hNodes[MAX_BROWSER_NODES];
			HTREEITEM hInt[6];
			HTREEITEM hPlug[MAX_BROWSER_PLUGINS];

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
			DECLARE_MESSAGE_MAP()
		private:
			static int _numPlugins;
			static PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
			static int _numDirs;
			static void FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress = 0);
			static bool LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount, bool verify);
			static bool SaveCacheFile();
			void UpdateList(bool bInit = false);
		public:
			afx_msg void OnEnChangeEdit1();
			afx_msg void OnStnClickedNamelabel();
			afx_msg void OnEnChangeRichedit21();
			afx_msg void OnBnClickedButton1();
		};

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
