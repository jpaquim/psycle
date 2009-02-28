///\file
///\brief interface file for psycle::host::CFrameMachine.
#pragma once
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

class CChildView;
class Machine;

#define K_XSIZE     28
#define K_YSIZE     28
#define K_NUMFRAMES 63
#define W_ROWWIDTH  150

/// parameters window of native machines.
class CFrameMachine : public CFrameWnd
{
	DECLARE_DYNCREATE(CFrameMachine)
protected:
	CFrameMachine(); // protected constructor used by dynamic creation
	virtual ~CFrameMachine();
public:
	//Legacy Constructor	
	CFrameMachine(class MachineGui* gen_gui);
	void Init(int x, int y);		
// Operations
public:
	virtual void SelectMachine(Machine* pMachine);
	virtual void Generate(double x, double y);
	virtual int ConvertXYtoParam(int x, int y);
	virtual void centerWindowOnPoint(int x, int y);
// Overrides
protected:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnParametersRandomparameters();
	afx_msg void OnParametersResetparameters();
	afx_msg void OnParametersCommand();
	afx_msg void OnMachineAboutthismachine();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnParametersShowpreset();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_MESSAGE_MAP()

	// Attributes
protected:
	Machine* _pMachine;
	MachineGui* gen_gui_;
	CChildView *wndView;
	bool* _pActive;	// It is used to help the program know if this window is open or not.
	//CBitmap b_knob;
	CFont	b_font;
	CFont	b_font_bold;

	// Environment
	int numParameters;
	UINT MachineIndex;

	bool istweak;
	float visualtweakvalue;
	bool finetweak;
	bool ultrafinetweak;
	int tweakpar;
	int tweakbase;
	int minval;
	int maxval;
	int sourcepoint;
	int prevval;

	int ncol;
	int parspercol;


};

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
