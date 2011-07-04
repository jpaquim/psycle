#if !defined(AFX_VSTGUI1_H__2C9E2D2B_5D69_11D4_A87E_00C026101D9A__INCLUDED_)
#define AFX_VSTGUI1_H__2C9E2D2B_5D69_11D4_A87E_00C026101D9A__INCLUDED_
/** @file 
 *  @brief header file
 *  $Date: 2004/09/19 10:19:38 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVstGui frame
#include "Vst\Aeffect.h"
#include "IVstGui.h"
//#include "PsycleWTLView.h"

class CVstGui : public CWindowImpl<CVstGui,IVstGui>
{
	
// Operations
public:
	DECLARE_WND_CLASS(NULL)
	CVstGui(int n){};
	CVstGui();
	virtual ~CVstGui();

	void DoTheInit();

	
// Attribute
	AEffect* const pEffect(void){return m_pEffect;};
	void pEffect(AEffect * const _pEffect){m_pEffect = _pEffect;};
	
	Machine* pMachine(void){ return m_pMachine;};
	void pMachine(Machine* _pMachine){m_pMachine = _pMachine;};
	
// Message Dispatch
	BEGIN_MSG_MAP(CVstGui)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ENTERIDLE, OnEnterIdle)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	END_MSG_MAP()
private:
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEnterIdle(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	AEffect* m_pEffect;
	Machine* m_pMachine;

public:
	virtual void OnFinalMessage(HWND hWnd);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSTGUI1_H__2C9E2D2B_5D69_11D4_A87E_00C026101D9A__INCLUDED_)
