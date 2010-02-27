///\file
///\brief interface file for psycle::host::CProgressDialog.
#pragma once
#include "Psycle.hpp"

namespace psycle { namespace host {

		/// progress meter window.
		class CProgressDialog : public CDialog
		{
		public:
			CProgressDialog(CWnd* pParent = 0);

			void SetRange(short low, short high) {
				progress_ctrl_.SetRange(low, high);
			}

			void SetStep(short step) {
				progress_ctrl_.SetStep(step);
			}

			void SetPos(short pos) {
				progress_ctrl_.SetPos(pos);
			}

			void StepIt() {
				progress_ctrl_.StepIt();
			}

		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

			DECLARE_MESSAGE_MAP()

		private:			
			CProgressCtrl progress_ctrl_;

		};

	}   // namespace host
}   // namespace psycle
