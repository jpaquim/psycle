///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include "Psycle.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/machinekey.hpp>
using namespace psy::core;
#else
#include "Machine.hpp"
#endif
#include "ProgressDialog.hpp"
#include <afxwin.h>
#include <afxcoll.h>
#include <iostream>
#include <typeinfo>
#include <map>
#include <cstdint>
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		const int MAX_BROWSER_NODES = 64;
		const int MAX_BROWSER_PLUGINS = 2048;

		// type Hosts::type
		// Allows to differentiate between machines of different hosts.
		namespace Hosts {
			enum type
			{
				INTERNAL=0,
				NATIVE,
				LADSPA,
				VST,
				//Keep at last position
				NUM_HOSTS
			};
		}
		namespace MachineRole {
			enum type {
				GENERATOR = 0,
				EFFECT,
				MASTER,
				CONTROLLER
			};
		}

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
			std::uint32_t APIversion;
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
#endif //#if !PSYCLE__CONFIGURATION__USE_PSYCORE

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
			CStatic	m_dllnameLabel;
			CStatic m_APIversionLabel;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		public:
			CImageList imgList;
			HTREEITEM tHand;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			MachineKey outputMachine;
#else
			int Outputmachine;
			std::string psOutputDll;
			int shellIdx;
#endif

			enum { groupHost = 0, groupRole = 1 };
			enum { displayDll = 0, displayDesc = 1 };
			static int machineGrouping;
			static int displayName;

#if !PSYCLE__CONFIGURATION__USE_PSYCORE
			static void learnDllName(const std::string & fullpath, MachineType type);
			static bool lookupDllName(const std::string, std::string & result, MachineType tye,int& shellIdx);
			static void DestroyPluginInfo();
			static void LoadPluginInfo(bool verify=true);
#endif

			static int selectedGroup;
			static int selectedRole;

			static bool TestFilename(const std::string & name,int shellIdx);
		protected:
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			std::map<HTREEITEM,MachineKey> treeToInfo;
#else
			static std::map<std::string,std::string> NativeNames;
			static std::map<std::string,std::string> VstNames;
			bool bAllowChanged;
			HTREEITEM hNodes[MAX_BROWSER_NODES];
			HTREEITEM hInt[6];
			HTREEITEM hPlug[MAX_BROWSER_PLUGINS];
#endif

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
#if !PSYCLE__CONFIGURATION__USE_PSYCORE
			static int _numPlugins;
			static PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
			static void FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress = 0);
			static bool LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount, bool verify);
			static bool SaveCacheFile();
#endif
			void UpdateList(bool bInit = false);
		public:
			afx_msg void OnEnChangeEdit1();
			afx_msg void OnStnClickedNamelabel();
			afx_msg void OnEnChangeRichedit21();
			afx_msg void OnBnClickedButton1();
		};

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
