///\file
///\brief interface file for psycle::host::CPresetsDlg.
#pragma once
#include "Psycle.hpp"
#include "Machine.hpp"
#include <cstring>

namespace psycle {
namespace host {

		#define MAX_PRESETS 256

		class CFrameMachine;
		class Machine;

		/// machine parameter preset window.
		class CPresetsDlg : public CDialog
		{
		public:
			void AddPreset(CPreset& preset);
			void AddPreset(const char* name ,float* parameters);
			void AddPreset(const char* name,int* parameters, byte* newdata);
			Machine* _pMachine;
			CPresetsDlg(CWnd* pParent = 0);
		// Dialog Data
			//{{AFX_DATA(CPresetsDlg)
			enum { IDD = IDD_PRESETS };
			CButton	m_exportButton;
			CButton	m_preview;
			CComboBox	m_preslist;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CPresetsDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			int numParameters;
			long int dataSizeStruct;
			int fileversion;
			void TweakMachine(CPreset &preset);
			void SavePresets();
			void ReadPresets();
			void UpdateList();
			CPreset iniPreset;
			CPreset presets[MAX_PRESETS];
			CString fileName;
			bool presetChanged;
			// Generated message map functions
			//{{AFX_MSG(CPresetsDlg)
			afx_msg void OnSave();
			afx_msg void OnDelete();
			afx_msg void OnImport();
			afx_msg void OnSelchangePresetslist();
			afx_msg void OnDblclkPresetslist();
			virtual void OnOK();
			virtual void OnCancel();
			virtual BOOL OnInitDialog();
			afx_msg void OnExport();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	}   // namespace host
}   // namespace psycle
