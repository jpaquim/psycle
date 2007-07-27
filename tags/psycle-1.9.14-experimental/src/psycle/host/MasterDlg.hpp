///\file
///\brief interface file for psycle::host::CMasterDlg.
#pragma once
#include <psycle/engine/Machine.hpp>
#include <psycle/engine/internal_machines.hpp>
#include <psycle/engine/constants.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		class CChildView;

		/// master machine window.
		class CMasterDlg : public CDialog
		{
			DECLARE_MESSAGE_MAP()
			public:
				CMasterDlg(CChildView * pParent);
				BOOL Create();
				void PaintNumbers(float val, int x, int y);
				void PaintNumbersDC(CDC* dc,CDC* memDC,float val,int x,int y);
				void PaintNames(char* name,int x,int y);
				void SetSliderValues();
				void UpdateUI(void);
				CBitmap m_numbers;
				CFont namesFont;
				Master* _pMachine;
				std::string macname[MAX_CONNECTIONS];
				afx_msg void OnCancel();
				enum { IDD = IDD_MASTERDLG };
				CStatic	m_masterpeak;
				CSliderCtrl	m_slidermaster;
				CSliderCtrl	m_sliderm[MAX_CONNECTIONS];
				CStatic	m_mixerview;
				CButton	m_autodec;
				virtual BOOL PreTranslateMessage(MSG* pMsg);
			protected:
				virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
				CChildView* m_pParent;
				virtual BOOL OnInitDialog();
				afx_msg void OnAutodec();
				afx_msg void OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult);
				#include <boost/preprocessor/repetition/repeat.hpp>
				#include <boost/preprocessor/cat.hpp>
					#define sad_tools(_, count, __) afx_msg void OnCustomdrawSliderm##count(NMHDR* pNMHDR, LRESULT* pResult);
					BOOST_PP_REPEAT(PSYCLE__MAX_CONNECTIONS, sad_tools, ~)
				#undef sad_tools
				afx_msg void OnPaint();
			private:
				void paint_names_in_grid();
				void OnStnClickedMixerview();
		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
