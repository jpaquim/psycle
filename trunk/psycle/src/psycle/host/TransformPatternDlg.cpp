// TransformPatternDlg.cpp : implementation file
//
#include <psycle/project.private.hpp>
#include "TransformPatternDlg.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		// CTransformPatternDlg dialog

		IMPLEMENT_DYNAMIC(CTransformPatternDlg, CDialog)

		CTransformPatternDlg::CTransformPatternDlg(CChildView* pChildView, CWnd* pParent /*=NULL*/)
			: CDialog(CTransformPatternDlg::IDD, pParent)
		{
			_pChildView = pChildView;
		}

		CTransformPatternDlg::~CTransformPatternDlg()
		{
		}

		void CTransformPatternDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CTransformPatternDlg)
			DDX_Control(pDX, IDC_FILTERNOTE, m_filternote);
			DDX_Control(pDX, IDC_FILTERINS, m_filterins);
			DDX_Control(pDX, IDC_FILTERMAC, m_filtermac);
			DDX_Control(pDX, IDC_FILTERCMD, m_filtercmd);
			DDX_Control(pDX, IDC_REPLACENOTE, m_replacenote);
			DDX_Control(pDX, IDC_REPLACEINS, m_replaceins);
			DDX_Control(pDX, IDC_REPLACEMAC, m_replacemac);
			DDX_Control(pDX, IDC_REPLACECMD, m_replacecmd);
			DDX_Check(pDX, IDC_APPLYTOBLOCK, m_applytoblock);
			//}}AFX_DATA_MAP
		}


		BEGIN_MESSAGE_MAP(CTransformPatternDlg, CDialog)
		//{{AFX_MSG_MAP(CTransformPatternDlg)
			ON_BN_CLICKED(IDAPPLY, &CTransformPatternDlg::OnBnClickedApply)
		//}}AFX_MSG_MAP

		END_MESSAGE_MAP()

		BOOL CTransformPatternDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_filternote.EnableWindow(false);
			m_filtercmd.EnableWindow(false);
			m_replacenote.EnableWindow(false);
			m_replacecmd.EnableWindow(false);

			return true;  // return true unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return false
		}

		// CTransformPatternDlg message handlers

		void CTransformPatternDlg::OnBnClickedApply()
		{
			char afilternote[32];
			char afilterins[32];
			char afiltermac[32];
			char afiltercmd[32];
			char areplacenote[32];
			char areplaceins[32];
			char areplacemac[32];
			char areplacecmd[32];

			m_filternote.GetWindowText(afilternote,16);
			m_filterins.GetWindowText(afilterins,16);
			m_filtermac.GetWindowText(afiltermac,16);
			m_filtercmd.GetWindowText(afiltercmd,16);
			m_replacenote.GetWindowText(areplacenote,16);
			m_replaceins.GetWindowText(areplaceins,16);
			m_replacemac.GetWindowText(areplacemac,16);
			m_replacecmd.GetWindowText(areplacecmd,16);	

			int filterins = -1;
			int filtermac = -1;
			int replaceins = -1;
			int replacemac = -1;

			if (afilterins[0] !=	'\0')
				filterins = atoi(afilterins);

			if (afiltermac[0] != '\0')
				filtermac = atoi(afiltermac);			

			if (areplaceins[0] != '\0')
				replaceins = atoi(areplaceins);

			if (areplacemac[0] != '\0')
				replacemac = atoi(areplacemac);

			if ((filterins != -1) | (filtermac != -1))
			{
				// now perform the pattern data replacement

				int matchTarget = 0;

				if (filterins != -1)
					matchTarget++;

				if (filtermac != -1)
					matchTarget++;

				int currentPattern;
				int currentColumn;
				int currentLine;

				Song* pSong = _pChildView->_pSong;

				int lastPatternUsed = pSong->GetLastPatternUsed();
				int columnCount = pSong->SONGTRACKS;
				int lineCount;

				TRACE("lastPatternUsed used is %i\n", lastPatternUsed);
				TRACE("columnCount used is %i\n", columnCount);

				int currentins;
				int currentmac;

				unsigned char * toffset;

				int matchCount;
				PatternEntry* patternEntry;
				int elementIndex;

				for (currentPattern = 0; currentPattern <= lastPatternUsed; currentPattern++)
				{
					if (pSong->IsPatternUsed(currentPattern))
					{				
						//patternEntry = (PatternEntry*) pSong->_ppattern(currentPattern);
						lineCount = pSong->patternLines[currentPattern];

						for (currentLine = 0; currentLine < lineCount; currentLine++)
						{
							for (currentColumn = 0; currentColumn < columnCount; currentColumn++)
							{
								unsigned char *base = pSong->_ppattern(currentPattern);
								int const displace=((currentLine*columnCount)+currentColumn)*EVENT_SIZE;
								unsigned char *offset=base+displace;
								patternEntry = (PatternEntry*) offset;
								currentins = patternEntry[(currentLine*columnCount)+currentColumn]._inst;
								currentmac = patternEntry[(currentLine*columnCount)+currentColumn]._mach;																				
							
								TRACE("[%i,%i] ins=%i mac=%i\n", currentLine, currentColumn, currentins, currentmac);

								matchCount = 0;

								if (currentins == filterins)
								{
									matchCount++;
								}

								if (currentmac == filtermac)
								{
									matchCount++;
								}

								if (matchCount == matchTarget)
								{
									TRACE("MATCH currentLine is %i\n", currentLine);
									if (filterins != -1)
									{
										patternEntry[(currentLine*columnCount)+currentColumn]._inst = replaceins;
									}
									if (filtermac != -1)
									{
										patternEntry[(currentLine*columnCount)+currentColumn]._mach = replacemac;
									}
								}
							}
						}
					}
				}
			}
		}
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
