// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psycle::host::CASIOConfig.
#pragma once
#include "ASIOInterface.hpp"
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		/// asio config window.
		class CASIOConfig : public CDialog
		{
		public:
			int m_sampleRate;
			int	m_bufferSize;
			CASIOConfig(CWnd* pParent = 0);
			ASIOInterface* pASIO;
		// Dialog Data
			//{{AFX_DATA(CASIOConfig)
			enum { IDD = IDD_ASIO_CONFIG };
			CComboBox	m_driverComboBox;
			CStatic	m_latency;
			CComboBox	m_sampleRateCombo;
			CComboBox	m_bufferSizeCombo;
			int		m_driverIndex;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CASIOConfig)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			void RecalcLatency();
			void FillBufferBox();
			// Generated message map functions
			//{{AFX_MSG(CASIOConfig)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			afx_msg void OnSelendokSamplerate();
			afx_msg void OnSelendokBuffersize();
			afx_msg void OnDestroy();
			afx_msg void OnExclusive();
			afx_msg void OnControlPanel();
			afx_msg void OnSelchangeAsioDriver();
			afx_msg void OnBnClickedOk();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
