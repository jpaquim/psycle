#pragma once
#include "AudioDriver.h"
///\file
///\brief interface file for psycle::host::COutputDlg.
namespace psycle
{
	namespace host
	{
		#define	MIN_HEADROOM	0
		#define	MAX_HEADROOM	9999

		/// audio device output config window.
		class COutputDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(COutputDlg)
		// Construction
		public:
			int m_driverIndex;
			int m_midiDriverIndex; // MIDI IMPLEMENTATION
			int m_syncDriverIndex;
			int _numDrivers;
			int _numMidiDrivers; // MIDI IMPLEMENTATION
			int m_midiHeadroom;
			AudioDriver** m_ppDrivers;
			COutputDlg();
		// Dialog Data
			//{{AFX_DATA(COutputDlg)
			enum { IDD = IDD_OUTPUT };
			CComboBox	m_midiDriverComboBox;
			CComboBox	m_midiSyncComboBox;
			CComboBox	m_driverComboBox;
			CEdit		m_midiHeadroomEdit;
			CSpinButtonCtrl m_midiHeadroomSpin;
			CButton		m_midiMachineViewSeqMode;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(COutputDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			int m_oldDriverIndex;
			int m_oldMidiDriverIndex; // MIDI IMPLEMENTATION
			int m_oldSyncDriverIndex;
			// Generated message map functions
			//{{AFX_MSG(COutputDlg)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();
			afx_msg void OnConfig();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
