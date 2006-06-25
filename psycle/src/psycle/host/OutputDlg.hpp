///\file
///\interface file for psycle::host::COutputDlg.
#pragma once
#include <psycle/host/audiodrivers/AudioDriver.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// audio device output config window.
		class COutputDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(COutputDlg)
			DECLARE_MESSAGE_MAP()
		public:
			COutputDlg();
			int const static IDD = IDD_OUTPUT;
			CComboBox       m_midiDriverComboBox;
			CComboBox       m_midiSyncComboBox;
			CComboBox       m_driverComboBox;
			CEdit           m_midiHeadroomEdit;
			CSpinButtonCtrl m_midiHeadroomSpin;
			CButton         m_midiMachineViewSeqMode;
		public:
			int m_driverIndex;
			int m_midiDriverIndex;
			int m_syncDriverIndex;
			int _numDrivers;
			int _numMidiDrivers;
			int m_midiHeadroom;
			AudioDriver** m_ppDrivers;
		private:
			int m_oldDriverIndex;
			int m_oldMidiDriverIndex;
			int m_oldSyncDriverIndex;
			int const static MIN_HEADROOM = 0;
			int const static MAX_HEADROOM	= 9999;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);
		protected:
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();
			afx_msg void OnConfig();
		};
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
