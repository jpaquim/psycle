///\file
///\brief implementation file for psycle::host::CWavFileDlg.
#include <psycle/project.private.hpp>
#include "WavFileDlg.hpp"
#include "Psycle.hpp"
#include "Song.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
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
			//{{AFX_MSG_MAP(CWavFileDlg)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()


		void CWavFileDlg::OnFileNameChange()
		{
			CString CurrExt=GetFileExt();
			
			CurrExt.MakeLower();
			_pSong->wavprev.SetInstrument(_pSong->_pInstrument[PREV_WAV_INS]);
			
			_pSong->wavprev.Stop();
		/*	if (_pSong->PW_Stage)
			{
				_pSong->PW_Stage=0;
				Sleep(LOCK_LATENCY);
			}
		*/	
			
			if (CurrExt=="wav" && _lastFile != GetFileName())
			{
				_lastFile=GetFileName();
				
				if (_pSong->WavAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
//					_pSong->PW_Play();
					_pSong->wavprev.Play();
					
				}
			}
			else if (CurrExt=="iff" && _lastFile != GetFileName())
			{
				_lastFile=GetFileName();
				
				if (_pSong->IffAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
					//_pSong->PW_Play();
					_pSong->wavprev.Play();
				}
			}

			CFileDialog::OnFileNameChange();
		}
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
