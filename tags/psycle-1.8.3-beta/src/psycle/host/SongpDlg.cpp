///\file
///\brief implementation file for psycle::host::CSongpDlg.
#include <project.private.hpp>
#include "Psycle.hpp"
#include "Song.hpp"
#include "SongpDlg.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		CSongpDlg::CSongpDlg(Song *song, CWnd* pParent /* = 0 */) : CDialog(CSongpDlg::IDD, pParent)
		,readonlystate(false)
		,_pSong(song)
		{
		}

		void CSongpDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_EDIT4, m_songcomments);
			DDX_Control(pDX, IDC_EDIT3, m_songcredits);
			DDX_Control(pDX, IDC_EDIT1, m_songtitle);
		}

		BEGIN_MESSAGE_MAP(CSongpDlg, CDialog)
			ON_BN_CLICKED(IDOK, OnOk)
		END_MESSAGE_MAP()

		BOOL CSongpDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			m_songtitle.SetLimitText(128);
			m_songcredits.SetLimitText(64);
			m_songcomments.SetLimitText(65535);
			m_songtitle.SetWindowText(_pSong->name.c_str());
			m_songcredits.SetWindowText(_pSong->author.c_str());
			m_songcomments.SetWindowText(_pSong->comments.c_str());
			m_songtitle.SetFocus();
			m_songtitle.SetSel(0,-1);

			if ( readonlystate )
			{
				m_songtitle.SetReadOnly();
				m_songcredits.SetReadOnly();
				m_songcomments.SetReadOnly();
				((CButton*)GetDlgItem(IDCANCEL))->ShowWindow(SW_HIDE);
				((CButton*)GetDlgItem(IDOK))->SetWindowText("Close");
				///\todo: find a way to set the "close" as the default button in this case.
			}

			return FALSE;
		}
		void CSongpDlg::SetReadOnly()
		{
			readonlystate=true;
		}

		void CSongpDlg::OnOk() 
		{
			if (!readonlystate)
			{
				char name[129]; char author[65]; char comments[65536];
				m_songtitle.GetWindowText(name,128);
				m_songcredits.GetWindowText(author,64);
				m_songcomments.GetWindowText(comments,65535);
				_pSong->name = name;
				_pSong->author = author;
				_pSong->comments = comments;
				CDialog::OnOK();
			}
			else CDialog::OnCancel();
		}
	NAMESPACE__END
NAMESPACE__END
