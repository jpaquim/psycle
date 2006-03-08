#if !defined(XM_SAMPLER_UI)
#define XM_SAMPLER_UI
/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearTracker.h : header file
//

#include "constants.h"
#include "XMSamplerUIGeneral.h"
#include "XMSamplerUIInst.h"
#include "XMSamplerUISample.h"

namespace SF {
/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog
class XMSampler;
class XMSamplerUI : public CPropertySheetImpl<XMSamplerUI>
{
// Construction
public:
XMSamplerUI( CPsycleWTLView* const parent,XMSampler * const pMachine,ATL::_U_STRINGorID title = (LPCTSTR) NULL, 
                        UINT uStartPage = 0, HWND hWndParent = NULL );

void OnCancel();
enum { IDD = IDD_XM_SAMPLER };

void pMachine(XMSampler *p){_pMachine = p;};
XMSampler * const pMachine(){return _pMachine;};

private:

//	XMSamplerUI();   // standard constructor
//	XMSamplerUI(CPsycleWTLView* parent) {m_pParent = parent;}
	XMSampler* _pMachine;
	SF::XMSamplerUIGeneral m_General;
	SF::XMSamplerUIInst m_Instrument;
	SF::XMSamplerUISample m_Sample;

//	BOOL Create();
	
// Implementation
protected:
	CPsycleWTLView* m_pParent;

public:

	// Generated message map functions
    BEGIN_MSG_MAP_EX(XMSamplerUI)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE,OnClose)
		COMMAND_HANDLER_EX(IDOK,BN_CLICKED,OnOk);
		COMMAND_HANDLER_EX(IDCANCEL,BN_CLICKED,OnCancelEvent);
		CHAIN_MSG_MAP(CPropertySheetImpl<XMSamplerUI>)
    END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	virtual void OnFinalMessage(HWND hWnd);

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void OnOk(UINT wID,int commandId,HWND hwnd)
	{
		OnCancel();
	};

	void OnCancelEvent(UINT wID,int commandId,HWND hwnd)
	{
		OnCancel();
	};

};

}
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARTRACKER_H__B1BFFCE0_0D6E_11D4_8913_98C1EA960D7C__INCLUDED_)
