///\file
///\brief interface file for psycle::host::CASIOConfig.
#pragma once
#include "Psycle.hpp"

namespace psycle { namespace host {

		class ASIOInterface;

		/// asio config window.
		class CASIOConfig : public CDialog
		{
		public:
			CASIOConfig(CWnd* pParent = 0);
			enum { IDD = IDD_ASIO_CONFIG };

			CComboBox	m_driverComboBox;
			CStatic		m_latency;
			CComboBox	m_sampleRateCombo;
			CComboBox	m_bufferSizeCombo;
			int	m_driverIndex;
			int m_sampleRate;
			int	m_bufferSize;
			ASIOInterface* pASIO;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		protected:
			void RecalcLatency();
			void FillBufferBox();
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			afx_msg void OnSelendokSamplerate();
			afx_msg void OnSelendokBuffersize();
			afx_msg void OnDestroy();
			afx_msg void OnExclusive();
			afx_msg void OnControlPanel();
			afx_msg void OnSelchangeAsioDriver();
			afx_msg void OnBnClickedOk();
			DECLARE_MESSAGE_MAP()
		};
}}
