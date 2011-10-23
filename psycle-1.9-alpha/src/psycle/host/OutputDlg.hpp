///\file
///\interface file for psycle::host::COutputDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include <psycle/audiodrivers/audiodriver.h>

namespace psycle { namespace host {

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
			audiodrivers::AudioDriver** m_ppDrivers;
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

	}   // namespace
}   // namespace
