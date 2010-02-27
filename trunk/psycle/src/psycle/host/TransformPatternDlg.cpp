// TransformPatternDlg.cpp : implementation file
//

#include "TransformPatternDlg.hpp"

#include "PatternView.hpp"

#include <psycle/helpers/hexstring_to_integer.hpp>

namespace psycle { namespace host {

		// CTransformPatternDlg dialog

		IMPLEMENT_DYNAMIC(CTransformPatternDlg, CDialog)

		CTransformPatternDlg::CTransformPatternDlg(CChildView* pChildView, CWnd* pParent /*=NULL*/)
			: CDialog(CTransformPatternDlg::IDD, pParent)
		{
			_pChildView = pChildView;
			m_applyto = 0;
		}

		CTransformPatternDlg::CTransformPatternDlg(PatternView* pattern_view, CWnd* pParent /*=NULL*/)
			: CDialog(CTransformPatternDlg::IDD, pParent),
			  pattern_view_(pattern_view),
			  m_applyto(0)
		{			
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
			DDX_Radio(pDX, IDC_APPLYTOSONG, m_applyto);
			DDX_Control(pDX, IDC_APPLYTOSONG, m_applytosong);
			DDX_Control(pDX, IDC_APPLYTOPATTERN, m_applytopattern);
			DDX_Control(pDX, IDC_APPLYTOBLOCK, m_applytoblock);
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
			m_applytosong.EnableWindow(false);
			m_applytopattern.EnableWindow(false);
			m_applytoblock.EnableWindow(false);
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
				filterins = hexstring_to_integer(afilterins);

			if (afiltermac[0] != '\0')		
				filtermac = hexstring_to_integer(afiltermac);			

			if (areplaceins[0] != '\0')
				replaceins = hexstring_to_integer(areplaceins);

			if (areplacemac[0] != '\0')
				replacemac = hexstring_to_integer(areplacemac);

			TRACE("filterins is %i", filterins);

			if ((filterins != -1) | (filtermac != -1))
			{
				// now perform the pattern data replacement

				int matchTarget = 0;

				if (filterins != -1)
					matchTarget++;

				if (filtermac != -1)
					matchTarget++;

				Song & song = *pattern_view_->song();
#if !PSYCLE__CONFIGURATION__USE_PSYCORE
				int lastPatternUsed = pSong->GetHighestPatternIndexInSequence();
				int columnCount = MAX_TRACKS;
				int lineCount;
				int currentPattern;
				int currentColumn;
				int currentLine;
#endif
				int currentins;
				int currentmac;

				int matchCount;
#if PSYCLE__CONFIGURATION__USE_PSYCORE

				Sequence::patterns_type::iterator patite = song.sequence().patterns_begin();
				for ( ; patite != song.sequence().patterns_end(); ++patite) {
					Pattern::iterator eventite = (**patite).begin();
					PatternEvent pat_event = eventite->second;

#else
				for (currentPattern = 0; currentPattern <= lastPatternUsed; currentPattern++)
				{
					if (!song.isPatternEmpty(currentPattern))
					{				
						PatternEvent* patternEntry = (PatternEvent*) song._ppattern(currentPattern);
						lineCount = pSong->patternLines[currentPattern];
						
						for (currentLine = 0; currentLine < lineCount; currentLine++)
						{
							for (currentColumn = 0; currentColumn < columnCount; currentColumn++)
							{
								PatternEvent pat_event = patternEntry[(currentLine*columnCount)+currentColumn];
#endif
								currentins = pat_event.instrument();
								currentmac = pat_event.machine();																																							

								matchCount = 0;

								if(currentins == filterins) ++matchCount;
								if(currentmac == filtermac) ++matchCount;
								if(matchCount == matchTarget) {
									if(filterins != -1)
										pat_event.setInstrument(replaceins);
									if(filtermac != -1)
										pat_event.setMachine(replacemac);
								}
#if PSYCLE__CONFIGURATION__USE_PSYCORE
							}
#else
							}
						}
					}
				}
#endif
			}
		}
	}   // namespace
}   // namespace
