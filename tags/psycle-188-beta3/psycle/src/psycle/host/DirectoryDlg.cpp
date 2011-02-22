///\file
///\brief implementation file for psycle::host::CDirectoryDlg.

#include "DirectoryDlg.hpp"
#include "Psycle.hpp"
#include <seib-Vsthost/JBridgeEnabler.hpp>

namespace psycle { namespace host {

		IMPLEMENT_DYNCREATE(CDirectoryDlg, CPropertyPage)

		CDirectoryDlg::CDirectoryDlg() : CPropertyPage(CDirectoryDlg::IDD)
		{
			_instPathChanged = false;
			_instPathBuf[0] = '\0';
			_songPathChanged = false;
			_songPathBuf[0] = '\0';
			_pluginPathChanged = false;
			_pluginPathBuf[0] = '\0';
			_vstPath32Changed = false;
			_vstPath32Buf[0] = '\0';
			_vstPath64Changed = false;
			_vstPath64Buf[0] = '\0';
			_isJbridged = false;
			_isPsycleBridged = false;
			_skinPathChanged = false;
			_skinPathBuf[0] = '\0';
			_waveRecPathChanged = false;
			_waveRecPathBuf[0] = '\0';
			initializingDlg = false;
		}

		CDirectoryDlg::~CDirectoryDlg()
		{
		}

		void CDirectoryDlg::DoDataExchange(CDataExchange* pDX)
		{
			CPropertyPage::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_VSTEDIT32, m_vst32Edit);
			DDX_Control(pDX, IDC_VSTEDIT64, m_vst64Edit);
			DDX_Control(pDX, IDC_ENABLEBRIDGE, m_bridgeSupport);
			DDX_Control(pDX, IDC_JBRIDGE, m_jBridge);
			DDX_Control(pDX, IDC_PSYCLEVSTBRIDGE, m_PsycleVstBridge);
			DDX_Control(pDX, IDC_PLUGINEDIT, m_pluginEdit);
			DDX_Control(pDX, IDC_SONGEDIT, m_songEdit);
			DDX_Control(pDX, IDC_INSTEDIT, m_instEdit);
			DDX_Control(pDX, IDC_SKINEDIT, m_skinEdit);
			DDX_Control(pDX, IDC_WAVERECEDIT, m_waveRec);
		}

		BEGIN_MESSAGE_MAP(CDirectoryDlg, CPropertyPage)
			ON_BN_CLICKED(IDC_BROWSEINST, OnBrowseInst)
			ON_BN_CLICKED(IDC_BROWSESONG, OnBrowseSong)
			ON_BN_CLICKED(IDC_BROWSEPLUGIN, OnBrowsePlugin)
			ON_BN_CLICKED(IDC_BROWSEVST32, OnBrowseVst32)
			ON_BN_CLICKED(IDC_BROWSEVST64, OnBrowseVst64)
			ON_BN_CLICKED(IDC_ENABLEBRIDGE, OnEnableBridge)
			ON_BN_CLICKED(IDC_JBRIDGE, OnEnableJBridge)
			ON_BN_CLICKED(IDC_PSYCLEVSTBRIDGE, OnEnablePsycleBridge)
			ON_EN_CHANGE(IDC_SONGEDIT, OnChangeSongedit)
			ON_EN_CHANGE(IDC_INSTEDIT, OnChangeInstedit)
			ON_EN_CHANGE(IDC_PLUGINEDIT, OnChangePluginedit)
			ON_EN_CHANGE(IDC_VSTEDIT32, OnChangeVst32edit)
			ON_EN_CHANGE(IDC_VSTEDIT64, OnChangeVst64edit)
			ON_BN_CLICKED(IDC_BROWSESKIN, OnBrowseSkin)
			ON_EN_CHANGE(IDC_SKINEDIT, OnChangeSkinedit)
			ON_BN_CLICKED(IDC_BROWSEWAVEREC, OnBnClickedBrowsewaverec)
			ON_EN_CHANGE(IDC_WAVERECEDIT, OnEnChangeWaverecedit)
		END_MESSAGE_MAP()

		/////////////////////////////////////////////////////////////////////////////
		// CDirectoryDlg message handlers

		void CDirectoryDlg::OnBrowseInst() 
		{
			if (BrowseForFolder(_instPathBuf))
			{
				_instPathChanged = true;
				m_instEdit.SetWindowText(_instPathBuf.c_str());
			}
		}

