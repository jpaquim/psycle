/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */

#pragma once

#include "resource.h"       // メイン シンボル
#include <atlhost.h>


// XMSamplerUISample

namespace SF {
class XMSampler;
class XMSamplerUISample : 
	public CPropertyPageImpl<XMSamplerUISample>
{
public:
	XMSamplerUISample()
	{
	}

	~XMSamplerUISample()
	{
	}

	enum { IDD = IDD_XMSAMPLERUISAMPLE };

BEGIN_MSG_MAP(XMSamplerUISample)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	//COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	//COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	CHAIN_MSG_MAP(CPropertyPageImpl<XMSamplerUISample>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);(& )

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 1;  // システムでフォーカスを設定します。
	}

	//LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	//{
	//	EndDialog(wID);
	//	return 0;
	//}

	//LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	//{
	//	EndDialog(wID);
	//	return 0;
	//}
	void pMachine(XMSampler * const p){m_pMachine = p;};
	XMSampler * const pMachine(){return m_pMachine;};

private:
	XMSampler *m_pMachine;

};

}
