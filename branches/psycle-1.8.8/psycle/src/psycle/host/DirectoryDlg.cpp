///\file
///\brief implementation file for psycle::host::CDirectoryDlg.

#include <psycle/host/detail/project.private.hpp>
#include "DirectoryDlg.hpp"
#include "PsycleConfig.hpp"

namespace psycle { namespace host {

		IMPLEMENT_DYNCREATE(CDirectoryDlg, CPropertyPage)

		CDirectoryDlg::CDirectoryDlg() : CPropertyPage(CDirectoryDlg::IDD)
			, initializingDlg(false)
			, _songPathChanged(false)
			, _waveRecPathChanged(false)
			, _instPathChanged(false)
			, _pluginPathChanged(false)
			, _vstPath32Changed(false)
			, _vstPath64Changed(false)
			, _isJbridged(false)
			, _isPsycleBridged(false)
			, _skinPathChanged(false)
			, _presetPathChanged(false)
		{
		}

		CDirectoryDlg::~CDirectoryDlg()
		{
		}

		void CDirectoryDlg::DoDataExchange(CDataExchange* pDX)
		{
			CPropertyPage::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_SONGEDIT, m_songEdit);
			DDX_Control(pDX, IDC_WAVERECEDIT, m_waveRec);
			DDX_Control(pDX, IDC_INSTEDIT, m_instEdit);
			DDX_Control(pDX, IDC_PLUGINEDIT, m_pluginEdit);
			DDX_Control(pDX, IDC_VSTEDIT32, m_vst32Edit);
			DDX_Control(pDX, IDC_VSTEDIT64, m_vst64Edit);
			DDX_Control(pDX, IDC_ENABLEBRIDGE, m_bridgeSupport);
			DDX_Control(pDX, IDC_JBRIDGE, m_jBridge);
			DDX_Control(pDX, IDC_PSYCLEVSTBRIDGE, m_PsycleVstBridge);
			DDX_Control(pDX, IDC_SKINEDIT, m_skinEdit);
			DDX_Control(pDX, IDC_USERPRESETS_EDIT, m_presetEdit);
		}

		BEGIN_MESSAGE_MAP(CDirectoryDlg, CPropertyPage)
			ON_BN_CLICKED(IDC_BROWSESONG, OnBrowseSong)
			ON_EN_CHANGE(IDC_SONGEDIT, OnChangeSongedit)
			ON_BN_CLICKED(IDC_BROWSEWAVEREC, OnBnClickedBrowsewaverec)
			ON_EN_CHANGE(IDC_WAVERECEDIT, OnEnChangeWaverecedit)
			ON_BN_CLICKED(IDC_BROWSEINST, OnBrowseInst)
			ON_EN_CHANGE(IDC_INSTEDIT, OnChangeInstedit)
			ON_BN_CLICKED(IDC_BROWSEPLUGIN, OnBrowsePlugin)
			ON_EN_CHANGE(IDC_PLUGINEDIT, OnChangePluginedit)
			ON_BN_CLICKED(IDC_BROWSEVST32, OnBrowseVst32)
			ON_EN_CHANGE(IDC_VSTEDIT32, OnChangeVst32edit)
			ON_BN_CLICKED(IDC_BROWSEVST64, OnBrowseVst64)
			ON_EN_CHANGE(IDC_VSTEDIT64, OnChangeVst64edit)
			ON_BN_CLICKED(IDC_ENABLEBRIDGE, OnEnableBridge)
			ON_BN_CLICKED(IDC_JBRIDGE, OnEnableJBridge)
			ON_BN_CLICKED(IDC_PSYCLEVSTBRIDGE, OnEnablePsycleBridge)
			ON_BN_CLICKED(IDC_BROWSESKIN, OnBrowseSkin)
			ON_EN_CHANGE(IDC_SKINEDIT, OnChangeSkinedit)
			ON_BN_CLICKED(IDC_BROWSE_USERPRESET, OnBrowsePresets)
			ON_EN_CHANGE(IDC_USERPRESETS_EDIT, OnChangePresetsedit)
		END_MESSAGE_MAP()

		/////////////////////////////////////////////////////////////////////////////
		// CDirectoryDlg message handlers

