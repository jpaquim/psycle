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
					s << std::setprecision(2) << i << ": " << dstMac.GetInputPinName(i) << " ";
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
			for (int s=0;s< srcpins; s++) {
				checked[s].resize(dstpins);
			}
				
			const Wire::Mapping &mapping = m_wire.GetMapping();
			for (int i(0);i<mapping.size();i++) {
				const Wire::PinConnection & con = mapping[i];
				assert(con.first < srcpins);
				assert(con.second < dstpins);
				checked[con.first][con.second] = true;
			}
			for (int s=0;s< srcpins; s++) {
				for (int d=0;d< dstpins; d++) {
					AddButton(s,d,dstpins, checked[s][d]);
				}
			}
			// 7 -> the dialog margin set in the left/right resource editor
			// 7 -> the dialog margin set in the top/bottom
			// 67 -> 60 (left text) + 7 margin right. 
			// 72 -> 30 (top text) + 14 (button height) * 2 + 7 (margin bottom) + 7 (space from check to button)
			CRect rect(7,7,67 + dstpins*40, 72 + srcpins*9);
			MapDialogRect(&rect);
			CRect rect2;
			GetDlgItem(IDOK)->GetWindowRect(&rect2);
			GetDlgItem(IDOK)->SetWindowPos(NULL,
				rect.right-(rect2.right-rect2.left) - rect.left,
				rect.bottom-((rect2.bottom-rect2.top)*2)- rect.top,0,0,SWP_NOZORDER | SWP_NOSIZE);
			GetDlgItem(IDCANCEL)->SetWindowPos(NULL,
				rect.right-(rect2.right-rect2.left) - rect.left,
				rect.bottom-((rect2.bottom-rect2.top)) - rect.top,0,0,SWP_NOZORDER | SWP_NOSIZE);
			rect.bottom += ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYFIXEDFRAME);
			rect.right += 2 * ::GetSystemMetrics(SM_CXFIXEDFRAME);
			SetWindowPos(NULL,0,0,rect.right,rect.bottom, SWP_NOZORDER | SWP_NOMOVE);
			return FALSE;
		}
		void CChannelMappingDlg::AddButton(int yRel, int xRel, int numins, bool checked)
		{
			// 60 -> left text
			// 30 -> top text
			int x = 60 + xRel*40;
			int y = 30 + yRel*9;
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
			for (int s=0;s< srcpins; s++) {
				for (int d=0;d< dstpins; d++, count++) {
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
