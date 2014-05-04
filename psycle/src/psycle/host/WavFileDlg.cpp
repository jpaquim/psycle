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

			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			m_pSong->wavprev.Stop();
			
			if (_lastFile != GetPathName() && !GetPathName().IsEmpty())
			{
				if (CurrExt=="wav" || GetOFN().nFilterIndex == 1 )
				{
					_lastFile=GetPathName();
					try {
						if (m_pSong->WavAlloc(PREV_WAV_INS, _lastFile.GetString()) == 1)
						{
							m_pSong->wavprev.Play();
							
						}
						else {
							//Would like to show the message, but sometimes, the dialog becomes frozen, so i only do it on load
							//MessageBox("Could not load the file, unrecognized format","Load Error",MB_ICONERROR);
						}
					}
					catch(const std::runtime_error & e) {
						//Would like to show the message, but sometimes, the dialog becomes frozen, so i only do it on load
						//std::ostringstream os;
						//os <<"Could not finish the operation: " << e.what();
						//MessageBox(os.str().c_str(),"Load Error",MB_ICONERROR);
					}
				}
				else if (CurrExt=="iff" || GetOFN().nFilterIndex == 2 )
				{
					_lastFile=GetPathName();
					try {

						if (m_pSong->IffAlloc(PREV_WAV_INS, _lastFile.GetString()) == 1)
						{
							m_pSong->wavprev.Play();
						}
						else {
							//Would like to show the message, but sometimes, the dialog becomes frozen, so i only do it on load
							//MessageBox("Could not load the file, unrecognized format","Load Error",MB_ICONERROR);
						}
					}
					catch(const std::runtime_error & e) {
						//Would like to show the message, but sometimes, the dialog becomes frozen, so i only do it on load
						//std::ostringstream os;
						//os <<"Could not finish the operation: " << e.what();
						//MessageBox(os.str().c_str(),"Load Error",MB_ICONERROR);
					}
				}
			}
			CFileDialog::OnFileNameChange();
		}
		void CWavFileDlg::OnClose()
		{
			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			m_pSong->wavprev.Stop();
			CFileDialog::OnClose();
		}
	}   // namespace
}   // namespace
