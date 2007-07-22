///\file
///\brief interface file for psycle::host::CMacProp.
#pragma once
#include <psycle/engine/Machine.hpp>
#include <psycle/host/ChildView.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		class Song;

		/// machine properties window.
		class CMacProp : public CDialog
		{
		public:
			CChildView *m_view;
			CMacProp(CWnd* pParent = 0);
			Machine *pMachine;
			Song* pSong;
			int thisMac;
			std::string txt;
			bool deleted;
			enum { IDD = IDD_MACPROP };
			CButton	m_soloCheck;
			CButton	m_bypassCheck;
			CButton	m_muteCheck;
			CEdit	m_macname;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);
			virtual BOOL OnInitDialog();
			afx_msg void OnChangeEdit1();
			afx_msg void OnButton1();
			afx_msg void OnMute();
			afx_msg void OnBypass();
			afx_msg void OnSolo();
			afx_msg void OnClone();
		DECLARE_MESSAGE_MAP()
		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
