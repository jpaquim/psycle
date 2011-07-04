/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "VstEditorDlg.h"
#include "FrameMachine.h"
#include "VstGui.h"
#include "Vst\AEffEditor.h"
#include "PresetsDlg.h"
#include "inputhandler.h"
#include "MainFrm.h"
#include ".\vsteditordlg.h"
		/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
// 日本語版に合わせVSTラックのサイズを変更
// わたなべ＠ノイエ技研 2003/08/08

#define VST_PARAMETRIC_WIDTH 263
//#define VST_PARAMETRIC_WIDTH 228
#define VST_PARAMETRIC_HEIGHT 314
//#define VST_PARAMETRIC_HEIGHT 338
#define VST_PARAM_PANE 1
#define VST_UI_PANE 0

//extern CPsycleApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CVstEditorDlg


CVstEditorDlg::CVstEditorDlg()
{
	m_bEditorGui = false;
	m_bCreatingWindow = false;
	m_pMachine = NULL;
	m_pWindow = NULL;
	m_pParamGui = NULL;

}

CVstEditorDlg::~CVstEditorDlg()
{
	if (m_pEditorActive != NULL) 
	{
		*m_pEditorActive = false;
	}

}



void CVstEditorDlg::Refresh(int par,float value)
{
	if ( par == -1 ) m_pParamGui->UpdateOne();
	else m_pParamGui->UpdateNew(par,value);
	if (!m_bEditorGui) m_pWindow->Invalidate(false);
}

void CVstEditorDlg::Resize(int w,int h)
{
	if (!m_bCreatingWindow)
	{
		int nw, nh;
		nw = w + VST_PARAMETRIC_WIDTH + GetSystemMetrics(SM_CXEDGE)*3;
		nh = h + VST_PARAMETRIC_HEIGHT + 9 + GetSystemMetrics(SM_CYCAPTION) +
			 GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE);

		SetWindowPos(NULL, 0, 0, nw, nh, SWP_NOMOVE | SWP_NOZORDER);

//		m_Splitter.SetColumnInfo(VST_UI_PANE,w,w);
		m_Splitter.SetSplitterPos(w);
	}
}



LRESULT CVstEditorDlg::OnParametersResetparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコマンド ハンドラ コードを追加します。

	return 0;
}

LRESULT CVstEditorDlg::OnParametersRandomparameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for (int c=0; c<((VSTPlugin*)m_pMachine)->NumParameters(); c++)
	{
		float randsem = (float)rand()*0.000030517578125f;

		((VSTPlugin*)m_pMachine)->SetParameter(c, randsem);
	}
	m_pWindow->Invalidate(false);
	return 0;
}

LRESULT CVstEditorDlg::OnParametersShowpreset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CPresetsDlg dlg(m_pMachine,this);
	dlg.DoModal();
	m_pParamGui->UpdateOne();
	if (!m_bEditorGui) m_pWindow->Invalidate(false);
	m_pWindow->SetFocus();
	return 0;
}

LRESULT CVstEditorDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{

//	CFrameWnd::OnClose();
	CMainFrame::GetInstance().m_view.SetFocus();
	DestroyWindow();
	bHandled = FALSE;
	return 0;
}

LRESULT CVstEditorDlg::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//CFrameWnd::OnSetFocus(pOldWnd);
	m_pWindow->SetFocus();
	return 0;
}

LRESULT CVstEditorDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。

	return 0;
}

LRESULT CVstEditorDlg::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	SIZE size;
	int width = 500;
	int height = 200;
	ERect * er;
	int guiWidth = 0;

	m_bCreatingWindow = true;
	const DWORD dwSplitStyle = WS_CHILD | WS_VISIBLE | 
                             WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            dwSplitExStyle = WS_EX_CLIENTEDGE;
	
	m_Splitter.Create( *this, rcDefault, NULL,dwSplitStyle, dwSplitExStyle );
//	m_Splitter.ShowWindow(SW_SHOW);
     
	m_hWndClient = m_Splitter;
	m_Splitter.SetSplitterPos( 200 );
	
