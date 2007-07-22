///\file
///\brief implementation file for psycle::host::CDefaultVstGui.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/DefaultVstGui.hpp>
#include <psycle/helpers/helpers.hpp>
#include <psycle/host/ChildView.hpp>
#include <psycle/host/uiconfiguration.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
IMPLEMENT_DYNCREATE(CDefaultVstGui, CFormView)

		CDefaultVstGui::CDefaultVstGui() : CFormView(CDefaultVstGui::IDD)
		{
			//{{AFX_DATA_INIT(CDefaultVstGui)
			//}}AFX_DATA_INIT
		}

		CDefaultVstGui::~CDefaultVstGui()
		{
		}

		void CDefaultVstGui::DoDataExchange(CDataExchange* pDX)
		{
			CFormView::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CDefaultVstGui)
			DDX_Control(pDX, IDC_COMBO1, m_program);
			DDX_Control(pDX, IDC_SLIDER1, m_slider);
			DDX_Control(pDX, IDC_TEXT1, m_text);
			DDX_Control(pDX, IDC_LIST1, m_parlist);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CDefaultVstGui, CFormView)
			//{{AFX_MSG_MAP(CDefaultVstGui)
			ON_WM_VSCROLL()
			ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
			ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnReleasedcaptureSlider1)
			ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
			ON_CBN_CLOSEUP(IDC_COMBO1, OnCloseupCombo1)
			ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
			ON_WM_CREATE()
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		/////////////////////////////////////////////////////////////////////////////
		// CDefaultVstGui diagnostics

		#if !defined  NDEBUG
			void CDefaultVstGui::AssertValid() const
			{
				CFormView::AssertValid();
			}

			void CDefaultVstGui::Dump(CDumpContext& dc) const
			{
				CFormView::Dump(dc);
			}
		#endif //!NDEBUG

		void CDefaultVstGui::Init() 
		{
			UpdateParList();
			InitializePrograms();
			//init slider range
			m_slider.SetRange(0, vst::quantization);
			nPar=0;
			UpdateOne();
		}

		void CDefaultVstGui::InitializePrograms()
		{
			m_program.ResetContent();

			int nump;
			try
			{
				nump = _pMachine->proxy().numPrograms();
			}
			catch(const std::exception &)
			{
				nump = 0;
			}
			for(int i(0) ; i < nump; ++i)
			{
				char s1[256];
				char s2[256];
				std::strcpy(s2, "<unnamed>");
				try
				{
					/// \todo Not used but... needed? (to call before getprogramname)
					//int categories = _pMachine->proxy().dispatcher(effGetNumProgramCategories); categories; // not used
					_pMachine->proxy().dispatcher(effGetProgramNameIndexed, i, -1, s2);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				std::sprintf(s1,"%d: %s",i+1,s2);
				m_program.AddString(s1);
			}
			try
			{
				m_program.SetCurSel(_pMachine->proxy().dispatcher(effGetProgram));
			}
			catch(const std::exception &)
			{
				// o_O`
			}
		}

		void CDefaultVstGui::UpdateParList()
		{
			m_parlist.ResetContent();

			long int params;
			try
			{
				params = _pMachine->proxy().numParams();
			}
			catch(const std::exception &)
			{
				params = 0;
			}
			for(int i(0) ; i < params; ++i)
			{
				char str[128], buf[128];
				std::memset(str, 0, 64);
				try
				{
					_pMachine->proxy().dispatcher(effGetParamName, i, 0, str);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				bool b;
				try
				{
					b = _pMachine->proxy().dispatcher(effCanBeAutomated, i);
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(b) std::sprintf(buf, "(A)%.3X: %s", i, str);
				else std::sprintf(buf, "(_)%.3X: %s", i, str);
				m_parlist.AddString(buf);
			}
			nPar = 0;
			m_parlist.SetCurSel(0);
		}

		void CDefaultVstGui::UpdateText(float value)
		{
			char str[512],str2[32];
			_pMachine->DescribeValue(nPar,str);
			sprintf(str2,"\t[Hex: %4X]",f2i(value*65535.0f));
			strcat(str,str2);
			m_text.SetWindowText(str);
		}

		void CDefaultVstGui::UpdateOne()
		{
			//update scroll bar with initial value
			float value;
			try
			{
				value = _pMachine->proxy().getParameter(nPar);
			}
			catch(const std::exception &)
			{
				value = 0; // hmm
			}
			UpdateText(value);
			value *= vst::quantization;
			updatingvalue =true;
			m_slider.SetPos(vst::quantization -(f2i(value)));
			updatingvalue =false;
		}

		void CDefaultVstGui::UpdateNew(int par,float value)
		{
			if (par != nPar )
			{
				nPar=par;
				m_parlist.SetCurSel(par);
			}
			UpdateText(value);
			value *= vst::quantization;
			updatingvalue=true;
			m_slider.SetPos(vst::quantization -(f2i(value)));
			updatingvalue=false;
		}
		void CDefaultVstGui::OnSelchangeList1() 
		{
			nPar=m_parlist.GetCurSel();
			UpdateOne();
			assert(mainView);
			mainView->SetFocus();
		}

		void CDefaultVstGui::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
		{
			if(!updatingvalue)
			{
				int val1(vst::quantization - m_slider.GetPos());
				float value = static_cast<float>(val1) / vst::quantization;
				try
				{
					_pMachine->proxy().setParameter(nPar, value);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				UpdateText(value);
				// well, this isn't so hard... just put the twk record here
				if(Global::configuration()._RecordTweaks)
				{
					assert(mainView);
					if (Global::configuration()._RecordMouseTweaksSmooth)
						childView->MousePatternTweakSlide(MachineIndex, nPar, val1);
					else
						childView->MousePatternTweak(MachineIndex, nPar, val1);
				}
			}
		}

		void CDefaultVstGui::OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			mainView->SetFocus();
			*pResult = 0;
		}

		void CDefaultVstGui::OnSelchangeCombo1() 
		{
			int const se=m_program.GetCurSel();
			try
			{
				_pMachine->proxy().dispatcher(effSetProgram, 0, se);
			}
			catch(const std::exception &)
			{
				// o_O`
			}
			UpdateOne();
		}

		void CDefaultVstGui::OnCloseupCombo1() 
		{
			assert(mainView);
			mainView->SetFocus();
		}

		void CDefaultVstGui::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
			const int se(m_program.GetCurSel() + pNMUpDown->iDelta);
			if(se >= 0)
			{
				m_program.SetCurSel(se);
				try
				{
					_pMachine->proxy().dispatcher(effSetProgram, 0, se);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
			}
			*pResult = 0;
			mainView->SetFocus();
		}

		int CDefaultVstGui::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if(CFormView::OnCreate(lpCreateStruct) == -1) return -1;
			return 0;
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
