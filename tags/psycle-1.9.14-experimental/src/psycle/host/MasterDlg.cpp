///\file
///\brief implementation file for psycle::host::CMasterDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/ChildView.hpp>
#include <psycle/host/MasterDlg.hpp>
#include <psycle/helpers/Dsp.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CMasterDlg::CMasterDlg(CChildView* pParent)
		:
			CDialog(CMasterDlg::IDD, pParent),
			m_pParent(pParent)
		{}

		void CMasterDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_MASTERPEAK, m_masterpeak);
			DDX_Control(pDX, IDC_SLIDERMASTER, m_slidermaster);
			#define sad_tools(_, count, __) DDX_Control(pDX, BOOST_PP_CAT(IDC_SLIDERM, BOOST_PP_INC(count)), m_sliderm[count]);
				BOOST_PP_REPEAT(PSYCLE__MAX_CONNECTIONS, sad_tools, ~)
			#undef sad_tools
			DDX_Control(pDX, IDC_MIXERVIEW, m_mixerview);
			DDX_Control(pDX, IDC_AUTODEC, m_autodec);
		}

		BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
			ON_BN_CLICKED(IDC_AUTODEC, OnAutodec)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERMASTER, OnCustomdrawSlidermaster)
			#define sad_tools(_, count, __) ON_NOTIFY(NM_CUSTOMDRAW, BOOST_PP_CAT(IDC_SLIDERM, BOOST_PP_INC(count)), OnCustomdrawSliderm##count)
				BOOST_PP_REPEAT(PSYCLE__MAX_CONNECTIONS, sad_tools, ~)
			#undef sad_tools
			ON_WM_PAINT()
			ON_STN_CLICKED(IDC_MIXERVIEW, OnStnClickedMixerview)
		END_MESSAGE_MAP()

		void CMasterDlg::OnStnClickedMixerview()
		{
		}

		BOOL CMasterDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			
			namesFont.CreatePointFont(80,"Tahoma");
			m_numbers.LoadBitmap(IDB_MASTERNUMBERS);
			
			m_slidermaster.SetRange(0, 208);
			m_slidermaster.SetPageSize(16);

			for(int i(0); i < MAX_CONNECTIONS; ++i)
			{
				m_sliderm[i].SetRange(0, 208);
				m_sliderm[i].SetPageSize(16);
			}

			SetSliderValues();
			if (((Master*)_pMachine)->decreaseOnClip) m_autodec.SetCheck(1);
			else m_autodec.SetCheck(0);
			return true;
		}

		void CMasterDlg::SetSliderValues()
		{
			float val;
			float db = dsp::dB(_pMachine->_outDry/256.0f);
			m_slidermaster.SetPos(208-(int)((db+40.0f)*4.0f));

			for(int i(0); i < MAX_CONNECTIONS; ++i)
			{
				if (_pMachine->_inputCon[i])
				{
					_pMachine->GetWireVolume(Wire::id_type(i),val);
					m_sliderm[i].SetPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
				}
				else
				{
					m_sliderm[i].SetPos(208);
				}
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
			//CBitmap* oldbmp;
			//memDC.CreateCompatibleDC(dc);
			//oldbmp = memDC.SelectObject(&m_numbers);
			
			PaintNumbersDC(dc,&memDC,val,x,y);

			//memDC.SelectObject(oldbmp);
			//memDC.DeleteDC();
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
			}
			else
			{
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

		#define sad_tools(_, count, __) \
			void CMasterDlg::OnCustomdrawSliderm##count(NMHDR* pNMHDR, LRESULT* pResult) \
			{ \
				float db = ((208-m_sliderm[count].GetPos())/4.0f)-40.0f; \
				_pMachine->SetWireVolume(Wire::id_type(count),dsp::dB2Amp(db)); \
				PaintNumbers(db, 92 + 20 * count, 142); \
				if(count == MAX_CONNECTIONS - 1) \
				{ \
					/* <some-anonymous-person-wrote> I know the following is Ugly, but it is the only solution I've found, because first, */ \
					/* OnPaint is called, after the bitmap is drawn, and finally the sliders are redrawn. */ \
					paint_names_in_grid(); \
				} \
				*pResult = 0; \
			}
			BOOST_PP_REPEAT(PSYCLE__MAX_CONNECTIONS, sad_tools, ~)
		#undef sad_tools

		void CMasterDlg::paint_names_in_grid()
		{
			CDC *dcm = m_mixerview.GetDC();
			CFont* oldfont = dcm->SelectObject(&namesFont);
			dcm->SetTextColor(0x00FFFFFF); // White
			dcm->SetBkColor(0x00000000); // Black
			int const xo(353);
			int const dx(75);
			int const yo(24);
			int const dy(22);
			int const columns(2);
			int y(yo);
			int i(0);
			while(i < MAX_CONNECTIONS)
			{
				int x(xo);
				for(int column(0); column < columns && i < MAX_CONNECTIONS; ++column, ++i)
				{
					dcm->TextOut(x, y, macname[i].c_str());
					x += dx;
				}
				y += dy;
			}
			dcm->SelectObject(oldfont);
		}

		void CMasterDlg::OnPaint() 
		{
			CPaintDC dc(this); // device context for painting
			
			if ( dc.m_ps.rcPaint.bottom >= 145 && dc.m_ps.rcPaint.top <= 155)
			{
				CDC *dcm = m_mixerview.GetDC();
				CDC memDC;
				//CBitmap* oldbmp;
				//memDC.CreateCompatibleDC(dcm);
				//oldbmp = memDC.SelectObject(&m_numbers);
				
				PaintNumbersDC(dcm,&memDC,((208-m_slidermaster.GetPos())/4.0f)-40.0f,32,142);
				for(int i(0); i < MAX_CONNECTIONS; ++i)
					PaintNumbersDC(dcm,&memDC,((208-m_sliderm[i].GetPos())/4.0f)-40.0f,92,142);
		
				//memDC.SelectObject(oldbmp);
				//memDC.DeleteDC();
			}
			if ( dc.m_ps.rcPaint.bottom >=25 && dc.m_ps.rcPaint.top<=155 && dc.m_ps.rcPaint.right >=350)
			{
				paint_names_in_grid();
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

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
