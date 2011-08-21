#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

#include "Song.hpp"

// CTransformPatternDlg dialog
namespace psycle { namespace host {
		class Song;

		class CTransformPatternDlg : public CDialog
		{
			DECLARE_DYNAMIC(CTransformPatternDlg)

		private:
			Song* pSong;
		public:
			CTransformPatternDlg(Song* _pSong, CWnd* pParent = NULL);   // standard constructor
			virtual ~CTransformPatternDlg();

			enum { IDD = IDD_TRANSFORMPATTERN };			
			CEdit	m_filternote;
			CEdit	m_filterins;
			CEdit	m_filtermac;
			CEdit	m_filtercmd;
			CEdit	m_replacenote;
			CEdit	m_replaceins;
			CEdit	m_replacemac;
			CEdit	m_replacecmd;
			int	m_applyto;
			CButton	m_applytosong;
			CButton	m_applytopattern;
			CButton	m_applytoblock;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
		protected:
		public:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnBnClickedApply();

		};

}   // namespace
}   // namespace
