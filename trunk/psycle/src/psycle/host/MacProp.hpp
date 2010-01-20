///\file
///\brief interface file for psycle::host::CMacProp.
#pragma once
#include "Psycle.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
using namespace psycle::core;
#endif

namespace psycle {
	namespace host {

		class CChildView;
		class MachineGui;

		/// machine properties window.
		class CMacProp : public CDialog
		{
		public:
			CMacProp(class MachineGui* gui);

			///\ todo should be private
			char txt[32];
			bool deleted;
			bool replaced;

		private:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			afx_msg void OnChangeEdit1();
			afx_msg void OnButton1();
			afx_msg void OnMute();
			afx_msg void OnBypass();
			afx_msg void OnSolo();
			afx_msg void OnClone();
			DECLARE_MESSAGE_MAP()
			afx_msg void OnBnClickedReplacemac();

			MachineGui* gui_;			
			enum { IDD = IDD_MACPROP };
			CButton	m_soloCheck;
			CButton	m_bypassCheck;
			CButton	m_muteCheck;
			CEdit	m_macname;
		};

	}   // namespace host
}   // namespace psycle
