#if !defined(AFX_VSTEDITORDLG_H__44F25588_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
#define AFX_VSTEDITORDLG_H__44F25588_ACD9_11D4_937A_BEE48B868538__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/** @file VstEditorDlg.h 
 *  @brief header file
 *  $Date: 2004/09/19 10:19:38 $
 *  $Revision: 1.3 $
 */

#include "VSTHost.h"
#include "DefaultVstGui.h"

class CPsycleWTLView;
/** @class CVstEditorDlg frame
 *  @brief VST Editor Dialog Class */
class CVstEditorDlg : public CFrameWindowImpl<CVstEditorDlg>
{

public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_VSTFRAME)///< FrameWindow Definition \n フレームウィンドウ定義 
	CVstEditorDlg(int n) {};///< Constructor \n コンストラクタ
	virtual ~CVstEditorDlg();//< Destructor \n デストラクタ
	CVstEditorDlg();///< Default Constructor 

// Attributes
public:
	CPsycleWTLView* pView(void) { return pView;};///< view pointer ビューポインタ
	void pView(CPsycleWTLView* _pView){ m_pView = _pView;};///< view pointer ビューポインタ
	
	//void pWindow(CWindow *_pWindow){m_pWindow = _pWindow;};
	//CWindow * pWindow(void) { return pWindow;};

	void MachineIndex(UINT _MachineIndex){ m_MachineIndex = _MachineIndex;};
	UINT MachineIndex(void){ return m_MachineIndex;};

	//CDefaultVstGui * pParamGui(void) { return m_pParamGui;};
	//void pParamGui(CDefaultVstGui *_pParamGui){m_pParamGui = _pParamGui;};

	VSTPlugin* pMachine(void){ return m_pMachine;};
	void pMachine(VSTPlugin* _pMachine){m_pMachine = _pMachine;};

	bool IsEditorActive(void) {
		ATLASSERT(m_pEditorActive != NULL);
		ATLASSERT(*m_pEditorActive == true || *m_pEditorActive == false); 
		return *m_pEditorActive;
	};
	void pEditorActive(bool *_pb){m_pEditorActive = _pb;};

// Message Handler
	BEGIN_MSG_MAP(CVstEditorDlg)
		COMMAND_ID_HANDLER(ID_PARAMETERS_RESETPARAMETERS, OnParametersResetparameters)
		COMMAND_ID_HANDLER(ID_PARAMETERS_RANDOMPARAMETERS, OnParametersRandomparameters)
		COMMAND_ID_HANDLER(ID_PARAMETERS_SHOWPRESET, OnParametersShowpreset)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		//CHAIN_MSG_MAP(CFrameWindowImpl<CVstEditorDlg>)
	END_MSG_MAP()

// Operations
	void Resize(int w,int h);
	void Refresh(int par,float val);

private:
	LRESULT OnParametersResetparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnParametersRandomparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnParametersShowpreset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	IVstGui *m_pWindow;
	CDefaultVstGui *m_pParamGui;
	VSTPlugin* m_pMachine;
	CPsycleWTLView* m_pView;
	UINT m_MachineIndex;
	bool* m_pEditorActive;
	bool m_bEditorGui;
	bool m_bCreatingWindow;
	WTL::CSplitterWindow m_Splitter;


public:
	virtual void OnFinalMessage(HWND hWnd);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSTEDITORDLG_H__44F25588_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
