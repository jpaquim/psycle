/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief interface file for psycle::host::CVstParamList.
#pragma once
#include "resources/resources.hpp"
#include <afxext.h>
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		namespace vst
		{
			class plugin;
		}

		class CVstParamList : public CDialog
		{
		public:
			CVstParamList(vst::plugin* effect);   // standard constructor
			virtual ~CVstParamList();
			virtual BOOL Create(CWnd *pParentWnd=NULL);

			enum { IDD = IDD_VSTRACK };
			CComboBox	m_program;
			CSliderCtrl	m_slider;
			CStatic		m_text;
			CListBox	m_parlist;
		
		// Attributes
		public:
			inline vst::plugin& machine(){ return *_pMachine; }

		protected:
			vst::plugin * _pMachine;
			int _quantizedvalue;
			CWnd* _mainView;

		// Operations
		public:
			void Init();
			void UpdateOne();
			void UpdateParList();
			void UpdateText(int value);
			void UpdateNew(int par,float value);
			void InitializePrograms(void);
		// Overrides
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		protected:
		#if !defined NDEBUG
			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;
		#endif
			// Generated message map functions
			virtual BOOL OnInitDialog();
			afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnSelchangeList();
			afx_msg void OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnSelchangeProgram();
			afx_msg void OnCloseupProgram();
			afx_msg void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);
			DECLARE_MESSAGE_MAP()
		};
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