		void CDirectoryDlg::OnBrowseSong() 
		{
			if (BrowseForFolder(_songPathBuf))
			{
				_songPathChanged = true;
				m_songEdit.SetWindowText(_songPathBuf.c_str());
			}
		}

		void CDirectoryDlg::OnBrowsePlugin() 
		{
			if (BrowseForFolder(_pluginPathBuf))
			{
				_pluginPathChanged = true;
				m_pluginEdit.SetWindowText(_pluginPathBuf.c_str());
			}
		}

		void CDirectoryDlg::OnBrowseVst32() 
		{
			if (BrowseForFolder(_vstPath32Buf))
			{
				_vstPath32Changed = true;
				m_vst32Edit.SetWindowText(_vstPath32Buf.c_str());
			}
		}

		void CDirectoryDlg::OnBrowseVst64() 
		{
			if (BrowseForFolder(_vstPath64Buf))
			{
				_vstPath64Changed = true;
				m_vst64Edit.SetWindowText(_vstPath64Buf.c_str());
			}
		}

		bool CDirectoryDlg::BrowseForFolder(std::string& rpath) 
		{
			bool val=false;
			
			LPMALLOC pMalloc;
			// Gets the Shell's default allocator
			//
			if (::SHGetMalloc(&pMalloc) == NOERROR)
			{
				BROWSEINFO bi;
				char pszBuffer[MAX_PATH];
				LPITEMIDLIST pidl;
				// Get help on BROWSEINFO struct - it's got all the bit settings.
				//
				bi.hwndOwner = m_hWnd;
				bi.pidlRoot = NULL;
				bi.pszDisplayName = pszBuffer;
				bi.lpszTitle = _T("Select Directory");
				bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
				bi.lpfn = NULL;
				bi.lParam = 0;
				// This next call issues the dialog box.
				//
				if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
				{
					if (::SHGetPathFromIDList(pidl, pszBuffer))
					{
						// At this point pszBuffer contains the selected path
						//
						val = true;
						rpath =pszBuffer;
					}
					// Free the PIDL allocated by SHBrowseForFolder.
					//
					pMalloc->Free(pidl);
				}
				// Release the shell's allocator.
				//
				pMalloc->Release();
			}
			return val;
		}

		BOOL CDirectoryDlg::OnInitDialog() 
		{
			CPropertyPage::OnInitDialog();
			initializingDlg=true;
			m_instEdit.SetWindowText(_instPathBuf.c_str());
			m_songEdit.SetWindowText(_songPathBuf.c_str());
			m_pluginEdit.SetWindowText(_pluginPathBuf.c_str());
			m_vst32Edit.SetWindowText(_vstPath32Buf.c_str());
			if(IsWin64()) {
				m_vst64Edit.SetWindowText(_vstPath64Buf.c_str());
				EnableSupportedBridges();
				if(_isJbridged||_isPsycleBridged) {
					m_bridgeSupport.SetCheck(TRUE);
				}
				else {
					DisableAllBridges();
				}
				m_jBridge.SetCheck(_isJbridged);
				m_PsycleVstBridge.SetCheck(_isPsycleBridged);
			}
			else {
				DisableAllBridges();
				m_bridgeSupport.EnableWindow(FALSE);
				m_vst64Edit.EnableWindow(FALSE);
				CButton* cb=(CButton*)GetDlgItem(IDC_BROWSEVST64);
				cb->EnableWindow(FALSE);
			}
			m_skinEdit.SetWindowText(_skinPathBuf.c_str());
			m_waveRec.SetWindowText(_waveRecPathBuf.c_str());
			initializingDlg=false;
			
			return TRUE;  // return TRUE unless you set the focus to a control
						// EXCEPTION: OCX Property Pages should return FALSE
		}
		void CDirectoryDlg::EnableSupportedBridges() {
			char testjBridge[MAX_PATH];
			JBridge::getJBridgeLibrary(testjBridge);
			if (testjBridge[0]!='\0') {
				m_jBridge.EnableWindow();
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
				//Since jbridge is the only available bridge so far, disable bridging.
				m_bridgeSupport.EnableWindow(FALSE);
				DisableAllBridges();
			}
			//todo: not ready yet
			//m_PsycleVstBridge.EnableWindow();
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
			if (BrowseForFolder(_skinPathBuf))
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

		void CDirectoryDlg::OnBnClickedBrowsewaverec()
		{
			if (BrowseForFolder(_waveRecPathBuf))
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

	}   // namespace
}   // namespace


