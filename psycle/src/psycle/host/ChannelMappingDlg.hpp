///\file
///\brief interface file for psycle::host::CVolumeDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {
		/// volume window.
		class Wire;
		class CChannelMappingDlg : public CDialog
		{
		public:
			CChannelMappingDlg(Wire& wire, CWnd* mainView_, CWnd* pParent = 0);
			enum { IDD = IDD_WIRE_CHANMAP };
		protected:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();
		protected:
			void AddButton(int xRel, int yRel, int numins, bool checked);
			DECLARE_MESSAGE_MAP()
			afx_msg void OnCheckChanMap(UINT index);

			Wire &         m_wire;
			CStatic			m_srcnames;
			CStatic			m_dstnames;
			std::vector<CButton*> buttons;
			CWnd*        mainView;
		};

	}   // namespace
}   // namespace