//	m_Splitter.CreateStatic(this, 1, 2);

	m_bEditorGui = (m_pMachine->_pEffect->flags & effFlagsHasEditor);

	if ( m_bEditorGui )
	{
		SIZE size={200,100};
		
		m_pWindow = new CVstGui();
	
		((CVstGui*)m_pWindow)->Create(m_hWndClient,0,_T(""),WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_Splitter.SetSplitterPane(0, *m_pWindow);
		
		//m_pWindow->Create(
		//m_Splitter.CreateView(0, VST_UI_PANE, RUNTIME_CLASS(CVstGui), size, pContext);
		//m_pWindow = m_Splitter.GetPane(0, VST_UI_PANE);
		
		((CVstGui*)m_pWindow)->pMachine(m_pMachine);
		((CVstGui*)m_pWindow)->pEffect(m_pMachine->_pEffect);

		m_pMachine->Dispatch(effEditOpen, 0, 0, m_pWindow->m_hWnd, 0.0f);
	
		m_pMachine->Dispatch(effEditGetRect, 0, 0, &er,0.0f);
		guiWidth = width = er->right - er->left;
		height = er->bottom - er->top;
		
		//m_Splitter.SetColumnInfo(VST_UI_PANE,width,width);
		((CVstGui*)m_pWindow)->DoTheInit();
		((CVstGui*)m_pWindow)->ShowWindow(SW_SHOW);
	}
	else
	{
		int numParameters = m_pMachine->NumParameters();
		int ncol = 1;
		while ( (numParameters / ncol) * 28 > ncol * 134) ncol++;
		int parspercol = numParameters / ncol;
		if ( parspercol * ncol < numParameters) parspercol++;
		size.cx = ncol * 134;
		size.cy = parspercol * 28;
		height = size.cy;
		guiWidth = width = size.cx;
		m_pWindow = new CFrameMachine();

		((CFrameMachine*)m_pWindow)->wndView = m_pView;
		((CFrameMachine*)m_pWindow)->MachineIndex = m_MachineIndex;
		((CFrameMachine*)m_pWindow)->Generate(m_hWndClient,true);
		m_Splitter.SetSplitterPane(0, *m_pWindow);
		((CFrameMachine*)m_pWindow)->SelectMachine(pMachine());
		((CFrameMachine*)m_pWindow)->ShowWindow(SW_SHOW);
		((CFrameMachine*)m_pWindow)->_pActive = NULL;
	}
	size.cx = VST_PARAMETRIC_WIDTH;
	size.cy = VST_PARAMETRIC_HEIGHT;
	width += size.cx;
	if (height < size.cy)
	{
		height = size.cy;
	}
	width += GetSystemMetrics(SM_CXEDGE) * 3;
	height += 9 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE);

	//m_Splitter.CreateView(0, VST_PARAM_PANE, RUNTIME_CLASS(CDefaultVstGui), size, pContext);
	m_pParamGui = new CDefaultVstGui();
	m_Splitter.SetSplitterPane(VST_PARAM_PANE,((CDefaultVstGui*)m_pParamGui)->Create(m_hWndClient));
	m_pParamGui->ShowWindow(SW_SHOW);
//	m_pParamGui= (CDefaultVstGui*)m_Splitter.GetPane(0,VST_PARAM_PANE);
	m_pParamGui->pMachine(m_pMachine);
	m_pParamGui->pWindow(m_pWindow);
	m_pParamGui->pView(m_pView);
	m_pParamGui->MachineIndex(m_MachineIndex);
	m_pParamGui->Init();

	m_bCreatingWindow = false;

	CWindow dsk(GetDesktopWindow());
	CRect rClient;
	dsk.GetClientRect(&rClient);

	MoveWindow(rClient.Width() / 2 - ( width / 2), rClient.Height() / 2 - (height / 2), width, height, true);	

//	SetWindowPos(this, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

	UpdateLayout();
	m_Splitter.SetSplitterPos(guiWidth);
	m_pWindow->SetFocus();
	return 0;
}

void CVstEditorDlg::OnFinalMessage(HWND hWnd)
{
	CFrameWindowImpl<CVstEditorDlg>::OnFinalMessage(hWnd);
	m_pMachine->editorWnd = NULL;
	
	if(m_pParamGui != NULL){
		//m_pParamGui->DestroyWindow();
		delete m_pParamGui;
		m_pParamGui = NULL;
	}

/*
	if(m_pWindow != NULL){
		//m_pWindow->DestroyWindow();
		if(m_bEditorGui)
		{
			CVstGui* ptemp = (CVstGui*)m_pWindow;
			delete ptemp;
		} else {
			CFrameMachine *ptemp = (CFrameMachine*)m_pWindow;
			delete ptemp;
		}
		m_pWindow = NULL;
	}
*/	

	delete this;
}
