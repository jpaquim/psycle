#include "stdafx.h"
#include "Psycle.h"
#include "DefaultVstGui.h"
#include "Helpers.h"
#include "ChildView.h"
#include "configuration.h"
///\file
///\brief implementation file for psycle::host::CDefaultVstGui.
namespace psycle
{
	namespace host
	{
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

		#ifdef _DEBUG
			void CDefaultVstGui::AssertValid() const
			{
				CFormView::AssertValid();
			}

			void CDefaultVstGui::Dump(CDumpContext& dc) const
			{
				CFormView::Dump(dc);
			}
		#endif //_DEBUG

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

			const int nump = _pMachine->NumPrograms();
			for (int i=0; i<nump; i++)
			{
				char s1[256];
				char s2[256];
				strcpy(s2, "<unnamed>");

				int categories = _pMachine->Dispatch(effGetNumProgramCategories, 0, 0, NULL, 0); categories; // not used
				_pMachine->Dispatch(effGetProgramNameIndexed, i, -1, s2, 0);

				sprintf(s1,"%d: %s",i+1,s2);
				m_program.AddString(s1);
			}
			m_program.SetCurSel(_pMachine->Dispatch(effGetProgram, 0, 0, NULL, 0));
		}

		void CDefaultVstGui::UpdateParList()
		{
			m_parlist.ResetContent();

			const long params = _pMachine->NumParameters();
			for (int i=0; i<params; i++)
			{
				char str[128], buf[128];

				memset(str,0,64);
				_pMachine->Dispatch(effGetParamName, i, 0, str, 0);
				if (_pMachine->Dispatch(effCanBeAutomated, i, 0, NULL, 0))
				{
					sprintf(buf, "(A)%.3X: %s", i, str);
				}
				else
				{
					sprintf(buf, "(_)%.3X: %s", i, str);
				}
				m_parlist.AddString(buf);
			}
			
			nPar=0;
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
			float value = _pMachine->GetParameter(nPar);
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
				_pMachine->SetParameter(nPar, value);
				UpdateText(value);
				// well, this isn't so hard... just put the twk record here
				if(Global::pConfig->_RecordTweaks)
				{
					assert(mainView);
					if (Global::pConfig->_RecordMouseTweaksSmooth)
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
			_pMachine->SetCurrentProgram(se);
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
				_pMachine->SetCurrentProgram(se);
			}
			*pResult = 0;
			mainView->SetFocus();
		}

		int CDefaultVstGui::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if(CFormView::OnCreate(lpCreateStruct) == -1) return -1;
			return 0;
		}
	}
}
