///\file
///\brief implementation file for psycle::host::CVstParamList.
#include <project.private.hpp>
#include "Psycle.hpp"
#include "VstParamList.hpp"
#include "vsthost24.hpp"
//#include "Helpers.hpp"
//#include "ChildView.hpp"
//#include "configuration.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

/*****************************************************************************/
/* Create : creates the dialog                                               */
/*****************************************************************************/

		CVstParamList::CVstParamList(vst::plugin* effect)
		: _pMachine(effect)
		, _mainView(0)
		, _quantizedvalue(0)
		{
		}
		CVstParamList::~CVstParamList()
		{
		}
		void CVstParamList::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_CMBPROGRAM, m_program);
			DDX_Control(pDX, IDC_SLIDERPARAM, m_slider);
			DDX_Control(pDX, IDC_STATUSPARAM, m_text);
			DDX_Control(pDX, IDC_LISTPARAM, m_parlist);
		}

		BEGIN_MESSAGE_MAP(CVstParamList, CDialog)
			ON_WM_VSCROLL()
			ON_LBN_SELCHANGE(IDC_LISTPARAM, OnSelchangeList)
			ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERPARAM, OnReleasedcaptureSlider)
			ON_CBN_SELCHANGE(IDC_CMBPROGRAM, OnSelchangeProgram)
			ON_CBN_CLOSEUP(IDC_CMBPROGRAM, OnCloseupProgram)
			ON_NOTIFY(UDN_DELTAPOS, IDC_SPINPARAM, OnDeltaposSpin)
			ON_WM_CREATE()
		END_MESSAGE_MAP()

		/////////////////////////////////////////////////////////////////////////////
		// CVstParamList diagnostics

		#if !defined  NDEBUG
			void CVstParamList::AssertValid() const
			{
				CDialog::AssertValid();
			}

			void CVstParamList::Dump(CDumpContext& dc) const
			{
				CDialog::Dump(dc);
			}
		#endif //!NDEBUG

		BOOL CVstParamList::Create(CWnd* pParentWnd) 
		{
			return CDialog::Create(IDD, pParentWnd);
		}

		BOOL CVstParamList::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			Init();
			return TRUE;
		}

		void CVstParamList::Init() 
		{
			UpdateParList();
			InitializePrograms();
			m_slider.SetRange(0, vst::quantization);
			UpdateOne();
		}

		void CVstParamList::InitializePrograms()
		{
			m_program.ResetContent();

			const int nump = machine().numPrograms();
			for(int i(0) ; i < nump; ++i)
			{
				char s1[256];
				char s2[256];
				machine().GetProgramNameIndexed(-1, i, s2);
				std::sprintf(s1,"%d: %s",i,s2);
				m_program.AddString(s1);
			}
			m_program.SetCurSel(machine().GetProgram());
		}

		void CVstParamList::UpdateParList()
		{
			const int nPar= m_parlist.GetCurSel();
			m_parlist.ResetContent();

			const long int params = machine().numParams();
			for(int i(0) ; i < params; ++i)
			{
				char str[128], buf[128];
				std::memset(str, 0, 64);
				machine().GetParamName(i, str);
				bool b = machine().CanBeAutomated(i);
				if(b) std::sprintf(buf, "(A)%.3X: %s", i, str);
				else std::sprintf(buf, "(_)%.3X: %s", i, str);
				m_parlist.AddString(buf);
			}
			if ( nPar != -1 )
				m_parlist.SetCurSel(nPar);
			else 
				m_parlist.SetCurSel(0);
		}

		void CVstParamList::UpdateText(int value)
		{
			char str[512],str2[32];
			machine().DescribeValue(m_parlist.GetCurSel(),str);
			std::sprintf(str2,"\t[Hex: %4X]",value);
			std::strcat(str,str2);
			m_text.SetWindowText(str);
		}

		void CVstParamList::UpdateOne()
		{
			int i = m_parlist.GetCurSel();
			int value = machine().GetParamValue(m_parlist.GetCurSel());
			UpdateText(value);
			_quantizedvalue = value;
			m_slider.SetPos(vst::quantization - _quantizedvalue);
		}

		void CVstParamList::UpdateNew(int par,float value)
		{
			if (par != m_parlist.GetCurSel() )
				m_parlist.SetCurSel(par);

			value *= vst::quantization;
			UpdateText(value);
			_quantizedvalue = (f2i(value));
			m_slider.SetPos(vst::quantization - _quantizedvalue);
		}
		void CVstParamList::OnSelchangeList() 
		{
			UpdateOne();
			if (_mainView) _mainView->SetFocus();
		}

		void CVstParamList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
		{

			const int nVal = vst::quantization - m_slider.GetPos();

			if(nVal != _quantizedvalue)
			{
				machine().SetParameter(m_parlist.GetCurSel(), nVal);
				UpdateText(nVal);
				///\todo:
/*				if(Global::pConfig->_RecordTweaks)
				{
					assert(childView);
					if (Global::pConfig->_RecordMouseTweaksSmooth)
						childView->MousePatternTweakSlide(MachineIndex, m_parlist.GetCurSel(), nVal);
					else
						childView->MousePatternTweak(MachineIndex, m_parlist.GetCurSel(), nVal);
				}
*/
			}
		}

		void CVstParamList::OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			if (_mainView) _mainView->SetFocus();
			*pResult = 0;
		}

		void CVstParamList::OnSelchangeProgram() 
		{
			int const se=m_program.GetCurSel();
			_pMachine->SetProgram(se);
			UpdateOne();
			if (_mainView) _mainView->SetFocus();
		}

		void CVstParamList::OnCloseupProgram() 
		{
			if (_mainView) _mainView->SetFocus();
		}

		void CVstParamList::OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
			const int se(m_program.GetCurSel() + pNMUpDown->iDelta);
			if(se >= 0 && se < machine().numPrograms())
			{
				m_program.SetCurSel(se);
				machine().SetProgram(se);
			}
			if (_mainView) _mainView->SetFocus();
			*pResult = 0;
		}

	NAMESPACE__END
NAMESPACE__END
