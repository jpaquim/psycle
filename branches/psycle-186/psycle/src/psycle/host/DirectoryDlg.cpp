///\file
///\brief implementation file for psycle::host::CDirectoryDlg.

#include "DirectoryDlg.hpp"
#include "Psycle.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		IMPLEMENT_DYNCREATE(CDirectoryDlg, CPropertyPage)

		CDirectoryDlg::CDirectoryDlg() : CPropertyPage(CDirectoryDlg::IDD)
		{
			//{{AFX_DATA_INIT(CDirectoryDlg)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
			_instPathChanged = false;
			_instPathBuf[0] = '\0';
			_songPathChanged = false;
			_songPathBuf[0] = '\0';
			_pluginPathChanged = false;
			_pluginPathBuf[0] = '\0';
			_vstPathChanged = false;
			_vstPathBuf[0] = '\0';
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
			//{{AFX_DATA_MAP(CDirectoryDlg)
			DDX_Control(pDX, IDC_VSTEDIT, m_vstEdit);
			DDX_Control(pDX, IDC_PLUGINEDIT, m_pluginEdit);
			DDX_Control(pDX, IDC_SONGEDIT, m_songEdit);
			DDX_Control(pDX, IDC_INSTEDIT, m_instEdit);
			DDX_Control(pDX, IDC_SKINEDIT, m_skinEdit);
			DDX_Control(pDX, IDC_WAVERECEDIT, m_waveRec);
			//}}AFX_DATA_MAP
		}


		BEGIN_MESSAGE_MAP(CDirectoryDlg, CPropertyPage)
			//{{AFX_MSG_MAP(CDirectoryDlg)
			ON_BN_CLICKED(IDC_BROWSEINST, OnBrowseInst)
			ON_BN_CLICKED(IDC_BROWSESONG, OnBrowseSong)
			ON_BN_CLICKED(IDC_BROWSEPLUGIN, OnBrowsePlugin)
			ON_BN_CLICKED(IDC_BROWSEVST, OnBrowseVst)
			ON_EN_CHANGE(IDC_SONGEDIT, OnChangeSongedit)
			ON_EN_CHANGE(IDC_INSTEDIT, OnChangeInstedit)
			ON_EN_CHANGE(IDC_PLUGINEDIT, OnChangePluginedit)
			ON_EN_CHANGE(IDC_VSTEDIT, OnChangeVstedit)
			ON_BN_CLICKED(IDC_BROWSESKIN, OnBrowseSkin)
			ON_EN_CHANGE(IDC_SKINEDIT, OnChangeSkinedit)
			ON_BN_CLICKED(IDC_BROWSEWAVEREC, OnBnClickedBrowsewaverec)
			ON_EN_CHANGE(IDC_WAVERECEDIT, OnEnChangeWaverecedit)
			//}}AFX_MSG_MAP
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

		void CDirectoryDlg::OnBrowseVst() 
		{
			if (BrowseForFolder(_vstPathBuf))
			{
				_vstPathChanged = true;
				m_vstEdit.SetWindowText(_vstPathBuf.c_str());
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
			m_vstEdit.SetWindowText(_vstPathBuf.c_str());
			m_skinEdit.SetWindowText(_skinPathBuf.c_str());
			m_waveRec.SetWindowText(_waveRecPathBuf.c_str());
			initializingDlg=false;
			
			return TRUE;  // return TRUE unless you set the focus to a control
						// EXCEPTION: OCX Property Pages should return FALSE
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

		void CDirectoryDlg::OnChangeVstedit() 
		{
			if (!initializingDlg)
			{
				_vstPathChanged = true;
				CString temp;
				m_vstEdit.GetWindowText(temp);
				_vstPathBuf=temp;
			}
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

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END


