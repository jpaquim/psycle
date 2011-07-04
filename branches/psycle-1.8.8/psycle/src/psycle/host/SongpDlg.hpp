///\file
///\brief interface file for psycle::host::CSongpDlg.
#pragma once
#include "Psycle.hpp"
namespace psycle {
	namespace host {

		class Song; // forward declaration

		/// song dialog window.
		class CSongpDlg : public CDialog
		{
		public:
			/// mfc compliant constructor.
			CSongpDlg(Song *song, CWnd* pParent = 0);
			Song* _pSong;
		// Dialog Data
			enum { IDD = IDD_SONGPROP };
			CEdit	m_songcomments;
			CEdit	m_songcredits;
			CEdit	m_songtitle;
			bool	readonlystate;
		public:
			void SetReadOnly();
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
		protected:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnOk();
		};

}}