		BOOL CDirectoryDlg::OnInitDialog() 
		{
			CPropertyPage::OnInitDialog();
			PsycleConfig& config = Global::psycleconf();
			initializingDlg=true;

			_songPathBuf    = config.GetSongDir();
			_waveRecPathBuf = config.GetWaveRecDir();
			_instPathBuf    = config.GetInstrumentDir();
			_pluginPathBuf  = config.GetPluginDir();
			_vstPath32Buf   = config.GetVst32Dir();
			_vstPath64Buf   = config.GetVst64Dir();
			_isJbridged     = config.UsesJBridge();
			_isPsycleBridged = config.UsesPsycleVstBridge();
			_skinPathBuf    = config.GetSkinDir();
			_presetPathBuf  = config.GetPresetsDir();

			m_songEdit.SetWindowText(_songPathBuf.c_str());
			m_waveRec.SetWindowText(_waveRecPathBuf.c_str());
			m_instEdit.SetWindowText(_instPathBuf.c_str());
			m_pluginEdit.SetWindowText(_pluginPathBuf.c_str());
			m_vst32Edit.SetWindowText(_vstPath32Buf.c_str());
			m_vst64Edit.SetWindowText(_vstPath64Buf.c_str());
			m_skinEdit.SetWindowText(_skinPathBuf.c_str());
			m_presetEdit.SetWindowText(_presetPathBuf.c_str());

			EnableSupportedBridges();
			m_jBridge.SetCheck(_isJbridged);
			m_PsycleVstBridge.SetCheck(_isPsycleBridged);
			initializingDlg=false;
			
			return TRUE;  // return TRUE unless you set the focus to a control
						// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CDirectoryDlg::OnOK()
		{
			PsycleConfig& config = Global::psycleconf();
			if (_songPathChanged)    config.SetSongDir(_songPathBuf);
			if (_waveRecPathChanged) config.SetWaveRecDir(_waveRecPathBuf);
			if (_instPathChanged)    config.SetInstrumentDir(_instPathBuf);
			if (_pluginPathChanged)  config.SetPluginDir(_pluginPathBuf);
			if (_vstPath32Changed)   config.SetVst32Dir(_vstPath32Buf);
			if (_vstPath64Changed)   config.SetVst64Dir(_vstPath64Buf);
			                         config.UseJBridge(_isJbridged);
			                         config.UsePsycleVstBridge(_isPsycleBridged);
			if (_skinPathChanged)    config.SetSkinDir(_skinPathBuf);
			if (_presetPathChanged)  config.SetPresetsDir(_presetPathBuf);
		}
		void CDirectoryDlg::OnBrowseSong() 
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select Song Directory"), _songPathBuf))
			{
				_songPathChanged = true;
				m_songEdit.SetWindowText(_songPathBuf.c_str());
			}
		}
		void CDirectoryDlg::OnChangeSongedit() 
		{
			if (!initializingDlg)
			{
				_songPathChanged = true;
				CString temp;
				m_songEdit.GetWindowText(temp);
				_songPathBuf = temp;
			}
		}

