///\file
///\brief interface file for psycle::host::CWaveEdInsertSilenceDialog.
#pragma once
#include "resources/resources.hpp"
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		class CWaveEdInsertSilenceDialog : public CDialog
		{
		// Construction
		public:
			CWaveEdInsertSilenceDialog(CWnd* pParent = 0);
		// Dialog Data
			//{{AFX_DATA(CWaveEdInsertSilenceDialog)
			enum { IDD = IDD_WAVED_INSERTSILENCE };
			CEdit	m_time;
			CButton m_atStart;
			CButton m_atEnd;
			CButton m_atCursor;
			//}}AFX_DATA
		public:
			enum insertPosition
			{
				at_start=0,
				at_end,
				at_cursor
			};
			float timeInSecs;
			insertPosition insertPos;
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWaveEdInsertSilenceDialog)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CWaveEdInsertSilenceDialog)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
