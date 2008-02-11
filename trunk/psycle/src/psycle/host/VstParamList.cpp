/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief implementation file for psycle::host::CVstParamList.
#include <psycle/project.private.hpp>
#include "VstParamList.hpp"
#include "Psycle.hpp"
#include "vsthost24.hpp"
//#include "Helpers.hpp"
//#include "configuration.hpp"
///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
#include "MainFrm.hpp"
#include "ChildView.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

	extern CPsycleApp theApp;

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
/*
	MSDN:
 	When you implement a modeless dialog box, always override the OnCancel member
    function and call DestroyWindow from within it. Don't call the base class
	CDialog::OnCancel, because it calls EndDialog, which will make the dialog box
	invisible but will not destroy it. You should also override PostNcDestroy for
	modeless dialog boxes in order to delete this, since modeless dialog boxes are
	usually allocated with new. Modal dialog boxes are usually constructed on the
	frame and do not need PostNcDestroy cleanup.

	I don't do that, because the default behaviour of hiding, and deleting when the
	parent window is closed is ok for this task.
 */
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
				char s1[kVstMaxProgNameLen+7];
				char s2[kVstMaxProgNameLen+1];
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
				char str[kVstMaxProgNameLen+9], buf[kVstMaxProgNameLen+1];
				std::memset(str, 0, kVstMaxProgNameLen+9);
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
			char str[kVstMaxProgNameLen*3],str2[14];
			machine().DescribeValue(m_parlist.GetCurSel(),str);
			std::sprintf(str2,"\t[Hex: %4X]",value);
			std::strcat(str,str2);
			m_text.SetWindowText(str);
		}

		void CVstParamList::UpdateOne()
		{
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
			_quantizedvalue = (helpers::math::rounded(value));
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
				///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
				if(Global::configuration()._RecordTweaks)
				{
					if(Global::configuration()._RecordMouseTweaksSmooth)
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(machine()._macIndex, m_parlist.GetCurSel(), nVal);
					else
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(machine()._macIndex, m_parlist.GetCurSel(), nVal);
				}
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

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
