///\file
///\brief interface file for psycle::host::CWaveOutDialog.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include <psycle/audiodrivers/microsoftmmewaveout.h>

namespace psycle { namespace host {

		/// mme config window.
		class CWaveOutDialog : public CDialog
		{
		public:
			CWaveOutDialog(CWnd * pParent = 0);
		// Dialog Data
			//{{AFX_DATA(CWaveOutDialog)
			enum { IDD = IDD_WAVEOUTCONFIG };
			CStatic	m_Latency;
			CSpinButtonCtrl	m_BufSizeSpin;
			CSpinButtonCtrl	m_BufNumSpin;
			CEdit	m_BufSizeEdit;
			CEdit	m_BufNumEdit;
			CButton	m_DitherCheck;
			CComboBox	m_DeviceList;
			CComboBox	m_SampleRateBox;
			int		m_BufNum;
			int		m_BufSize;
			int		m_Device;
			BOOL	m_Dither;
			//}}AFX_DATA
			int m_SampleRate;
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWaveOutDialog)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CWaveOutDialog)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			afx_msg void OnChangeConfigBufnum();
			afx_msg void OnChangeConfigBufsize();
			afx_msg void OnSelendokConfigSamplerate();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		private:
			void RecalcLatency();
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
		class MMEUi : public audiodrivers::MMEUiInterface
		{
		public:
			MMEUi() {}
			~MMEUi() {}

			/*override*/ int DoModal() {
				return dlg_.DoModal();
			}

			/*override*/ void SetValues(
				int device_idx, bool dither,
				int sample_rate, int buffer_size, int buffer_count);

			/*override*/ void GetValues(
				int & device_idx, bool & dither,
				int & sample_rate, int & buffer_size, int & buffer_count);
				
			/*override*/ void WriteConfig(
				int device_idx, bool dither,
				int sample_rate, int buffer_size, int buffer_count);

			/*override*/ void ReadConfig(
				int & device_idx, bool & dither,
				int & sample_rate, int & buffer_size, int & buffer_count);

			/*override*/ void Error(std::string const & msg);
		protected:
			CWaveOutDialog dlg_;
		};


	}   // namespace
}   // namespace
