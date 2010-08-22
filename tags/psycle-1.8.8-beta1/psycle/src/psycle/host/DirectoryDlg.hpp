///\file
///\brief interface file for psycle::host::CDirectoryDlg.
#pragma once

#include "Psycle.hpp"
#include <afxwin.h>

namespace psycle { namespace host {

		/// directory config window.
		class CDirectoryDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(CDirectoryDlg)
		public:
			bool _instPathChanged;
			std::string _instPathBuf;
			bool _songPathChanged;
			std::string _songPathBuf;
			bool _pluginPathChanged;
			std::string _pluginPathBuf;
			bool _vstPath32Changed;
			std::string _vstPath32Buf;
			bool _vstPath64Changed;
			std::string _vstPath64Buf;
			bool _isJbridged;
			bool _isPsycleBridged;
			bool _skinPathChanged;
			std::string _skinPathBuf;
			bool _waveRecPathChanged;
			std::string _waveRecPathBuf;
			bool initializingDlg;
			CDirectoryDlg();
			virtual ~CDirectoryDlg();
		// Dialog Data
			enum { IDD = IDD_DIRECTORIES };
			CButton m_bridgeSupport;
			CButton m_jBridge;
			CButton m_PsycleVstBridge;
			CEdit	m_vst32Edit;
			CEdit	m_vst64Edit;
			CEdit	m_pluginEdit;
			CEdit	m_songEdit;
			CEdit	m_instEdit;
			CEdit	m_skinEdit;
			CEdit   m_waveRec;
		// Overrides
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		protected:
			void EnableSupportedBridges();
			void DisableAllBridges();
			bool BrowseForFolder(std::string& rpath);
			virtual BOOL OnInitDialog();
			afx_msg void OnBrowseInst();
			afx_msg void OnBrowseSong();
			afx_msg void OnBrowsePlugin();
			afx_msg void OnBrowseVst32();
			afx_msg void OnBrowseVst64();
			afx_msg void OnChangeSongedit();
			afx_msg void OnChangeInstedit();
			afx_msg void OnChangePluginedit();
			afx_msg void OnChangeVst32edit();
			afx_msg void OnChangeVst64edit();
			afx_msg void OnEnableBridge();
			afx_msg void OnEnableJBridge();
			afx_msg void OnEnablePsycleBridge();
			afx_msg void OnBridgesupport();
			afx_msg void OnBrowseSkin();
			afx_msg void OnChangeSkinedit();
			afx_msg void OnBnClickedBrowsewaverec();
			afx_msg void OnEnChangeWaverecedit();
			DECLARE_MESSAGE_MAP()
		public:
		};
	}   // namespace
}   // namespace
