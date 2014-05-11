///\file
///\brief implementation file for psycle::host::CWavFileDlg.
#include <psycle/host/detail/project.private.hpp>
#include "WavFileDlg.hpp"
#include "Song.hpp"
#include "ITModule2.h"
#include <psycle/helpers/filetypedetector.hpp>

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
			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			m_pSong->wavprev.Stop();

			if (_lastFile != GetPathName() && !GetPathName().IsEmpty())
			{
				uint32_t* filterCodes = reinterpret_cast<uint32_t*>(GetOFN().lCustData);
				uint32_t selCode;
				bool play=false;

				if (GetOFN().nFilterIndex == 1) {
					helpers::FormatDetector detect;
					selCode = detect.AutoDetect(GetPathName().GetString());
				}
				else selCode = filterCodes[GetOFN().nFilterIndex-2];

				try {
					if (selCode == helpers::FormatDetector::WAVE_ID) {
						play = (m_pSong->WavAlloc(PREV_WAV_INS, GetPathName().GetString()));
					}
					else if (selCode == helpers::FormatDetector::SCRS_ID) {
						ITModule2 itsong;
						itsong.Open(GetPathName().GetString());
						XMInstrument dummy;
						play = itsong.LoadS3MInstX(*m_pSong, dummy, PREV_WAV_INS);
						itsong.Close();
					}
					else if (selCode == helpers::FormatDetector::IMPS_ID) {
						XMInstrument::WaveData<>& wave = m_pSong->wavprev.UsePreviewWave();
						ITModule2 itsong;
						itsong.Open(GetPathName().GetString());
						play = itsong.LoadITSample(wave);
						itsong.Close();
					}
					else if (selCode == helpers::FormatDetector::AIFF_ID) {
						play = (m_pSong->AIffAlloc(PREV_WAV_INS, GetPathName().GetString()));
					}
					else if (selCode == helpers::FormatDetector::SVX8_ID) {
						play = (m_pSong->IffAlloc(PREV_WAV_INS, GetPathName().GetString()));
					}
				}
				catch(const std::runtime_error & /*e*/) {
					//Would like to show the message, but sometimes, the dialog becomes frozen, so i only do it on load
					//std::ostringstream os; os <<"Could not finish the operation: " << e.what();
					//MessageBox(os.str().c_str(),"Load Error",MB_ICONERROR);
				}
				if (play) { 
					_lastFile=GetPathName(); 
					m_pSong->wavprev.Play();
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
