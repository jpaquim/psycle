// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief implementation file for psycle::host::CMasterDlg.
#include <psycle/project.private.hpp>
#include "MasterDlg.hpp"
#include "Psycle.hpp"
#include "ChildView.hpp"
#include "Dsp.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		BEGIN_MESSAGE_MAP(CVolumeCtrl, CSliderCtrl)
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONUP()
		END_MESSAGE_MAP()

		void CVolumeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
		{
			editing=true;
			CSliderCtrl::OnLButtonDown(nFlags, point);
		}
		void CVolumeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
		{
			editing=false;
			CSliderCtrl::OnLButtonUp(nFlags, point);
		}

		CMasterDlg::CMasterDlg(CChildView* pParent) : CDialog(CMasterDlg::IDD, pParent)
		{
			m_pParent = pParent;
			//{{AFX_DATA_INIT(CMasterDlg)
			//}}AFX_DATA_INIT
			memset(macname,0,sizeof(macname));
		}

		void CMasterDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CMasterDlg)
			DDX_Control(pDX, IDC_MASTERPEAK, m_masterpeak);
			DDX_Control(pDX, IDC_SLIDERMASTER, m_slidermaster);
			DDX_Control(pDX, IDC_SLIDERM9, m_sliderm9);
			DDX_Control(pDX, IDC_SLIDERM8, m_sliderm8);
			DDX_Control(pDX, IDC_SLIDERM7, m_sliderm7);
			DDX_Control(pDX, IDC_SLIDERM6, m_sliderm6);
			DDX_Control(pDX, IDC_SLIDERM5, m_sliderm5);
			DDX_Control(pDX, IDC_SLIDERM4, m_sliderm4);
			DDX_Control(pDX, IDC_SLIDERM3, m_sliderm3);
			DDX_Control(pDX, IDC_SLIDERM2, m_sliderm2);
			DDX_Control(pDX, IDC_SLIDERM12, m_sliderm12);
			DDX_Control(pDX, IDC_SLIDERM11, m_sliderm11);
			DDX_Control(pDX, IDC_SLIDERM10, m_sliderm10);
			DDX_Control(pDX, IDC_SLIDERM1, m_sliderm1);
			DDX_Control(pDX, IDC_MIXERVIEW, m_mixerview);
			DDX_Control(pDX, IDC_AUTODEC, m_autodec);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
			//{{AFX_MSG_MAP(CMasterDlg)
			ON_BN_CLICKED(IDC_AUTODEC, OnAutodec)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERMASTER, OnCustomdrawSlidermaster)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM1, OnCustomdrawSliderm1)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM10, OnCustomdrawSliderm10)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM11, OnCustomdrawSliderm11)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM12, OnCustomdrawSliderm12)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM2, OnCustomdrawSliderm2)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM3, OnCustomdrawSliderm3)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM4, OnCustomdrawSliderm4)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM5, OnCustomdrawSliderm5)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM6, OnCustomdrawSliderm6)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM7, OnCustomdrawSliderm7)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM8, OnCustomdrawSliderm8)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM9, OnCustomdrawSliderm9)
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
			m_sliderm1.SetRange(0, 832);
			m_sliderm2.SetRange(0, 832);
			m_sliderm3.SetRange(0, 832);
			m_sliderm4.SetRange(0, 832);
			m_sliderm5.SetRange(0, 832);
			m_sliderm6.SetRange(0, 832);
			m_sliderm7.SetRange(0, 832);
			m_sliderm8.SetRange(0, 832);
			m_sliderm9.SetRange(0, 832);
			m_sliderm10.SetRange(0, 832);
			m_sliderm11.SetRange(0, 832);
			m_sliderm12.SetRange(0, 832);
			
			m_slidermaster.SetPageSize(96);
			m_sliderm1.SetPageSize(96);
			m_sliderm2.SetPageSize(96);
			m_sliderm3.SetPageSize(96);
			m_sliderm4.SetPageSize(96);
			m_sliderm5.SetPageSize(96);
			m_sliderm6.SetPageSize(96);
			m_sliderm7.SetPageSize(96);
			m_sliderm8.SetPageSize(96);
			m_sliderm9.SetPageSize(96);
			m_sliderm10.SetPageSize(96);
			m_sliderm11.SetPageSize(96);
			m_sliderm12.SetPageSize(96);

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


			if (_pMachine->_inputCon[0])
			{
				_pMachine->GetWireVolume(0,val);
				m_sliderm1.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm1.SetPos(832);
			}

			if (_pMachine->_inputCon[1])
			{
				_pMachine->GetWireVolume(1,val);
				m_sliderm2.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm2.SetPos(832);
			}

			if (_pMachine->_inputCon[2])
			{
				_pMachine->GetWireVolume(2,val);
				m_sliderm3.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm3.SetPos(832);
			}

			if (_pMachine->_inputCon[3])
			{
				_pMachine->GetWireVolume(3,val);
				m_sliderm4.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm4.SetPos(832);
			}

			if (_pMachine->_inputCon[4])
			{
				_pMachine->GetWireVolume(4,val);
				m_sliderm5.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm5.SetPos(832);
			}

			if (_pMachine->_inputCon[5])
			{
				_pMachine->GetWireVolume(5,val);
				m_sliderm6.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm6.SetPos(832);
			}

			if (_pMachine->_inputCon[6])
			{
				_pMachine->GetWireVolume(6,val);
				m_sliderm7.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm7.SetPos(832);
			}

			if (_pMachine->_inputCon[7])
			{
				_pMachine->GetWireVolume(7,val);
				m_sliderm8.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm8.SetPos(832);
			}

			if (_pMachine->_inputCon[8])
			{
				_pMachine->GetWireVolume(8,val);
				m_sliderm9.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm9.SetPos(832);
			}

			if (_pMachine->_inputCon[9])
			{
				_pMachine->GetWireVolume(9,val);
				m_sliderm10.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm10.SetPos(832);
			}

			if (_pMachine->_inputCon[10])
			{
				_pMachine->GetWireVolume(10,val);
				m_sliderm11.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm11.SetPos(832);
			}

			if (_pMachine->_inputCon[11])
			{
				_pMachine->GetWireVolume(11,val);
				m_sliderm12.SetPos(832-(int)((helpers::dsp::dB(val)+40.0f)*16.0f));
			} else {
				m_sliderm12.SetPos(832);
			}
		}

		void CMasterDlg::OnAutodec() 
		{
			if (m_autodec.GetState() &0x0003)
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


		BOOL CMasterDlg::Create()
		{
			return CDialog::Create(IDD, m_pParent);
		}

		void CMasterDlg::OnCancel()
		{
			m_pParent->MasterMachineDialog = NULL;
			m_back.DeleteObject();
			m_numbers.DeleteObject();
			m_sliderknob.DeleteObject();
			DestroyWindow();
			delete this;
		}

		void CMasterDlg::OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_slidermaster.GetPos())/16.0f)-40.0f;
			if (m_slidermaster.editing)_pMachine->_outDry = int(helpers::dsp::dB2Amp(db)*256.0f);

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

		
		void CMasterDlg::OnCustomdrawSliderm1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm1.GetPos())/16.0f)-40.0f;
			if (m_sliderm1.editing)_pMachine->SetWireVolume(0,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,112,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm10(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm10.GetPos())/16.0f)-40.0f;
			if (m_sliderm10.editing)_pMachine->SetWireVolume(9,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,328,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm11(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm11.GetPos())/16.0f)-40.0f;
			if (m_sliderm11.editing)_pMachine->SetWireVolume(10,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,352,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm12(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm12.GetPos())/16.0f)-40.0f;
			if (m_sliderm12.editing)_pMachine->SetWireVolume(11,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,376,171);

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
			while(i < MAX_CONNECTIONS)
			{
				dcm->ExtTextOut(xo, y, ETO_CLIPPED, CRect(xo,y,xo+77,y+13), CString(macname[i]), 0);
				i++;
				y += dy;
			}
			dcm->SelectObject(oldfont);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm2.GetPos())/16.0f)-40.0f;
			if (m_sliderm2.editing)_pMachine->SetWireVolume(1,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,136,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm3(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm3.GetPos())/16.0f)-40.0f;
			if (m_sliderm3.editing)_pMachine->SetWireVolume(2,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,160,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm4(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm4.GetPos())/16.0f)-40.0f;
			if (m_sliderm4.editing)_pMachine->SetWireVolume(3,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,184,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm5(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm5.GetPos())/16.0f)-40.0f;
			if (m_sliderm5.editing)_pMachine->SetWireVolume(4,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,208,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm6(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm6.GetPos())/16.0f)-40.0f;
			if (m_sliderm6.editing)_pMachine->SetWireVolume(5,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,232,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm7(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm7.GetPos())/16.0f)-40.0f;
			if (m_sliderm7.editing)_pMachine->SetWireVolume(6,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,256,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm8(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm8.GetPos())/16.0f)-40.0f;
			if (m_sliderm8.editing)_pMachine->SetWireVolume(7,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,280,171);
			
			*pResult = DrawSliderGraphics(pNMHDR);
		}

		void CMasterDlg::OnCustomdrawSliderm9(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((832-m_sliderm9.GetPos())/16.0f)-40.0f;
			if (m_sliderm9.editing)_pMachine->SetWireVolume(8,helpers::dsp::dB2Amp(db));
			PaintNumbers(db,304,171);
			
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
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm1.GetPos())/16.0f)-40.0f,112,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm2.GetPos())/16.0f)-40.0f,136,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm3.GetPos())/16.0f)-40.0f,160,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm4.GetPos())/16.0f)-40.0f,184,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm5.GetPos())/16.0f)-40.0f,208,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm6.GetPos())/16.0f)-40.0f,232,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm7.GetPos())/16.0f)-40.0f,256,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm8.GetPos())/16.0f)-40.0f,280,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm9.GetPos())/16.0f)-40.0f,304,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm10.GetPos())/16.0f)-40.0f,328,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm11.GetPos())/16.0f)-40.0f,352,171);
				PaintNumbersDC(dcm,&memDC,((832-m_sliderm12.GetPos())/16.0f)-40.0f,376,171);
		
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
					m_sliderm1.GetClientRect(r);
					nmcd.rc = r;
					pDC->BitBlt(nmcd.rc.left,nmcd.rc.top,nmcd.rc.right-nmcd.rc.left+1,nmcd.rc.bottom-nmcd.rc.top,&memDC,36,49,SRCCOPY);
					pDC->Detach();
					memDC.SelectObject(oldbmp);
				}
				return CDRF_SKIPDEFAULT;
			}
			else return 0;

		}


		BOOL CMasterDlg::PreTranslateMessage(MSG* pMsg) 
		{
			if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
			{
				m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
			}
			
			return CDialog::PreTranslateMessage(pMsg);
		}

		void CMasterDlg::OnStnClickedMixerview()
		{
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
