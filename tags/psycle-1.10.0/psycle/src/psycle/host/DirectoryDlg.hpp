///\file
///\brief interface file for psycle::host::CDirectoryDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {

		/// directory config window.
		class CDirectoryDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(CDirectoryDlg)
		public:
			CDirectoryDlg();
			virtual ~CDirectoryDlg();
			enum { IDD = IDD_DIRECTORIES };
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			virtual void OnOK( );
		protected:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnBrowseSong();
			afx_msg void OnChangeSongedit();
			afx_msg void OnBnClickedBrowsewaverec();
			afx_msg void OnEnChangeWaverecedit();
			afx_msg void OnBrowseInst();
			afx_msg void OnChangeInstedit();
			afx_msg void OnBrowsePlugin();
			afx_msg void OnChangePluginedit();
			afx_msg void OnBrowseVst32();
			afx_msg void OnChangeVst32edit();
			afx_msg void OnBrowseVst64();
			afx_msg void OnChangeVst64edit();
			afx_msg void OnEnableBridge();
			afx_msg void OnEnableJBridge();
			afx_msg void OnEnablePsycleBridge();
			afx_msg void OnBrowseSkin();
			afx_msg void OnChangeSkinedit();
			afx_msg void OnBrowsePresets();
			afx_msg void OnChangePresetsedit();
		protected:
			void EnableSupportedBridges();
			void DisableAllBridges();
		protected:
			CEdit	m_songEdit;
			CEdit   m_waveRec;
			CEdit	m_instEdit;
			CEdit	m_pluginEdit;
			CEdit	m_vst32Edit;
			CEdit	m_vst64Edit;
			CButton m_bridgeSupport;
			CButton m_jBridge;
			CButton m_PsycleVstBridge;
			CEdit	m_skinEdit;
			CEdit	m_presetEdit;

			bool initializingDlg;
			bool _songPathChanged;
			std::string _songPathBuf;
			bool _waveRecPathChanged;
			std::string _waveRecPathBuf;
			bool _instPathChanged;
			std::string _instPathBuf;
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
			bool _presetPathChanged;
			std::string _presetPathBuf;
		};
	}   // namespace
}   // namespace
