///\file
///\brief implementation file for psycle::host::CWavFileDlg.
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
		END_MESSAGE_MAP()


		void CWavFileDlg::OnFileNameChange()
		{
			CString CurrExt=GetFileExt();
			
			CurrExt.MakeLower();
			_pSong->wavprev.SetInstrument(_pSong->_pInstrument[PREV_WAV_INS]);
			
			_pSong->wavprev.Stop();
			CExclusiveLock lock(&_pSong->semaphore, 2, true);
			if (CurrExt=="wav" && _lastFile != GetPathName())
			{
				_lastFile=GetPathName();
				
				if (_pSong->WavAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
					_pSong->wavprev.Play();
					
				}
			}
			else if (CurrExt=="iff" && _lastFile != GetPathName())
			{
				_lastFile=GetPathName();
				
				if (_pSong->IffAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
					_pSong->wavprev.Play();
				}
			}

			CFileDialog::OnFileNameChange();
		}
	}   // namespace
}   // namespace
