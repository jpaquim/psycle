///\file
///\brief implementation file for psycle::host::CGearTracker.

#include "GearTracker.hpp"
#include "Psycle.hpp"
#include "ChildView.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		CGearTracker::CGearTracker(CChildView* pParent)
			: CDialog(CGearTracker::IDD, pParent)
		{
			m_pParent = pParent;
			//{{AFX_DATA_INIT(CGearTracker)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
		}

		void CGearTracker::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CGearTracker)
			DDX_Control(pDX, IDC_COMBO1, m_interpol);
			DDX_Control(pDX, IDC_TRACKSLIDER2, m_polyslider);
			DDX_Control(pDX, IDC_TRACKLABEL2, m_polylabel);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CGearTracker, CDialog)
			//{{AFX_MSG_MAP(CGearTracker)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_TRACKSLIDER2, OnCustomdrawTrackslider2)
			ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CGearTracker::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_interpol.AddString("None [Lowest quality]");
			m_interpol.AddString("Linear [Low quality]");
			m_interpol.AddString("Spline [Medium Quality]");
			m_interpol.AddString("512p Sinc [Highest Quality]");

			m_interpol.SetCurSel(_pMachine->_resampler.GetQuality());

			SetWindowText(_pMachine->_editName);

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
			_pMachine->_resampler.SetQuality((helpers::dsp::ResamplerQuality)m_interpol.GetCurSel());
		}

		BOOL CGearTracker::Create()
		{
			return CDialog::Create(IDD, m_pParent);
		}

		void CGearTracker::OnCancel()
		{
			m_pParent->SamplerMachineDialog = NULL;
			DestroyWindow();
			delete this;
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
