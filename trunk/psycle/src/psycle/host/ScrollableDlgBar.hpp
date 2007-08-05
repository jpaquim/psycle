///\file
///\brief interface file for psycle::host::CScrollableDlgBar.
#pragma once
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		/// this class is a specialization of CDialogBar with one very small but extremely important modifation:  it handles WM_HSCROLL and WM_VSCROLL
		/// messages properly, to allow the normal use of sliders and scrollbars.
		class CScrollableDlgBar : public CDialogBar
		{
		public:
			CScrollableDlgBar() {}
			virtual ~CScrollableDlgBar() {}

			virtual LRESULT WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
		};

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
