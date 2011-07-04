///\file
///\brief interface file for psycle::host::CGearTracker.
#pragma once
#include "Psycle.hpp"

namespace psycle {
namespace host {

		class Sampler;

		/// gear tracker window.
		class CGearTracker : public CDialog
		{
		public:
			CGearTracker(CGearTracker** windowVar, Sampler& machineref);
		// Dialog Data
			enum { IDD = IDD_GEAR_TRACKER };
			CComboBox	m_interpol;
			CSliderCtrl	m_polyslider;
			CStatic	m_polylabel;
			Sampler& machine;
			CGearTracker** windowVar_;
		// Overrides
		protected:
			virtual BOOL OnInitDialog();
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual void PostNcDestroy();
		// Implementation
		protected:
			afx_msg void OnClose();
			afx_msg void OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnSelchangeCombo1();
			DECLARE_MESSAGE_MAP()
		};

	}   // namespace
}   // namespace
