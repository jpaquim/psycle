/** @file 
 *  @brief SongBar dialog
 *  $Date: 2010-08-15 18:18:35 +0200 (dg., 15 ag 2010) $
 *  $Revision: 9831 $
 */

#include "SongBar.hpp"
#include "PsycleConfig.hpp"
#include "MainFrm.hpp"
#include "ChildView.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Player.hpp"

namespace psycle{ namespace host{

	extern CPsycleApp theApp;
IMPLEMENT_DYNAMIC(SongBar, CDialogBar)

	SongBar::SongBar()
	{
		vuprevR = 0;
		vuprevL = 0;
	}

	SongBar::~SongBar()
	{
	}
	void SongBar::InitializeValues(CMainFrame* frame, CChildView* view, Song* song)
	{
		m_pParentMain = frame;
		m_pWndView = view;
		m_pSong = song;
	}

	void SongBar::DoDataExchange(CDataExchange* pDX)
	{
		CDialogBar::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_TRACKCOMBO, m_trackcombo);
		DDX_Control(pDX, IDC_COMBOOCTAVE, m_octavecombo);
		DDX_Control(pDX, IDC_MASTERSLIDER, m_masterslider);
		DDX_Control(pDX, IDC_BPMLABEL, m_bpmlabel);
		DDX_Control(pDX, IDC_TPBLABEL, m_tpblabel);
	}

	//Message Maps are defined in SongBar, since this isn't a window, but a DialogBar.
	BEGIN_MESSAGE_MAP(SongBar, CDialogBar)
		ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
		ON_WM_HSCROLL()
	END_MESSAGE_MAP()

