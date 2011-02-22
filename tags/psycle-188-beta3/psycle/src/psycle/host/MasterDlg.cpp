///\file
///\brief implementation file for psycle::host::CMasterDlg.

#include "MasterDlg.hpp"
#include "ChildView.hpp"

#include "Machine.hpp"

#include <psycle/helpers/dsp.hpp>

namespace psycle { namespace host {

		BEGIN_MESSAGE_MAP(CVolumeCtrl, CSliderCtrl)
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONUP()
		END_MESSAGE_MAP()

		void CVolumeCtrl::OnLButtonDown(UINT nFlags, CPoint point) {
			editing_ = true;
			CSliderCtrl::OnLButtonDown(nFlags, point);
		}

		void CVolumeCtrl::OnLButtonUp(UINT nFlags, CPoint point) {
			editing_ = false;
			CSliderCtrl::OnLButtonUp(nFlags, point);
		}

		/// master dialog
		CMasterDlg::CMasterDlg(CChildView* pParent, Master* new_master) 
			: CDialog(CMasterDlg::IDD, pParent), m_pParent(pParent), 
				_pMachine(new_master){
			memset(macname,0,sizeof(macname));
			for (int i = 0; i < 12; ++i) {
				CVolumeCtrl* slider = new CVolumeCtrl(i);
				sliders_.push_back(slider);
			}
			CDialog::Create(IDD, m_pParent);
			std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
			for ( ; it != sliders_.end(); ++it ) {
				CVolumeCtrl* slider = *it;
				slider->SetRange(0, 832);
				slider->SetPageSize(96);
			}
		}

		CMasterDlg::~CMasterDlg() {		
			m_back.DeleteObject();
			m_numbers.DeleteObject();
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
			DDX_Control(pDX, IDC_MIXERVIEW, m_mixerview);
			DDX_Control(pDX, IDC_AUTODEC, m_autodec);
		}

		BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
			//{{AFX_MSG_MAP(CMasterDlg)
			ON_BN_CLICKED(IDC_AUTODEC, OnAutodec)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERMASTER, OnCustomdrawSlidermaster)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM1, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM10, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM11, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM12, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM2, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM3, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM4, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM5, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM6, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM7, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM8, OnCustomdrawSliderm)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM9, OnCustomdrawSliderm)
			ON_WM_PAINT()
			//}}AFX_MSG_MAP
			ON_STN_CLICKED(IDC_MIXERVIEW, OnStnClickedMixerview)
		END_MESSAGE_MAP()

		BOOL CMasterDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_sliderknob.LoadBitmap(IDB_MASTERSLIDER);
			m_back.LoadBitmap(IDB_MASTERBACK);

			namesFont.CreatePointFont(80,"Tahoma");
			m_numbers.LoadBitmap(IDB_MASTERNUMBERS);
			
			m_slidermaster.SetRange(0, 832);
			m_slidermaster.SetPageSize(96);
			SetSliderValues();
			if (((Master*)_pMachine)->decreaseOnClip) m_autodec.SetCheck(1);
			else m_autodec.SetCheck(0);
			return TRUE;
		}

		void CMasterDlg::SetSliderValues()
		{
			float val;
			float db = helpers::dsp::dB(_pMachine->_outDry/256.0f);
			m_slidermaster.SetPos(832-(int)((db+40.0f)*16.0f));
			std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
			for ( int i = 0; it != sliders_.end(); ++it, ++i ) {
				CVolumeCtrl* slider = *it;
				if (_pMachine->_inputCon[i]) {		
					_pMachine->GetWireVolume(i,val);
					slider->SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
				} else {
					slider->SetPos(832);
				}
			}
		}

		void CMasterDlg::OnAutodec() 
		{
			if (m_autodec.GetCheck())
			{
				_pMachine->decreaseOnClip=true;
			}
			else _pMachine->decreaseOnClip=false;
		}

		void CMasterDlg::UpdateUI(void)
		{
			if (!--_pMachine->peaktime) 
			{
				char peak[10];
				if ( _pMachine->currentpeak > 0)
				{
					sprintf(peak,"%.2fdB",helpers::dsp::dB(_pMachine->currentpeak*0.00003051f));
				}
				else strcpy(peak,"-99dB");
				m_masterpeak.SetWindowText(peak);
				
				SetSliderValues();

				_pMachine->peaktime=25;
				_pMachine->currentpeak=0.0f;
			}
		}

		void CMasterDlg::OnCancel()
		{
			m_pParent->MasterMachineDialog = NULL;
			CDialog::OnCancel();
		}

		void CMasterDlg::OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_slidermaster.GetPos())/16.0f)-40.0f;
			if (m_slidermaster.editing())_pMachine->_outDry = int(helpers::dsp::dB2Amp(db)*256.0f);

			PaintNumbers(db,40,171);

			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::PaintNumbers(float val, int x, int y)
		{
			CDC *dc = m_mixerview.GetDC();
			CDC memDC;
//			CBitmap* oldbmp;
//			memDC.CreateCompatibleDC(dc);
//			oldbmp = memDC.SelectObject(&m_numbers);
			
			PaintNumbersDC(dc,&memDC,val,x,y);

//			memDC.SelectObject(oldbmp);
//			memDC.DeleteDC();
		}

		void CMasterDlg::PaintNumbersDC(CDC *dc, CDC *memDC, float val, int x, int y)
		{
			char valtxt[6];
			CFont* oldfont = dc->SelectObject(&namesFont);
			dc->SetTextColor(0x00FFFFFF); // White
			dc->SetBkColor(0x00000000); // Black

			if ( fabs(val) < 10.0f )
			{
				if ( val < 0 ) sprintf(valtxt,"%.01f",val);
				else sprintf(valtxt," %.01f",val);
			} else {
				if ( val < -39.5f) strcpy(valtxt,"-99 ");
				else if ( val < 0) sprintf(valtxt,"%.0f ",val);
				else sprintf(valtxt," %.0f ",val);
			}
			
			dc->TextOut(x,y-2,valtxt);
			dc->SelectObject(oldfont);
		}

		void CMasterDlg::PaintNames(char *name, int x, int y)
		{
			CDC *dc = m_mixerview.GetDC();
			CFont* oldfont = dc->SelectObject(&namesFont);
			dc->SetTextColor(0x00FFFFFF); // White
			dc->SetBkColor(0x00000000); // Black
			dc->TextOut(x,y,name);
			dc->SelectObject(oldfont);
		}

		void CMasterDlg::OnCustomdrawSliderm(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			CVolumeCtrl* slider =(CVolumeCtrl*) GetDlgItem(pNMHDR->idFrom);
			float db = ((832-slider->GetPos())/16.0f)-40.0f;
			if (slider->editing())_pMachine->SetWireVolume(slider->index(),helpers::dsp::dB2Amp(db));
			PaintNumbers(db,112 + slider->index() *24,171);
			
			if (slider->index() == 10) {
				// I know the following is Ugly, but it is the only solution I've found, because first,
				// OnPaint is called, after the bitmap is drawn, and finally the sliders are redrawn.
				CDC *dcm = m_mixerview.GetDC();
				CFont* oldfont = dcm->SelectObject(&namesFont);
				dcm->SetTextColor(0x00FFFFFF); // White
				dcm->SetBkColor(0x00000000); // Black
				int const xo(417);
				int const yo(16);
				int const dy(17);
				int y(yo);
				int i(0);
				while(i < MAX_CONNECTIONS) {
					dcm->ExtTextOut(xo, y, ETO_CLIPPED, CRect(xo,y,xo+77,y+13), CString(macname[i]), 0);
					i++;
					y += dy;
				}
				dcm->SelectObject(oldfont);
			}	
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnPaint() 
		{
			CPaintDC dc(this); // device context for painting
			
			if ( dc.m_ps.rcPaint.bottom >= 170 && dc.m_ps.rcPaint.top <= 185)
			{
				CDC *dcm = m_mixerview.GetDC();
				CDC memDC;
//				CBitmap* oldbmp;
//				memDC.CreateCompatibleDC(dcm);
//				oldbmp = memDC.SelectObject(&m_numbers);
				
				PaintNumbersDC(dcm,&memDC,((832-m_slidermaster.GetPos())/16.0f)-40.0f,40,171);
				std::vector<CVolumeCtrl*>::iterator it = sliders_.begin();
				for ( int i= 0; it != sliders_.end(); ++it, ++i) {
					CVolumeCtrl* slider = *it;
					PaintNumbersDC(dcm,&memDC,((832-slider->GetPos())/16.0f)-40.0f,112 +i*24,171);
				}
//				memDC.SelectObject(oldbmp);
//				memDC.DeleteDC();
			}
			if ( dc.m_ps.rcPaint.bottom >=16 && dc.m_ps.rcPaint.top<=200 && dc.m_ps.rcPaint.right >=410)
			{
				CDC *dcm = m_mixerview.GetDC();
				CFont* oldfont = dcm->SelectObject(&namesFont);
				dcm->SetTextColor(0x00FFFFFF); // White
				dcm->SetBkColor(0x00000000); // Black

				int const xo(417);
				int const yo(16);
				int const dy(17);
				int y(yo);
				int i(0);
				while(i < MAX_CONNECTIONS)
				{
					dcm->ExtTextOut(xo, y, ETO_CLIPPED, CRect(xo,y,xo+77,y+13), CString(macname[i]), 0);
					i++;
					y += dy;
				}
				dcm->SelectObject(oldfont);
			}
			// Do not call CDialog::OnPaint() for painting messages
		}

		LRESULT CMasterDlg::DrawSliderGraphics(NMHDR* pNMHDR)
		{
			NMCUSTOMDRAW nmcd = *reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

			if ( nmcd.dwDrawStage == CDDS_PREPAINT )
			{
				// return CDRF_NOTIFYITEMDRAW so that we will get subsequent 
				// CDDS_ITEMPREPAINT notifications

				CDC* pDC = CDC::FromHandle( nmcd.hdc );
				CDC memDC;
				CBitmap* oldbmp;
				memDC.CreateCompatibleDC(pDC);
				oldbmp=memDC.SelectObject(&m_back);
				pDC->BitBlt(nmcd.rc.left,nmcd.rc.top,nmcd.rc.right-nmcd.rc.left,nmcd.rc.bottom-nmcd.rc.top,&memDC,0,0,SRCCOPY);
				pDC->Detach();
				memDC.SelectObject(oldbmp);

				return CDRF_NOTIFYITEMDRAW ;
			}
			else if ( nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
			{
				if ( nmcd.dwItemSpec == TBCD_THUMB )
				{
					CDC* pDC = CDC::FromHandle( nmcd.hdc );
					CDC memDC;
					CBitmap* oldbmp;
					memDC.CreateCompatibleDC(pDC);
					oldbmp=memDC.SelectObject(&m_sliderknob);
					pDC->BitBlt(nmcd.rc.left,nmcd.rc.top,nmcd.rc.right-nmcd.rc.left,nmcd.rc.bottom-nmcd.rc.top,&memDC,0,0,SRCCOPY);
					pDC->Detach();
					memDC.SelectObject(oldbmp);
				}
				else if(nmcd.dwItemSpec == TBCD_CHANNEL)
				{
					CDC* pDC = CDC::FromHandle( nmcd.hdc );

					CDC memDC;
					CBitmap* oldbmp;
					memDC.CreateCompatibleDC(pDC);
					oldbmp=memDC.SelectObject(&m_back);
					CRect r;
					sliders_[0]->GetClientRect(r);
					nmcd.rc = r;
					pDC->BitBlt(nmcd.rc.left,nmcd.rc.top,nmcd.rc.right-nmcd.rc.left+1,nmcd.rc.bottom-nmcd.rc.top,&memDC,36,49,SRCCOPY);
					pDC->Detach();
					memDC.SelectObject(oldbmp);
				}
				return CDRF_SKIPDEFAULT;
			}
			else return 0;

		}


		BOOL CMasterDlg::PreTranslateMessage(MSG* pMsg) {
			if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP)) {
				m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
			}
			return CDialog::PreTranslateMessage(pMsg);
		}

		void CMasterDlg::OnStnClickedMixerview() {
		}

	}   // namespace
}   // namespace
