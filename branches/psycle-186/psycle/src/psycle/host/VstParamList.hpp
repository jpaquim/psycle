///\file
///\brief interface file for psycle::host::CVstParamList.
#pragma once
#include "Psycle.hpp"

namespace psycle {
	namespace host {
		namespace vst
		{
			class plugin;
		}

		class CVstParamList : public CDialog
		{
		public:
			CVstParamList(vst::plugin& effect, CVstParamList** windowVar);   // standard constructor
			virtual ~CVstParamList();

			enum { IDD = IDD_VSTRACK };
			CComboBox	m_program;
			CSliderCtrl	m_slider;
			CStatic		m_text;
			CListBox	m_parlist;
		
		// Attributes
		public:
			vst::plugin& machine;

		protected:
			int _quantizedvalue;
			CWnd* mainView;
			CVstParamList** windowVar;

		// Operations
		public:
			void Init();
			void UpdateOne();
			void UpdateParList();
			void UpdateText(int value);
			void UpdateNew(int par,float value);
			void InitializePrograms(void);
			void SelectProgram(long index);

		// Overrides
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			virtual void PostNcDestroy();
			virtual void OnCancel();
		protected:
		#if !defined NDEBUG
			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;
		#endif
			// Generated message map functions
			afx_msg void OnClose();
			afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnSelchangeList();
			afx_msg void OnSelchangeProgram();
			afx_msg void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);
			DECLARE_MESSAGE_MAP()
		};
	}   // namespace
}   // namespace
