///\file
///\brief implementation file for psycle::host::CMasterDlg.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/gui/Psycle.hpp>
#include <psycle/host/gui/ChildView.hpp>
#include <psycle/host/gui/MasterDlg.hpp>
#include <psycle/host/engine/Dsp.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CMasterDlg::CMasterDlg(CChildView* pParent) : CDialog(CMasterDlg::IDD, pParent)
		{
			m_pParent = pParent;
			//{{AFX_DATA_INIT(CMasterDlg)
			//}}AFX_DATA_INIT
			memset(macname,0,32*MAX_CONNECTIONS);
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
			
			namesFont.CreatePointFont(80,"Tahoma");
			m_numbers.LoadBitmap(IDB_MASTERNUMBERS);
			
			m_slidermaster.SetRange(0, 208);
			m_sliderm1.SetRange(0, 208);
			m_sliderm2.SetRange(0, 208);
			m_sliderm3.SetRange(0, 208);
			m_sliderm4.SetRange(0, 208);
			m_sliderm5.SetRange(0, 208);
			m_sliderm6.SetRange(0, 208);
			m_sliderm7.SetRange(0, 208);
			m_sliderm8.SetRange(0, 208);
			m_sliderm9.SetRange(0, 208);
			m_sliderm10.SetRange(0, 208);
			m_sliderm11.SetRange(0, 208);
			m_sliderm12.SetRange(0, 208);
			
			m_slidermaster.SetPageSize(16);
			m_sliderm1.SetPageSize(16);
			m_sliderm2.SetPageSize(16);
			m_sliderm3.SetPageSize(16);
			m_sliderm4.SetPageSize(16);
			m_sliderm5.SetPageSize(16);
			m_sliderm6.SetPageSize(16);
			m_sliderm7.SetPageSize(16);
			m_sliderm8.SetPageSize(16);
			m_sliderm9.SetPageSize(16);
			m_sliderm10.SetPageSize(16);
			m_sliderm11.SetPageSize(16);
			m_sliderm12.SetPageSize(16);

			SetSliderValues();
			if (((Master*)_pMachine)->decreaseOnClip) m_autodec.SetCheck(1);
			else m_autodec.SetCheck(0);
			return TRUE;
		}

		void CMasterDlg::SetSliderValues()
		{
			float val;
			float db = dsp::dB(_pMachine->_outDry/256.0f);
			m_slidermaster.SetPos(208-(int)((db+40.0f)*4.0f));

			if (_pMachine->_inputCon[0])
			{
				_pMachine->GetWireVolume(0,val);
				m_sliderm1.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm1.SetPos(208);
			}

			if (_pMachine->_inputCon[1])
			{
				_pMachine->GetWireVolume(1,val);
				m_sliderm2.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm2.SetPos(208);
			}

			if (_pMachine->_inputCon[2])
			{
				_pMachine->GetWireVolume(2,val);
				m_sliderm3.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm3.SetPos(208);
			}

			if (_pMachine->_inputCon[3])
			{
				_pMachine->GetWireVolume(3,val);
				m_sliderm4.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm4.SetPos(208);
			}

			if (_pMachine->_inputCon[4])
			{
				_pMachine->GetWireVolume(4,val);
				m_sliderm5.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm5.SetPos(208);
			}

			if (_pMachine->_inputCon[5])
			{
				_pMachine->GetWireVolume(5,val);
				m_sliderm6.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm6.SetPos(208);
			}

			if (_pMachine->_inputCon[6])
			{
				_pMachine->GetWireVolume(6,val);
				m_sliderm7.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm7.SetPos(208);
			}

			if (_pMachine->_inputCon[7])
			{
				_pMachine->GetWireVolume(7,val);
				m_sliderm8.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm8.SetPos(208);
			}

			if (_pMachine->_inputCon[8])
			{
				_pMachine->GetWireVolume(8,val);
				m_sliderm9.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm9.SetPos(208);
			}

			if (_pMachine->_inputCon[9])
			{
				_pMachine->GetWireVolume(9,val);
				m_sliderm10.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm10.SetPos(208);
			}

			if (_pMachine->_inputCon[10])
			{
				_pMachine->GetWireVolume(10,val);
				m_sliderm11.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm11.SetPos(208);
			}

			if (_pMachine->_inputCon[11])
			{
				_pMachine->GetWireVolume(11,val);
				m_sliderm12.SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
			} else {
				m_sliderm12.SetPos(208);
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
					sprintf(peak,"%.2fdB",dsp::dB(_pMachine->currentpeak*0.00003051f));
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
			DestroyWindow();
			delete this;
		}


		void CMasterDlg::OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_slidermaster.GetPos())/4.0f)-40.0f;
			_pMachine->_outDry = int(dsp::dB2Amp(db)*256.0f);

			PaintNumbers(db,32,142);
			
			*pResult = 0;
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
			float db = ((208-m_sliderm1.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(0,dsp::dB2Amp(db));
			PaintNumbers(db,92,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm10(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm10.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(9,dsp::dB2Amp(db));
			PaintNumbers(db,272,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm11(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm11.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(10,dsp::dB2Amp(db));
			PaintNumbers(db,292,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm12(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm12.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(11,dsp::dB2Amp(db));
			PaintNumbers(db,312,142);

		// I know the following is Ugly, but it is the only solution I've found, because first,
		// OnPaint is called, after the bitmap is drawn, and finally the sliders are redrawn.
			CDC *dcm = m_mixerview.GetDC();
			CFont* oldfont = dcm->SelectObject(&namesFont);
			dcm->SetTextColor(0x00FFFFFF); // White
			dcm->SetBkColor(0x00000000); // Black
			dcm->TextOut(353,24,macname[0]);
			dcm->TextOut(428,24,macname[1]);
			dcm->TextOut(353,46,macname[2]);
			dcm->TextOut(428,46,macname[3]);
			dcm->TextOut(353,68,macname[4]);
			dcm->TextOut(428,68,macname[5]);
			dcm->TextOut(353,90,macname[6]);
			dcm->TextOut(428,90,macname[7]);
			dcm->TextOut(353,112,macname[8]);
			dcm->TextOut(428,112,macname[9]);
			dcm->TextOut(353,134,macname[10]);
			dcm->TextOut(428,134,macname[11]);
			dcm->SelectObject(oldfont);	
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm2.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(1,dsp::dB2Amp(db));
			PaintNumbers(db,112,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm3(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm3.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(2,dsp::dB2Amp(db));
			PaintNumbers(db,132,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm4(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm4.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(3,dsp::dB2Amp(db));
			PaintNumbers(db,152,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm5(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm5.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(4,dsp::dB2Amp(db));
			PaintNumbers(db,172,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm6(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm6.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(5,dsp::dB2Amp(db));
			PaintNumbers(db,192,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm7(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm7.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(6,dsp::dB2Amp(db));
			PaintNumbers(db,212,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm8(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm8.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(7,dsp::dB2Amp(db));
			PaintNumbers(db,232,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnCustomdrawSliderm9(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			float db = ((208-m_sliderm9.GetPos())/4.0f)-40.0f;
			_pMachine->SetWireVolume(8,dsp::dB2Amp(db));
			PaintNumbers(db,252,142);
			
			*pResult = 0;
		}

		void CMasterDlg::OnPaint() 
		{
			CPaintDC dc(this); // device context for painting
			
			if ( dc.m_ps.rcPaint.bottom >= 145 && dc.m_ps.rcPaint.top <= 155)
			{
				CDC *dcm = m_mixerview.GetDC();
				CDC memDC;
//				CBitmap* oldbmp;
//				memDC.CreateCompatibleDC(dcm);
//				oldbmp = memDC.SelectObject(&m_numbers);
				
				PaintNumbersDC(dcm,&memDC,((208-m_slidermaster.GetPos())/4.0f)-40.0f,32,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm1.GetPos())/4.0f)-40.0f,92,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm2.GetPos())/4.0f)-40.0f,112,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm3.GetPos())/4.0f)-40.0f,132,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm4.GetPos())/4.0f)-40.0f,152,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm5.GetPos())/4.0f)-40.0f,172,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm6.GetPos())/4.0f)-40.0f,192,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm7.GetPos())/4.0f)-40.0f,212,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm8.GetPos())/4.0f)-40.0f,232,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm9.GetPos())/4.0f)-40.0f,252,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm10.GetPos())/4.0f)-40.0f,272,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm11.GetPos())/4.0f)-40.0f,292,142);
				PaintNumbersDC(dcm,&memDC,((208-m_sliderm12.GetPos())/4.0f)-40.0f,312,142);
		
//				memDC.SelectObject(oldbmp);
//				memDC.DeleteDC();
			}
			if ( dc.m_ps.rcPaint.bottom >=25 && dc.m_ps.rcPaint.top<=155 && dc.m_ps.rcPaint.right >=350)
			{
				CDC *dcm = m_mixerview.GetDC();
				CFont* oldfont = dcm->SelectObject(&namesFont);
				dcm->SetTextColor(0x00FFFFFF); // White
				dcm->SetBkColor(0x00000000); // Black
				dcm->TextOut(353,24,macname[0]);
				dcm->TextOut(428,24,macname[1]);
				dcm->TextOut(353,46,macname[2]);
				dcm->TextOut(428,46,macname[3]);
				dcm->TextOut(353,68,macname[4]);
				dcm->TextOut(428,68,macname[5]);
				dcm->TextOut(353,90,macname[6]);
				dcm->TextOut(428,90,macname[7]);
				dcm->TextOut(353,112,macname[8]);
				dcm->TextOut(428,112,macname[9]);
				dcm->TextOut(353,134,macname[10]);
				dcm->TextOut(428,134,macname[11]);
				dcm->SelectObject(oldfont);
			}
			// Do not call CDialog::OnPaint() for painting messages
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
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