		void CDirectoryDlg::OnBnClickedBrowsewaverec()
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select Recording Directory"), _waveRecPathBuf))
			{
				_waveRecPathChanged = true;
				m_waveRec.SetWindowText(_waveRecPathBuf.c_str());
			}
		}
		void CDirectoryDlg::OnEnChangeWaverecedit()
		{
			if (!initializingDlg)
			{
				_waveRecPathChanged = true;
				CString temp;
				m_waveRec.GetWindowText(temp);
				_waveRecPathBuf=temp;
			}
		}

		void CDirectoryDlg::OnBrowseInst() 
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select Instrument Directory"), _instPathBuf))
			{
				_instPathChanged = true;
				m_instEdit.SetWindowText(_instPathBuf.c_str());
			}
		}
		void CDirectoryDlg::OnChangeInstedit() 
		{
			if (!initializingDlg)
			{
				_instPathChanged = true;
				CString temp;
				m_instEdit.GetWindowText(temp);
				_instPathBuf = temp;
			}
		}

		void CDirectoryDlg::OnBrowsePlugin() 
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select Native Plugin Directory"), _pluginPathBuf))
			{
				_pluginPathChanged = true;
				m_pluginEdit.SetWindowText(_pluginPathBuf.c_str());
			}
		}
		void CDirectoryDlg::OnChangePluginedit() 
		{
			if (!initializingDlg)
			{
				_pluginPathChanged = true;
				CString temp;
				m_pluginEdit.GetWindowText(temp);
				_pluginPathBuf = temp;
			}
		}

		void CDirectoryDlg::OnBrowseVst32() 
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select 32bit VST Directory"), _vstPath32Buf))
			{
				_vstPath32Changed = true;
				m_vst32Edit.SetWindowText(_vstPath32Buf.c_str());
			}
		}
		void CDirectoryDlg::OnChangeVst32edit() 
		{
			if (!initializingDlg)
			{
				_vstPath32Changed = true;
				CString temp;
				m_vst32Edit.GetWindowText(temp);
				_vstPath32Buf=temp;
			}
		}

		void CDirectoryDlg::OnBrowseVst64() 
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select 64bit VST Directory"), _vstPath64Buf))
			{
				_vstPath64Changed = true;
				m_vst64Edit.SetWindowText(_vstPath64Buf.c_str());
			}
		}
		void CDirectoryDlg::OnChangeVst64edit() 
		{
			if (!initializingDlg)
			{
				_vstPath64Changed = true;
				CString temp;
				m_vst64Edit.GetWindowText(temp);
				_vstPath64Buf=temp;
			}
		}

		void CDirectoryDlg::OnEnableBridge() 
		{
			if(m_bridgeSupport.GetCheck()) {
				EnableSupportedBridges();
			}
			else {
				DisableAllBridges();
			}
		}
		void CDirectoryDlg::OnEnableJBridge()
		{
			_isJbridged=m_jBridge.GetCheck();
		}
		void CDirectoryDlg::OnEnablePsycleBridge()
		{
			_isPsycleBridged=m_PsycleVstBridge.GetCheck();
		}

		void CDirectoryDlg::OnBrowseSkin() 
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select Skin Directory"), _skinPathBuf))
			{
				_skinPathChanged = true;
				m_skinEdit.SetWindowText(_skinPathBuf.c_str());
			}
		}
		void CDirectoryDlg::OnChangeSkinedit() 
		{
			if (!initializingDlg)
			{
				_skinPathChanged = true;
				CString temp;
				m_skinEdit.GetWindowText(temp);
				_skinPathBuf=temp;
			}
		}
		void CDirectoryDlg::OnBrowsePresets() 
		{
			if (CPsycleApp::BrowseForFolder(m_hWnd, _T("Select Preset Directory"), _presetPathBuf))
			{
				_presetPathChanged = true;
				m_presetEdit.SetWindowText(_presetPathBuf.c_str());
			}
		}
		void CDirectoryDlg::OnChangePresetsedit() 
		{
			if (!initializingDlg)
			{
				_presetPathChanged = true;
				CString temp;
				m_presetEdit.GetWindowText(temp);
				_presetPathBuf=temp;
			}
		}

		void CDirectoryDlg::EnableSupportedBridges() {
			bool bridging = false;
			if (Global::psycleconf().SupportsJBridge())
			{
				m_jBridge.EnableWindow();
				bridging = true;
			}
			else {
				//Since jbridge is the only available bridge so far, disable bridging.
				m_bridgeSupport.EnableWindow(FALSE);
			}
			//todo: not ready yet
			//m_PsycleVstBridge.EnableWindow();
			if(false) {
			}
			else {
				m_PsycleVstBridge.EnableWindow(FALSE);
			}

			if(bridging){
				if(_isJbridged||_isPsycleBridged) {
					m_bridgeSupport.SetCheck(TRUE);
				}
				if(IsWow64()) {
					m_vst64Edit.EnableWindow(TRUE);
					CButton* cb=(CButton*)GetDlgItem(IDC_BROWSEVST64);
					cb->EnableWindow(TRUE);
				} else if(IsWin64()) {
					m_vst32Edit.EnableWindow(TRUE);
					CButton* cb=(CButton*)GetDlgItem(IDC_BROWSEVST32);
					cb->EnableWindow(TRUE);
				}
			}
			else {
				//If there is no bridge available, disable bridging.
				m_bridgeSupport.EnableWindow(FALSE);
				DisableAllBridges();
				if(IsWow64()) {
					m_vst64Edit.EnableWindow(FALSE);
					CButton* cb=(CButton*)GetDlgItem(IDC_BROWSEVST64);
					cb->EnableWindow(FALSE);
				} else if(IsWin64()) {
					m_vst32Edit.EnableWindow(FALSE);
					CButton* cb=(CButton*)GetDlgItem(IDC_BROWSEVST32);
					cb->EnableWindow(FALSE);
				}
			}
		}
		void CDirectoryDlg::DisableAllBridges() {
			m_jBridge.EnableWindow(FALSE);
			m_jBridge.SetCheck(FALSE);
			_isJbridged=false;
			m_PsycleVstBridge.EnableWindow(FALSE);
			m_PsycleVstBridge.SetCheck(FALSE);
			_isPsycleBridged=false;
			if(IsWin64() && !IsWow64()) {
				m_vst32Edit.EnableWindow(FALSE);
				CButton* cb=(CButton*)GetDlgItem(IDC_BROWSEVST32);
				cb->EnableWindow(FALSE);
			} else {
				m_vst64Edit.EnableWindow(FALSE);
				CButton* cb=(CButton*)GetDlgItem(IDC_BROWSEVST64);
				cb->EnableWindow(FALSE);
			}
		}
	}   // namespace
}   // namespace


