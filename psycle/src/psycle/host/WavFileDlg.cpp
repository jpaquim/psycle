///\file
///\brief implementation file for psycle::host::CWavFileDlg.
#include <psycle/host/detail/project.private.hpp>
#include "WavFileDlg.hpp"
#include "Song.hpp"
namespace psycle { namespace host {

IMPLEMENT_DYNAMIC(CWavFileDlg, CFileDialog)

		CWavFileDlg::CWavFileDlg(
								BOOL bOpenFileDialog,
								LPCTSTR lpszDefExt,
								LPCTSTR lpszFileName,
								DWORD dwFlags,
								LPCTSTR lpszFilter,
								CWnd* pParentWnd) 
								: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
		{
			_lastFile='\0';
		}


		BEGIN_MESSAGE_MAP(CWavFileDlg, CFileDialog)
			ON_WM_CLOSE()
		END_MESSAGE_MAP()


		void CWavFileDlg::OnFileNameChange()
		{
			CString CurrExt=GetFileExt();
			
			CurrExt.MakeLower();

			m_pSong->wavprev.Stop(true);
			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			if (CurrExt=="wav" && _lastFile != GetPathName())
			{
				_lastFile=GetPathName();
				
				if (m_pSong->WavAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
					m_pSong->wavprev.Play();
					
				}
			}
			else if (CurrExt=="iff" && _lastFile != GetPathName())
			{
				_lastFile=GetPathName();
				
				if (m_pSong->IffAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
					m_pSong->wavprev.Play();
				}
			}

			CFileDialog::OnFileNameChange();
		}
		void CWavFileDlg::OnClose()
		{
			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			m_pSong->wavprev.Stop(true);
			CFileDialog::OnClose();
		}
	}   // namespace
}   // namespace
