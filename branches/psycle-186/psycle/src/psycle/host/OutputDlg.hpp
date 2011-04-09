///\file
///\interface file for psycle::host::COutputDlg.
#pragma once
#include "Psycle.hpp"

namespace psycle { namespace host {
		class AudioDriver;

		/// audio device output config window.
		class COutputDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(COutputDlg)
		public:
			COutputDlg();
			enum { IDD = IDD_OUTPUT };
			CComboBox       m_driverComboBox;
			CComboBox       m_midiDriverComboBox;
			CButton         m_inmediate;
			CButton         m_sequenced;
			CComboBox       m_midiSyncComboBox;
			CEdit           m_midiHeadroomEdit;
			CSpinButtonCtrl m_midiHeadroomSpin;
		public:
			int m_driverIndex;
			int m_midiDriverIndex;
			int m_syncDriverIndex;
			int m_midiHeadroom;
		private:
			int m_oldDriverIndex;
			int m_oldMidiDriverIndex;
			int m_oldSyncDriverIndex;
			int const static MIN_HEADROOM = 0;
			int const static MAX_HEADROOM = 9999;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);
			void EnableClockOptions();
			void DisableClockOptions();
			void PopulateListbox( CComboBox * listbox, int numDevs, bool issync );
		protected:
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();
			afx_msg void OnConfig();
			afx_msg void OnSelChangeOutput();
			afx_msg void OnSelChangeMidi();
			afx_msg void OnSelChangeSync();
			afx_msg void OnEnableInmediate();
			afx_msg void OnEnableSequenced();
			DECLARE_MESSAGE_MAP()
		};

	}   // namespace
}   // namespace
