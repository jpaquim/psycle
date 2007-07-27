///\file
///\brief implementation file for psycle::host::CGearTracker.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/GearTracker.hpp>
#include <psycle/host/ChildView.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
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
			m_interpol.AddString("Linear [Medium quality]");
			m_interpol.AddString("Spline [Higher Quality]");
			m_interpol.AddString("Bandlimited [Higher Quality]");

			m_interpol.SetCurSel(_pMachine->_resampler.GetQuality());

			SetWindowText(_pMachine->GetEditName().c_str());

			m_polyslider.SetRange(2, SAMPLER_MAX_POLYPHONY, true);
			m_polyslider.SetPos(_pMachine->_numVoices);

			return true;
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
			_pMachine->_resampler.SetQuality((dsp::ResamplerQuality)m_interpol.GetCurSel());

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
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
