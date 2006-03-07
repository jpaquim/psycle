#if !defined(AFX_SUBMARCO_H__2CEE7C17_3D28_11D4_AB64_00C026101D9A__INCLUDED_)
#define AFX_SUBMARCO_H__2CEE7C17_3D28_11D4_AB64_00C026101D9A__INCLUDED_
/** @file 
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 *
 */


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WaveEdChildView.h"

class CMainFrame;

class CWaveEdFrame :  public CFrameWindowImpl<CWaveEdFrame> ,public CUpdateUI<CWaveEdFrame>,
		public CMessageFilter, public CIdleHandler
{
public:
	/// ウィンドウクラス定義
	DECLARE_FRAME_WND_CLASS_EX(NULL, IDR_WAVEFRAME,CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,0)
	CWaveEdFrame();///< コンストラクタ
	CWaveEdFrame(Song* _sng,CMainFrame* pframe);///< デストラクタ
    virtual BOOL OnIdle();///< アイドル時の処理
	virtual ~CWaveEdFrame();///< デストラクタ
    virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(this->IsWindowVisible()){
			if(CFrameWindowImpl<CWaveEdFrame>::PreTranslateMessage(pMsg))
				return TRUE;
			return m_WaveView.PreTranslateMessage(pMsg);
		}
		return FALSE;
	}

	
//	SetWave(signed short *pleft,signed short *pright,int numsamples, bool stereo);
	void GenerateView();
	void Notify(void);
	Song *m_pSong;
	CMainFrame *m_pFrame;

	// UI Map //
	
	BEGIN_UPDATE_UI_MAP(CWaveEdFrame)
		UPDATE_ELEMENT(ID_SELECTION_AMPLIFY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECTION_REVERSE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECTION_FADEIN, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECTION_FADEOUT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECTION_NORMALIZE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECTION_REMOVEDC, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECCION_ZOOM, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECCION_ZOOMOUT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_SELECTION_SHOWALL, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_DELETE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CONVERT_MONO, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_SELECT_ALL, UPDUI_MENUPOPUP)
	    UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(2, UPDUI_STATUSBAR)
    END_UPDATE_UI_MAP()

	// Message Map //
	
	BEGIN_MSG_MAP_EX(CWaveEdFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER_EX2(ID_SELECCION_ZOOM, m_WaveView.OnSelectionZoom)
		COMMAND_ID_HANDLER_EX2(ID_SELECCION_ZOOMOUT, m_WaveView.OnSelectionZoomOut)
		COMMAND_ID_HANDLER_EX2(ID_SELECTION_FADEIN, m_WaveView.OnSelectionFadeIn)
		COMMAND_ID_HANDLER_EX2(ID_SELECTION_FADEOUT, m_WaveView.OnSelectionFadeOut)
		COMMAND_ID_HANDLER_EX2(ID_SELECTION_NORMALIZE, m_WaveView.OnSelectionNormalize)
		COMMAND_ID_HANDLER_EX2(ID_SELECTION_REMOVEDC, m_WaveView.OnSelectionRemoveDC)
		COMMAND_ID_HANDLER_EX2(ID_SELECTION_AMPLIFY, m_WaveView.OnSelectionAmplify)
		COMMAND_ID_HANDLER_EX2(ID_SELECTION_REVERSE, m_WaveView.OnSelectionReverse)
		COMMAND_ID_HANDLER_EX2(ID_SELECTION_SHOWALL, m_WaveView.OnSelectionShowall)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_COPY, m_WaveView.OnEditCopy)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_CUT, m_WaveView.OnEditCut)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_PASTE, m_WaveView.OnEditPaste)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_DELETE, m_WaveView.OnEditDelete)
		COMMAND_ID_HANDLER_EX2(ID_CONVERT_MONO, m_WaveView.OnConvertMono)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_SELECT_ALL, m_WaveView.OnEditSelectAll)
		CHAIN_MSG_MAP(CUpdateUI<CWaveEdFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CWaveEdFrame>)
	END_MSG_MAP()
private:
	void AdjustStatusBar(int ins, int wav);
	CMultiPaneStatusBarCtrl m_StatusBar;///< Status Bar  ステータスバー
	CWaveEdChildView m_WaveView;///< View  ビュー

// Implementation
private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUBMARCO_H__2CEE7C17_3D28_11D4_AB64_00C026101D9A__INCLUDED_)
