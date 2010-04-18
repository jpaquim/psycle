///\file
///\brief implementation file for psycle::host::CGearTracker.
#include "GearTracker.hpp"

#include <psycle/helpers/dsp.hpp>
#include <psycle/core/sampler.h>

#include "ChildView.hpp"
#include "MachineGui.hpp"
#include "MachineView.hpp"

namespace psycle { namespace host {

		BEGIN_MESSAGE_MAP(CGearTracker, CDialog)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_TRACKSLIDER2, OnCustomdrawTrackslider2)
			ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
		END_MESSAGE_MAP()

		CGearTracker::CGearTracker(CChildView* pParent)
			: CDialog(CGearTracker::IDD, pParent),
			  m_pParent(pParent) {
		}

		CGearTracker::CGearTracker(MachineGui* gui)
			: CDialog(CGearTracker::IDD, gui->view()->child_view()),
			  gui_(gui),
			  m_pParent(gui->view()->child_view()),
				_pMachine((Sampler*)gui->mac()) {

		}

		void CGearTracker::DoDataExchange(CDataExchange* pDX) {
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_COMBO1, m_interpol);
			DDX_Control(pDX, IDC_TRACKSLIDER2, m_polyslider);
			DDX_Control(pDX, IDC_TRACKLABEL2, m_polylabel);
		}

		BOOL CGearTracker::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_interpol.AddString("None [Lowest quality]");
			m_interpol.AddString("Linear [Low quality]");
			m_interpol.AddString("Spline [Medium Quality]");
			m_interpol.AddString("512p Sinc [Highest Quality]");

			m_interpol.SetCurSel(_pMachine->resampler_.quality());

			SetWindowText(_pMachine->GetEditName().c_str());

			m_polyslider.SetRange(2, SAMPLER_MAX_POLYPHONY, true);
			m_polyslider.SetPos(_pMachine->_numVoices);

			return TRUE;
		}

		void CGearTracker::OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			*pResult = 0;
		}

		void CGearTracker::OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			// Assign new value
			_pMachine->_numVoices = m_polyslider.GetPos();

			for(int c=0; c<SAMPLER_MAX_POLYPHONY; c++)
			{
				_pMachine->NoteOffFast(c);
			}

			// Label on dialog display
			char buffer[8];
			sprintf(buffer, "%d", _pMachine->_numVoices);
			m_polylabel.SetWindowText(buffer);

			*pResult = 0;
		}

		void CGearTracker::OnSelchangeCombo1() 
		{
			_pMachine->resampler_.quality((psycle::helpers::dsp::resampler::quality::type)m_interpol.GetCurSel());
		}

		BOOL CGearTracker::Create()
		{
			return CDialog::Create(IDD, m_pParent);
		}

		void CGearTracker::OnCancel()
		{
			if (gui_)
			  gui_->BeforeDeleteDlg();
			DestroyWindow();
			delete this;
		}

	}   // namespace host
}   // namespace psycle
