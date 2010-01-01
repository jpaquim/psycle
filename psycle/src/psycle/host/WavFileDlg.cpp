///\file
///\brief implementation file for psycle::host::CWavFileDlg.
#include "WavFileDlg.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/sampler.h>
using namespace psycle::core;
#else
#include "Song.hpp"
#include "Sampler.hpp"
#endif
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
			Sampler::wavprev.SetInstrument(_pSong->_pInstrument[PREV_WAV_INS]);
			
			Sampler::wavprev.Stop();
#if !PSYCLE__CONFIGURATION__USE_PSYCORE
			Sleep(LOCK_LATENCY);
#endif
			if (CurrExt=="wav" && _lastFile != GetFileName())
			{
				_lastFile=GetFileName();
				
				if (_pSong->WavAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
					Sampler::wavprev.Play();
					
				}
			}
			else if (CurrExt=="iff" && _lastFile != GetFileName())
			{
				_lastFile=GetFileName();
				
				if (_pSong->IffAlloc(PREV_WAV_INS, _lastFile) == 1)
				{
					Sampler::wavprev.Play();
				}
			}

			CFileDialog::OnFileNameChange();
		}
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
