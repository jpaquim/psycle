///\file
///\brief implementation file for psycle::host::CGearTracker.
#include <psycle/host/detail/project.private.hpp>
#include "GearTracker.hpp"
#include "Sampler.hpp"

namespace psycle { namespace host {

		CGearTracker::CGearTracker(CGearTracker** windowVar, Sampler& machineref)
			: CDialog(CGearTracker::IDD, AfxGetMainWnd())
			, machine(machineref)
			, windowVar_(windowVar)
		{
			CDialog::Create(IDD, AfxGetMainWnd());
		}

		void CGearTracker::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_COMBO1, m_interpol);
			DDX_Control(pDX, IDC_TRACKSLIDER2, m_polyslider);
			DDX_Control(pDX, IDC_TRACKLABEL2, m_polylabel);
		}

		BEGIN_MESSAGE_MAP(CGearTracker, CDialog)
			ON_WM_CLOSE()
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_TRACKSLIDER2, OnCustomdrawTrackslider2)
			ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
		END_MESSAGE_MAP()

		BOOL CGearTracker::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_interpol.AddString("None [Lowest quality]");
			m_interpol.AddString("Linear [Low quality]");
			m_interpol.AddString("Spline [Medium Quality]");
			m_interpol.AddString("512p Sinc [Highest Quality]");

			m_interpol.SetCurSel(machine._resampler.quality());

			SetWindowText(machine._editName);

			m_polyslider.SetRange(2, SAMPLER_MAX_POLYPHONY, true);
			m_polyslider.SetPos(machine._numVoices);

			return TRUE;
		}

		void CGearTracker::OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			*pResult = 0;
		}

		void CGearTracker::OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			// Assign new value
			machine._numVoices = m_polyslider.GetPos();

			for(int c=0; c<SAMPLER_MAX_POLYPHONY; c++)
			{
				machine.NoteOffFast(c);
			}

			// Label on dialog display
			char buffer[8];
			sprintf(buffer, "%d", machine._numVoices);
			m_polylabel.SetWindowText(buffer);

			*pResult = 0;
		}

		void CGearTracker::OnSelchangeCombo1() 
		{
			machine._resampler.quality((helpers::dsp::resampler::quality::type)m_interpol.GetCurSel());
		}

		void CGearTracker::OnCancel() {
			DestroyWindow();
		}
		void CGearTracker::OnClose()
		{
			CDialog::OnClose();
			DestroyWindow();
		}
		void CGearTracker::PostNcDestroy()
		{
			if(windowVar_ !=NULL) *windowVar_ = NULL;
			delete this;
		}
}}
