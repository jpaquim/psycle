///\file
///\brief implementation file for psycle::host::CVolumeDlg.
#include <psycle/host/detail/project.private.hpp>
#include "ChannelMappingDlg.hpp"
#include <psycle/host/Machine.hpp>

#include <string>
#include <sstream>

namespace psycle { namespace host {
		CChannelMappingDlg::CChannelMappingDlg(Wire& wire, CWnd* pParent)
			: CDialog(CChannelMappingDlg::IDD, pParent)
			, m_wire(wire)
		{
		}

		void CChannelMappingDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_WIRE_OUTPUT_NAMES, m_dstnames);
			DDX_Control(pDX, IDC_WIRE_INPUT_NAMES, m_srcnames);
		}

		BEGIN_MESSAGE_MAP(CChannelMappingDlg, CDialog)
			ON_COMMAND_RANGE(IDC_CHK_CHANMAP_0, IDC_CHK_CHANMAP_0+255, OnCheckChanMap)
		END_MESSAGE_MAP()

		BOOL CChannelMappingDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			const Machine & dstMac = m_wire.GetDstMachine();
			const Machine & srcMac = m_wire.GetSrcMachine();
			int srcpins=std::min(srcMac.GetNumOutputPins(),16);
			int dstpins =std::min(dstMac.GetNumInputPins(),16);
			{
				std::stringstream s;
				for (int i=0;i< dstpins; i++) {
					s << std::setprecision(2) << i << ": " << dstMac.GetInputPinName(i) << std::endl;
				}
				m_dstnames.SetWindowText(s.str().c_str());
			}
			{
				std::stringstream s;
				for (int i=0;i< srcpins; i++) {
					s << std::setprecision(2) << i << ": " << srcMac.GetOutputPinName(i) << std::endl;
				}
				m_srcnames.SetWindowText(s.str().c_str());
			}

			std::vector<std::vector<bool>> checked;
			checked.resize(srcpins);
			for (int d=0;d< srcpins; d++) {
				checked[d].resize(dstpins);
			}
				
			const Wire::Mapping &mapping = m_wire.GetMapping();
			for (int i(0);i<mapping.size();i++) {
				const Wire::PinConnection & con = mapping[i];
				assert(con.first < srcpins);
				assert(con.second < dstpins);
				checked[con.first][con.second] = true;
			}
			for (int d=0;d< dstpins; d++) {
				for (int s=0;s< srcpins; s++) {
					AddButton(s,d,srcpins, checked[s][d]);
				}
			}
			return FALSE;
		}
		void CChannelMappingDlg::AddButton(int xRel, int yRel, int numins, bool checked)
		{
			int x = 60 + xRel*10;
			int y = 20 + yRel*10;
			CButton* m_button = new CButton();
			CRect rect(x,y,x+10,y+8);
			MapDialogRect(&rect);
			
			m_button->Create(_T(""),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_CHECKBOX,
				rect,this,IDC_CHK_CHANMAP_0+((yRel*numins)+xRel));
			m_button->ShowWindow(SW_SHOW);
			
			m_button->SetCheck(checked);
			buttons.push_back(m_button);
		}

		void CChannelMappingDlg::OnOK() 
		{
			Wire::Mapping mapping;
			int srcpins = std::min(m_wire.GetSrcMachine().GetNumOutputPins(),16);
			int dstpins = std::min(m_wire.GetDstMachine().GetNumInputPins(),16);
			int count=0;
			for (int d=0;d< dstpins; d++) {
				for (int s=0;s< srcpins; s++, count++) {
					if (buttons[count]->GetCheck()) {
						mapping.push_back(Wire::PinConnection(s,d));
					}
				}
			}
			m_wire.ChangeMapping(mapping);

			for(int i=0; i < buttons.size();i++) {
				delete buttons[i];
			}

			CDialog::OnOK();
		}

		void CChannelMappingDlg::OnCancel() 
		{
			for(int i=0; i < buttons.size();i++) {
				delete buttons[i];
			}
			CDialog::OnCancel();
		}

		void CChannelMappingDlg::OnCheckChanMap(UINT index) 
		{
			int i = index-IDC_CHK_CHANMAP_0;
			buttons[i]->SetCheck(!buttons[i]->GetCheck());
		}

	}   // namespace
}   // namespace
