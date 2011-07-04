#if !defined(AFX_FRAMEMACHINE_H__D8C72DE0_5378_11D4_8913_F64D9CA20B48__INCLUDED_)
#define AFX_FRAMEMACHINE_H__D8C72DE0_5378_11D4_8913_F64D9CA20B48__INCLUDED_

/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FrameMachine.h : header file
//

#include "Machine.h"
#include "IVstGui.h"
//#include "PsycleWTLView.h"	// Added by ClassView

class Song;
class CPsycleWTLView;

/////////////////////////////////////////////////////////////////////////////
// CFrameMachine frame

class CFrameMachine : public CFrameWindowImpl<CFrameMachine,IVstGui>
{
public:
	DECLARE_FRAME_WND_CLASS(_T("MachineFrame"), IDR_MACHINEFRAME)
	CFrameMachine(int dum){MachineIndex = dum;};
	CFrameMachine();           // protected constructor used by dynamic creation
	virtual ~CFrameMachine();
	virtual void OnFinalMessage(HWND hWnd);
	void SelectMachine(Machine* const pMachine);
	void Generate(const HWND hwndParent,const bool bChild = false);

private:
	Machine* _pMachine;
	CBitmap b_knob;
	CFont	b_font;
// Enviroment

	int numParameters;
	bool me;

	bool istweak;
	bool finetweak;
	bool ultrafinetweak;
	int tweakpar;
	int tweakbase;
	int sourcepoint;
	
	int ncol;
	int parspercol;
// Operations
public:
	CPsycleWTLView *wndView;
	UINT MachineIndex;

	bool* _pActive;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrameMachine)
	//}}AFX_VIRTUAL

// Implementation
// Message Handler
public:
	typedef CFrameWindowImpl<CFrameMachine , IVstGui> ThisClass;

	BEGIN_MSG_MAP(CFrameMachine)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		COMMAND_ID_HANDLER(ID_PARAMETERS_RESETPARAMETERS, OnParametersResetparameters)
		COMMAND_ID_HANDLER(ID_PARAMETERS_RANDOMPARAMETERS, OnParametersRandomparameters)
		COMMAND_ID_HANDLER(ID_PARAMETERS_SHOWPRESET, OnParametersShowpreset)
		COMMAND_ID_HANDLER(ID_MACHINE_ABOUTTHISMACHINE, OnMachineAboutthismachine)
		COMMAND_ID_HANDLER(ID_MACHINE_ABOUTPSYGEARV10, OnMachineAboutpsygearv10)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_MACHINE_COMMAND, OnMachineCommand)
		CHAIN_MSG_MAP(ThisClass)
	END_MSG_MAP()
private:
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnParametersResetparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnParametersRandomparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnParametersShowpreset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineAboutthismachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineAboutpsygearv10(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMachineCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
public:
//	virtual void OnFinalMessage(HWND hWnd);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRAMEMACHINE_H__D8C72DE0_5378_11D4_8913_F64D9CA20B48__INCLUDED_)
