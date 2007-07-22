///\file
///\brief interface file for psycle::host::CKeyConfigDlg.
#pragma once
#include "InputHandler.hpp"
#include "resources/resources.hpp"
#include <afxwin.h>
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		/// key config window.
		class CKeyConfigDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(CKeyConfigDlg)
		public:
			void FillCmdList();
			void UpdateCmdList();
			int FindCmdonList(const char* cmdname);
			CKeyConfigDlg();
			std::string _skinPathBuf;
			//removed by alk
			//CButton m_move_cursor_paste;
		// Dialog Data
			//{{AFX_DATA(CKeyConfigDlg)
			enum { IDD = IDD_KEYCONFIG };
			CSpinButtonCtrl	m_autosave_spin;
			CEdit	m_autosave_mins;
			CButton	m_autosave;
			CButton	m_save_reminders;
			CButton	m_tweak_smooth;
			CButton	m_record_unarmed;			
			CButton m_navigation_ignores_step;
			CButton	m_show_info;
			CButton	m_cmdShiftArrows;
			CButton	m_cmdFT2Del;
			CButton	m_cmdNewHomeBehaviour;
			CButton	m_cmdCtrlPlay;
			CListBox	m_lstCmds;
			CHotKeyCtrl	m_hotkey0;
			CButton	m_wrap;
			CButton	m_centercursor;
			CButton	m_cursordown;
			CEdit	m_numlines;
			CSpinButtonCtrl	m_spinlines;
			CStatic m_textlines;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CKeyConfigDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			BOOL bInit;
			long m_prvIdx;
			void UpdateHotKey();	
			void SaveHotKey(long idx,WORD key,WORD mods);
			void FindKey(long idx,WORD &key,WORD &mods);
			CmdDef FindCmd(long idx);
			virtual BOOL OnInitDialog();
			afx_msg void OnSelchangeCmdlist();
			virtual void OnCancel();
			virtual void OnOK();
			afx_msg void OnImportreg();
			afx_msg void OnExportreg();
			afx_msg void OnDefaults();
			afx_msg void OnBnClickedSpecialKeys();
			afx_msg void OnNone();
			afx_msg void OnUpdateNumLines();
			DECLARE_MESSAGE_MAP()
		public:
			// number of beats to show a full row in the pattern editor.
			CComboBox m_timesig;
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