	// SongBar message handlers
	LRESULT SongBar::OnInitDialog ( WPARAM wParam, LPARAM lParam)
	{
		BOOL bRet = HandleInitDialog(wParam, lParam);

		if (!UpdateData(FALSE))
		{
		   TRACE0("Warning: UpdateData failed during dialog init.\n");
		}

		((CButton*)GetDlgItem(IDC_BPM_DECTEN))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_LESSLESS),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_BPM_DECONE))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_LESS),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_BPM_ADDONE))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_MORE),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_BPM_ADDTEN))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_MOREMORE),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_DEC_TPB))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_LESS),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_INC_TPB))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_MORE),IMAGE_ICON,16,16,0));

		for(int i=4;i<=MAX_TRACKS;i++)
		{
			char s[4];
			_snprintf(s,4,"%i",i);
			m_trackcombo.AddString(s);
		}
		m_trackcombo.SetCurSel(m_pSong->SONGTRACKS-4);

		m_masterslider.SetRange(0,1024);
		m_masterslider.SetTipSide(TBTS_TOP);
		float val = 1.0f;
		if ( m_pSong->_pMachine[MASTER_INDEX] != NULL) {
			 val =((Master*)m_pSong->_pMachine[MASTER_INDEX])->_outDry/256.f;
		}
		int nPos = helpers::dsp::AmountToSlider(val);
		m_masterslider.SetPos(nPos);
		m_masterslider.SetTicFreq(64);
		m_masterslider.SetPageSize(64);

		return bRet;
	}


	void SongBar::OnSelchangeTrackcombo() 
	{
		m_pSong->SONGTRACKS=m_trackcombo.GetCurSel()+4;
		if (m_pWndView->editcur.track >= m_pSong->SONGTRACKS )
			m_pWndView->editcur.track= m_pSong->SONGTRACKS-1;

		m_pWndView->RecalculateColourGrid();
		m_pWndView->Repaint();
		m_pWndView->SetFocus();
	}

	void SongBar::OnCloseupTrackcombo() 
	{
		m_pWndView->SetFocus();
	}

	void SongBar::OnBpmAddOne()
	{
		SetAppSongBpm(1);
		((CButton*)GetDlgItem(IDC_BPM_ADDONE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();	
	}

	void SongBar::OnBpmAddTen() 
	{
		SetAppSongBpm(10);
		((CButton*)GetDlgItem(IDC_BPM_ADDTEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();	
	}

	void SongBar::OnBpmDecOne() 
	{
		SetAppSongBpm(-1);
		((CButton*)GetDlgItem(IDC_BPM_DECONE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();	
	}

	void SongBar::OnBpmDecTen() 
	{
		SetAppSongBpm(-10);
		((CButton*)GetDlgItem(IDC_BPM_DECTEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();	
	}

	void SongBar::OnDecTPB()
	{
		SetAppSongTpb(-1);
		((CButton*)GetDlgItem(IDC_DEC_TPB))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
		m_pWndView->Repaint();
	}

	void SongBar::OnIncTPB()
	{
		SetAppSongTpb(+1);
		((CButton*)GetDlgItem(IDC_INC_TPB))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
		m_pWndView->Repaint();
	}

	void SongBar::SetAppSongBpm(int x) 
	{
		char buffer[16];
		if ( x != 0 )
		{
			if (Global::pPlayer->_playing ) 
			{
				Global::song().BeatsPerMin(Global::pPlayer->bpm+x);
			}
			else Global::song().BeatsPerMin(Global::song().BeatsPerMin()+x);
			Global::pPlayer->SetBPM(Global::song().BeatsPerMin(),Global::song().LinesPerBeat());
			sprintf(buffer,"%d",Global::song().BeatsPerMin());
		}
		else sprintf(buffer,"%d",Global::pPlayer->bpm);
		
		m_bpmlabel.SetWindowText(buffer);
	}

	void SongBar::SetAppSongTpb(int x) 
	{
		char buffer[16];
		if ( x != 0)
		{
			if (Global::pPlayer->_playing ) 
			{
				Global::song().LinesPerBeat(Global::pPlayer->tpb+x);
			}
			else Global::song().LinesPerBeat(Global::song().LinesPerBeat()+x);
			Global::pPlayer->SetBPM(Global::song().BeatsPerMin(), Global::song().LinesPerBeat());
			sprintf(buffer,"%d",Global::song().LinesPerBeat());
		}
		else sprintf(buffer, "%d", Global::pPlayer->tpb);
		
		m_tpblabel.SetWindowText(buffer);
	}

	void SongBar::OnCloseupCombooctave() 
	{
		m_pWndView->SetFocus();
	}

	void SongBar::OnSelchangeCombooctave() 
	{
		m_pSong->currentOctave=m_octavecombo.GetCurSel();
		
		m_pWndView->Repaint();
		m_pWndView->SetFocus();
	}

	//////////////////////////////////////////////////////////////////////
	// Function that shift the current editing octave

	void SongBar::ShiftOctave(int x)
	{
		m_pSong->currentOctave += x;
		if ( m_pSong->currentOctave < 0 )	 { m_pSong->currentOctave = 0; }
		else if ( m_pSong->currentOctave > 8 ){ m_pSong->currentOctave = 8; }

		m_octavecombo.SetCurSel(m_pSong->currentOctave);
	}
	void SongBar::UpdateMasterValue(int newvalue)
	{
		int value = 1024-helpers::dsp::AmountToSlider(newvalue/256.f);
		if ( m_pSong->_pMachine[MASTER_INDEX] != NULL)
		{
			if (m_masterslider.GetPos() != value) {
				m_masterslider.SetPos(value);
			}
		}
	}
	void SongBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
		CSliderCtrl* the_slider = reinterpret_cast<CSliderCtrl*>(pScrollBar);

		switch(nSBCode){
		case TB_BOTTOM: //fallthrough
		case TB_LINEDOWN: //fallthrough
		case TB_PAGEDOWN: //fallthrough
		case TB_TOP: //fallthrough
		case TB_LINEUP: //fallthrough
		case TB_PAGEUP: //fallthrough
			if (  the_slider == &m_masterslider) {
				((Master*)m_pSong->_pMachine[MASTER_INDEX])->_outDry = 256 * helpers::dsp::SliderToAmount(1024-m_masterslider.GetPos());
			}
			break;
		case TB_THUMBPOSITION: //fallthrough
		case TB_THUMBTRACK:
			if ( m_pSong->_pMachine[MASTER_INDEX] != NULL && the_slider == &m_masterslider) {
				((Master*)m_pSong->_pMachine[MASTER_INDEX])->_outDry = 256.f* helpers::dsp::SliderToAmount(1024-nPos);
			}
			break;
		case TB_ENDTRACK:
			m_pWndView->SetFocus();
			break;
		}
		CDialogBar::OnHScroll(nSBCode, nPos, pScrollBar);
	}

	void SongBar::OnClipbut() 
	{
		((Master*)(Global::song()._pMachine[MASTER_INDEX]))->_clip = false;
		((CButton*)GetDlgItem(IDC_CLIPBUT))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	//l and r are the left and right vu meter values
	void SongBar::UpdateVumeters(float l, float r,COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip)
	{
		if (Global::psycleconf().macView().draw_vus)
		{
			if(l<1)l=1;
			if(r<1)r=1;
			
			CStatic *stclip=(CStatic *)GetDlgItem(IDC_FRAMECLIP);
			CClientDC clcanvas(stclip);
			
			if (clip) clcanvas.FillSolidRect(0,0,9,20,vu3);
			else  clcanvas.FillSolidRect(0,0,9,20,vu2);
			
			CStatic *lv=(CStatic *)GetDlgItem(IDC_LVUM);
			CClientDC canvasl(lv);

			int log_l=100*log10f(l);
			int log_r=100*log10f(r);
			log_l=log_l-225;
			if ( log_l < 0 )log_l=0;
			if ( log_l > 225 )log_l=225;
			log_r=log_r-225;
			if ( log_r < 0 )log_r=0;
			if ( log_r > 225 )log_r=225;
			
			if (log_l || vuprevL)
			{
				canvasl.FillSolidRect(0,0,log_l,4,vu1);
				if (vuprevL > log_l )
				{
					canvasl.FillSolidRect(log_l,0,vuprevL-log_l,4,vu3);
					canvasl.FillSolidRect(vuprevL,0,225-vuprevL,4,vu2);
					vuprevL-=2;
				}
				else 
				{
					canvasl.FillSolidRect(log_l,0,225-log_l,4,vu2);
					vuprevL = log_l;
				}
			}
			else
				canvasl.FillSolidRect(0,0,225,4,vu2);

			if (log_r || vuprevR)
			{
				canvasl.FillSolidRect(0,5,log_r,4,vu1);
				
				if (vuprevR > log_r )
				{
					canvasl.FillSolidRect(log_r,5,vuprevR-log_r,4,vu3);
					canvasl.FillSolidRect(vuprevR,5,225-vuprevR,4,vu2);
					vuprevR-=2;
				}
				else 
				{
					canvasl.FillSolidRect(log_r,5,225-log_r,4,vu2);
					vuprevR = log_r;
				}
			}
			else
				canvasl.FillSolidRect(0,5,225,4,vu2);
		}
	}


BOOL SongBar::OnToolTipNotify( UINT unId, NMHDR *pstNMHDR, LRESULT *pstResult )
{
	TOOLTIPTEXT* pstTTT = (TOOLTIPTEXT * )pstNMHDR;
	//UINT nID = pstNMHDR->idFrom;
	if ((pstTTT->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		//nID = ::GetDlgCtrlID((HWND)nID);

		sprintf(pstTTT->szText, "%.02f dB", helpers::dsp::dB(helpers::dsp::SliderToAmount(1024-m_masterslider.GetPos())));
		pstTTT->hinst = AfxGetResourceHandle();
		return(TRUE);
	}
	return (FALSE);
} 

}}