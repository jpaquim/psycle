///\file
///\brief implementation file for psycle::host::CMasterDlg.
#include <psycle/host/detail/project.private.hpp>
#include "MasterDlg.hpp"
#include "InputHandler.hpp"
#include "Machine.hpp"
#include "DPI.hpp"
#include <psycle/helpers/dsp.hpp>

namespace psycle { namespace host {

	int CMasterDlg::numbersMasterX = 22;
	int CMasterDlg::numbersX = 118;
	int CMasterDlg::numbersY = 186;
	int CMasterDlg::numbersAddX = 24;
	int CMasterDlg::textX = 427;
	int CMasterDlg::textY = 32;
	int CMasterDlg::textYAdd = 15;
	int CMasterDlg::textW = 75;
	int CMasterDlg::textH = 12;
//	int CMasterVu::vuImgH = 162;
//	int CMasterVu::vuImgW = 31;
//	int CMasterVu::vuImgX = 62;
//	int CMasterVu::vuImgY = 37;
	int CMasterVu::vuImgH = 159;
	int CMasterVu::vuImgW = 18;
	  
  BEGIN_MESSAGE_MAP(CMasterVu, CProgressCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
  END_MESSAGE_MAP()

	CMasterVu::CMasterVu():CProgressCtrl()
	{
	}
	CMasterVu::~CMasterVu()
	{
		m_vuOn.DeleteObject();
		m_vuOff.DeleteObject();
	}
	void CMasterVu::LoadBitmap(UINT IDControlOn, UINT IDControlOff)
	{
		m_vuOn.LoadBitmap(IDControlOn);
		m_vuOff.LoadBitmap(IDControlOff);
	}
	BOOL CMasterVu::OnEraseBkgnd(CDC* pDC) 
	{
		return TRUE;
	}
	void CMasterVu::OnPaint() 
	{
		CPaintDC dc(this);
		dc.SetStretchBltMode(HALFTONE);
		CRect crect, wrect;
		GetClientRect(&crect);
		CBitmap* oldbmp;
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		oldbmp=memDC.SelectObject(&m_vuOn);
		int vol = vuImgH - GetPos()*vuImgH/100;
		int volRel = crect.Height() - GetPos()*crect.Height()/100;
		dc.StretchBlt(0,volRel,crect.Width(),crect.Height(),&memDC,0,vol,vuImgW,vuImgH,SRCCOPY);
		//dc.BitBlt(0, vol, crect.Width(), crect.Height(), &memDC, 0, vol, SRCCOPY);
		memDC.SelectObject(&m_vuOff);
		dc.StretchBlt(0,0,crect.Width(),volRel,&memDC,0,0,vuImgW,vol,SRCCOPY);
		//dc.BitBlt(0, 0, crect.Width(), vol, &memDC, vuImgX, vuImgY, SRCCOPY);
		memDC.SelectObject(oldbmp);
	}


		/// master dialog
		CMasterDlg::CMasterDlg(CWnd* wndView, Master& new_master, CMasterDlg** windowVar) 
			: CDialog(CMasterDlg::IDD, AfxGetMainWnd()), windowVar_(windowVar)
			, machine(new_master), m_slidermaster(-1)
			, mainView(wndView)
		{
			memset(macname,0,sizeof(macname));
			for (int i = 0; i < 12; ++i) {
				CVolumeCtrl* slider = new CVolumeCtrl(i);
				sliders_.push_back(slider);
			}
			// Using CDPI example class
			CDPI& g_metrics = Global::dpiSetting();

			numbersMasterX = g_metrics.ScaleX(numbersMasterX);
			numbersX = g_metrics.ScaleX(numbersX);
			numbersY = g_metrics.ScaleY(numbersY);
			numbersAddX = g_metrics.ScaleX(numbersAddX);
			textX = g_metrics.ScaleX(textX);
			textY = g_metrics.ScaleY(textY);
			textYAdd = g_metrics.ScaleY(textYAdd);
			textW = g_metrics.ScaleX(textW);
			textH = g_metrics.ScaleY(textH);
			CDialog::Create(CMasterDlg::IDD, AfxGetMainWnd());
		}

		CMasterDlg::~CMasterDlg() {
			namesFont.DeleteObject();
			m_back.DeleteObject();
			m_sliderknob.DeleteObject();
			std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
			for ( ; it != sliders_.end(); ++it ) {
				delete *it;
			}
		}
		void CMasterDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_MASTERPEAK, m_masterpeak);
			DDX_Control(pDX, IDC_SLIDERMASTER, m_slidermaster);
			DDX_Control(pDX, IDC_SLIDERM9, *sliders_[8]);
			DDX_Control(pDX, IDC_SLIDERM8, *sliders_[7]);
			DDX_Control(pDX, IDC_SLIDERM7, *sliders_[6]);
			DDX_Control(pDX, IDC_SLIDERM6, *sliders_[5]);
			DDX_Control(pDX, IDC_SLIDERM5, *sliders_[4]);
			DDX_Control(pDX, IDC_SLIDERM4, *sliders_[3]);
			DDX_Control(pDX, IDC_SLIDERM3, *sliders_[2]);
			DDX_Control(pDX, IDC_SLIDERM2, *sliders_[1]);
			DDX_Control(pDX, IDC_SLIDERM12, *sliders_[11]);
			DDX_Control(pDX, IDC_SLIDERM11, *sliders_[10]);
			DDX_Control(pDX, IDC_SLIDERM10, *sliders_[9]);
			DDX_Control(pDX, IDC_SLIDERM1, *sliders_[0]);
			DDX_Control(pDX, IDC_AUTODEC, m_autodec);
			DDX_Control(pDX, IDC_MASTERDLG_VULEFT, m_vuLeft);
			DDX_Control(pDX, IDC_MASTERDLG_VURIGHT, m_vuRight);
		}

		BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
			ON_WM_VSCROLL()
			ON_WM_PAINT()
	        ON_WM_ERASEBKGND()
			ON_WM_CLOSE()
			ON_BN_CLICKED(IDC_AUTODEC, OnAutodec)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERMASTER, OnCustomdrawSlidermaster)
			ON_NOTIFY_RANGE(NM_CUSTOMDRAW, IDC_SLIDERM1, IDC_SLIDERM12, OnCustomdrawSliderm)
		END_MESSAGE_MAP()

		BOOL CMasterDlg::PreTranslateMessage(MSG* pMsg) {
			if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP)) {
				CmdDef def = Global::pInputHandler->KeyToCmd(pMsg->wParam,0);
				if(def.GetType() == CT_Note) {
					mainView->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
					return true;
				}
			}
			return CDialog::PreTranslateMessage(pMsg);
		}

		BOOL CMasterDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_sliderknob.LoadBitmap(IDB_MASTERKNOB);
			m_back.LoadBitmap(IDB_MASTER_BGND);
			m_vuLeft.LoadBitmap(IDB_MASTER_VUON_LEFT,IDB_MASTER_VUOFF_LEFT);
			m_vuRight.LoadBitmap(IDB_MASTER_VUON_RIGHT, IDB_MASTER_VUOFF_RIGHT);
			// Get dimension
			BITMAP bm;
			m_back.GetBitmap(&bm);
			m_nBmpWidth = bm.bmWidth;
			m_nBmpHeight = bm.bmHeight;

			namesFont.CreatePointFont(80,"Tahoma");
			
			m_slidermaster.SetRange(0, 832);
			m_slidermaster.SetPageSize(96);
			std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
			for ( ; it != sliders_.end(); ++it ) {
				CVolumeCtrl* slider = *it;
				slider->SetRange(0, 832);
				slider->SetPageSize(96);
			}
			SetSliderValues();
			m_vuLeft.SetRange(0,100);
			m_vuRight.SetRange(0,100);
			if (machine.decreaseOnClip) m_autodec.SetCheck(1);
			else m_autodec.SetCheck(0);
			return TRUE;
		}
		void CMasterDlg::OnCancel()
		{
			DestroyWindow();
		}
		void CMasterDlg::OnClose()
		{
			CDialog::OnClose();
			DestroyWindow();
		}
		void CMasterDlg::PostNcDestroy()
		{
			CDialog::PostNcDestroy();
			if(windowVar_!=NULL) *windowVar_ = NULL;
			delete this;
		}

		void CMasterDlg::OnAutodec() 
		{
			if (m_autodec.GetCheck())
			{
				machine.decreaseOnClip=true;
			}
			else machine.decreaseOnClip=false;
		}



		void CMasterDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
			CSliderCtrl* the_slider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
			switch(nSBCode){
			case TB_BOTTOM: //fallthrough
			case TB_LINEDOWN: //fallthrough
			case TB_PAGEDOWN: //fallthrough
			case TB_TOP: //fallthrough
			case TB_LINEUP: //fallthrough
			case TB_PAGEUP: //fallthrough
				if(the_slider == &m_slidermaster) {
					OnChangeSliderMaster(m_slidermaster.GetPos());
				}
				else {
					std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
					for ( ; it != sliders_.end(); ++it ) {
						if( the_slider == *it) {
							OnChangeSliderMacs(*it);
						}
					}
				}
				break;
			case TB_THUMBPOSITION: //fallthrough
			case TB_THUMBTRACK:
				if(the_slider == &m_slidermaster) {
					OnChangeSliderMaster(m_slidermaster.GetPos());
				}
				else {
					std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
					for ( ; it != sliders_.end(); ++it ) {
						if( the_slider == *it) {
							OnChangeSliderMacs(*it);
						}
					}
				}
				break;
			}
			CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
		}

		void CMasterDlg::SetSliderValues()
		{
			float val;
			float db;
			if(machine._outDry>0) {
				db = helpers::dsp::dB(machine._outDry/256.0f);
			}
			else {
				db = -99.f;
			}
			m_slidermaster.SetPos(832-(int)((db+40.0f)*16.0f));
			std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
			for ( int i = 0; it != sliders_.end(); ++it, ++i ) {
				CVolumeCtrl* slider = *it;
				if (machine._inputCon[i]) {		
					machine.GetWireVolume(i,val);
					slider->SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
				} else {
					slider->SetPos(832);
				}
			}
		}

		void CMasterDlg::UpdateUI(void)
		{
			if (!--machine.peaktime) 
			{
				std::string peak;
				
				char tmp[10];
				if ( machine.currentpeak > 0.001f) //26bits of precision in the display
				{
					sprintf(tmp,"%.2fdB",helpers::dsp::dB(machine.currentpeak*(1.f/machine.GetAudioRange())));
					peak = tmp;
				}
				else peak = "-inf dB";
				if ( machine.currentrms > 0.001f) //26bits of precision in the display
				{
					sprintf(tmp,"%.2fdB",helpers::dsp::dB(machine.currentrms*(1.f/machine.GetAudioRange())));
					peak = peak + "/" + tmp;
				}
				else peak = peak + "/-inf dB";

				m_masterpeak.SetWindowText(peak.c_str());
				
				SetSliderValues();

				machine.peaktime=25;
				machine.currentpeak=0.0f;
				machine.currentrms=0.0f;
				CRect r;
				r.top=textY;
				r.left=textX;
				r.right=textX+textW;
				r.bottom=textY+MAX_CONNECTIONS*textYAdd;
				InvalidateRect(r,FALSE);
			}
			m_vuLeft.SetPos(machine.volumeDisplayLeft);
			m_vuRight.SetPos(machine.volumeDisplayRight);
		}

		void CMasterDlg::OnChangeSliderMaster(int pos)
		{
			float db = ((832-pos)/16.0f)-40.0f;
			machine._outDry = int(helpers::dsp::dB2Amp(db)*256.0f);
		}

		void CMasterDlg::OnChangeSliderMacs(CVolumeCtrl* slider)
		{
			float db = ((832-slider->GetPos())/16.0f)-40.0f;
			machine.SetWireVolume(slider->index(),helpers::dsp::dB2Amp(db));
		}

		void CMasterDlg::OnPaint() 
		{
			CPaintDC dc(this); // device context for painting
	
			RECT& rect = dc.m_ps.rcPaint;
			if ( rect.bottom >= numbersY && rect.top <= numbersY+textH)
			{
				PaintNumbersDC(&dc,((832-m_slidermaster.GetPos())/16.0f)-40.0f,numbersMasterX,numbersY);
				std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
				for ( int i= 0; it != sliders_.end(); ++it, ++i) {
					CVolumeCtrl* slider = *it;
					PaintNumbersDC(&dc,((832-slider->GetPos())/16.0f)-40.0f,numbersX +i*numbersAddX,numbersY);
				}
			}
			if (rect.right >=textX)
			{
				CFont* oldfont = dc.SelectObject(&namesFont);
				dc.SetTextColor(0x00FFFFFF); // White
				dc.SetBkColor(0x00000000); // Black

				for(int i=0, y=textY ; i < MAX_CONNECTIONS; i++, y += textYAdd)
				{
					dc.ExtTextOut(textX, y-1, ETO_OPAQUE|ETO_CLIPPED, CRect(textX,y,textX+textW,y+textH), CString(macname[i]), 0);
				}
				dc.SelectObject(oldfont);
			}
			// Do not call CDialog::OnPaint() for painting messages
		}
		void CMasterDlg::PaintNumbersDC(CDC *dc, float val, int x, int y)
		{
			char valtxt[6];
			if ( fabs(val) < 10.0f )
			{
				if ( val < 0 ) sprintf(valtxt,"%.01f",val);
				else sprintf(valtxt," %.01f",val);
			} else {
				if ( val < -39.5f) strcpy(valtxt,"-99 ");
				else if ( val < 0) sprintf(valtxt,"%.0f ",val);
				else sprintf(valtxt," %.0f ",val);
			}
			dc->SetTextColor(0x00FFFFFF); // White
			dc->SetBkColor(0x00000000); // Black
			CFont* oldfont = dc->SelectObject(&namesFont);
			dc->ExtTextOut(x, y-2, ETO_CLIPPED, CRect(x,y,x+numbersAddX-2,y+textH), CString(valtxt), 0);
			dc->SelectObject(oldfont);
		}

		void CMasterDlg::OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NMCUSTOMDRAW nmcd = *reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
			if (nmcd.dwDrawStage == CDDS_POSTPAINT)
			{
				float db = ((832-m_slidermaster.GetPos())/16.0f)-40.0f;
				CClientDC dc(this);
				PaintNumbersDC(&dc,db,numbersMasterX,numbersY);
				*pResult = CDRF_DODEFAULT;
			}
			else {
				*pResult = DrawSliderGraphics(pNMHDR);
			}
		}

		void CMasterDlg::OnCustomdrawSliderm(UINT idx, NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NMCUSTOMDRAW nmcd = *reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
			if (nmcd.dwDrawStage == CDDS_POSTPAINT)
			{
				CVolumeCtrl* slider =(CVolumeCtrl*) GetDlgItem(pNMHDR->idFrom);
				float db = ((832-slider->GetPos())/16.0f)-40.0f;
				CClientDC dc(this);
				PaintNumbersDC(&dc,db,numbersX + slider->index()*numbersAddX,numbersY);
				*pResult = CDRF_DODEFAULT;
			}
			else {
				*pResult = DrawSliderGraphics(pNMHDR);
			}
		}

		LRESULT CMasterDlg::DrawSliderGraphics(NMHDR* pNMHDR)
		{
			NMCUSTOMDRAW nmcd = *reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
			switch(nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
			{
				return CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
			}
			case CDDS_ITEMPREPAINT:
			{
				if ( nmcd.dwItemSpec == TBCD_THUMB )
				{
					CDC* pDC = CDC::FromHandle( nmcd.hdc );
					CDC memDC;
					CBitmap* oldbmp;
					memDC.CreateCompatibleDC(pDC);
					oldbmp=memDC.SelectObject(&m_sliderknob);
					pDC->StretchBlt(nmcd.rc.left,nmcd.rc.top,nmcd.rc.right-nmcd.rc.left,nmcd.rc.bottom-nmcd.rc.top,&memDC,0,0,22,10,SRCCOPY);
					//pDC->BitBlt(nmcd.rc.left,nmcd.rc.top,nmcd.rc.right-nmcd.rc.left,nmcd.rc.bottom-nmcd.rc.top,&memDC,0,0,SRCCOPY);
					memDC.SelectObject(oldbmp);
				}
				else if(nmcd.dwItemSpec == TBCD_CHANNEL)
				{
					//Drawing the whole background, not only the channel.
					CDC* pDC = CDC::FromHandle( nmcd.hdc );
					CDC memDC;
					CBitmap* oldbmp;
					CDPI& g_metrics = Global::dpiSetting();
					pDC->SetStretchBltMode(HALFTONE);
					memDC.CreateCompatibleDC(pDC);
					oldbmp=memDC.SelectObject(&m_back);
					CVolumeCtrl* slider =(CVolumeCtrl*) GetDlgItem(pNMHDR->idFrom);
					CRect crect, wrect;
					slider->GetClientRect(&crect);
					slider->GetWindowRect(&wrect);
					ScreenToClient(wrect);
					pDC->StretchBlt(0, 0, crect.Width(), crect.Height(), &memDC, g_metrics.UnscaleX(wrect.left), g_metrics.UnscaleY(wrect.top), g_metrics.UnscaleX(crect.Width()), g_metrics.UnscaleY(crect.Height()), SRCCOPY);
					//pDC->BitBlt(0, 0, crect.Width(), crect.Height(), &memDC, wrect.left, wrect.top, SRCCOPY);
					memDC.SelectObject(oldbmp);
				}
				else {
					//Do not draw ticks.
				}
				return CDRF_SKIPDEFAULT;
			}
			default:
				return CDRF_DODEFAULT;
			}
		}
		
		BOOL CMasterDlg::OnEraseBkgnd(CDC* pDC) 
		{
			BOOL res = CDialog::OnEraseBkgnd(pDC);
			if (m_back.m_hObject != NULL)
			{
				// Using CDPI example class
				CDPI& g_metrics = Global::dpiSetting();
				pDC->SetStretchBltMode(HALFTONE);
				CDC memDC;
				memDC.CreateCompatibleDC(pDC);
				CBitmap* pOldBitmap = memDC.SelectObject(&m_back);
				pDC->StretchBlt(0,0,g_metrics.ScaleX(m_nBmpWidth),g_metrics.ScaleY(m_nBmpHeight),&memDC,0,0,m_nBmpWidth,m_nBmpHeight,SRCCOPY);
				//pDC->BitBlt(0, 0, m_nBmpWidth, m_nBmpHeight, &memDC, 0, 0, SRCCOPY);
				memDC.SelectObject(pOldBitmap);
				res = TRUE;
			}
			return res;
		}
	}   // namespace
}   // namespace
