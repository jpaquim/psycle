#if !defined(AFX_WAVEEDCHILDVIEW_H__266742FB_1A13_41EF_BE56_FAF1FB456280__INCLUDED_)
#define AFX_WAVEEDCHILDVIEW_H__266742FB_1A13_41EF_BE56_FAF1FB456280__INCLUDED_
/** @File 
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "WaveEdAmplifyDialog.h"
//#include "WaveEdFrame.h"
class CMainFrame;
class CWaveEdFrame;
class Song;

/** @Class CWaveEdChiled View */
class CWaveEdChildView : public CWindowImpl<CWaveEdChildView>
{
public:
	CWaveEdChildView();
	virtual ~CWaveEdChildView();
	DECLARE_WND_CLASS(NULL)
	void SetSong(Song*);
	void SetParent(CMainFrame*);
	void SetWaveEdFrame(CWaveEdFrame* p){
		m_pWaveEdFrame = p;
	};


	void GenerateAndShow();
	void SetViewData(int ins, int wav);
    void UpdateUICompo();
	BOOL PreTranslateMessage(MSG* pMsg){return FALSE;};
		/*
		BEGIN_MESSAGE_MAP(CWaveEdChildView, CWnd)
			//{{AFX_MSG_MAP(CWaveEdChildView)
			ON_WM_PAINT()
			ON_WM_RBUTTONDOWN()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONDBLCLK()

			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_COMMAND(ID_SELECCION_ZOOM, OnSelectionZoom)
			ON_COMMAND(ID_SELECCION_ZOOMOUT, OnSelectionZoomOut)
			ON_COMMAND(ID_SELECTION_FADEIN, OnSelectionFadeIn)
			ON_COMMAND(ID_SELECTION_FADEOUT, OnSelectionFadeOut)
			ON_COMMAND(ID_SELECTION_NORMALIZE, OnSelectionNormalize)
			ON_COMMAND(ID_SELECTION_REMOVEDC, OnSelectionRemoveDC)
			ON_COMMAND(ID_SELECTION_AMPLIFY, OnSelectionAmplify)
			ON_COMMAND(ID_SELECTION_REVERSE, OnSelectionReverse)
			ON_COMMAND(ID_SELECTION_SHOWALL, OnSelectionShowall)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_AMPLIFY, OnUpdateSelectionAmplify)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_REVERSE, OnUpdateSelectionReverse)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_FADEIN, OnUpdateSelectionFadein)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_FADEOUT, OnUpdateSelectionFadeout)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_NORMALIZE, OnUpdateSelectionNormalize)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_REMOVEDC, OnUpdateSelectionRemovedc)
			ON_UPDATE_COMMAND_UI(ID_SELECCION_ZOOM, OnUpdateSeleccionZoom)
			ON_UPDATE_COMMAND_UI(ID_SELECCION_ZOOMOUT, OnUpdateSeleccionZoomout)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_SHOWALL, OnUpdateSelectionShowall)
			ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
			ON_COMMAND(ID_EDIT_CUT, OnEditCut)
			ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
			ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
			ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
			ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
			ON_COMMAND(ID_CONVERT_MONO, OnConvertMono)
			ON_UPDATE_COMMAND_UI(ID_CONVERT_MONO, OnUpdateConvertMono)
			ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
			ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
			ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
			ON_WM_DESTROYCLIPBOARD()
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()
		*/
public:
	BEGIN_MSG_MAP_EX(CWaveEdChildView)
			MSG_WM_PAINT(OnPaint)
			MSG_WM_RBUTTONDOWN(OnRButtonDown)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_LBUTTONUP(OnLButtonUp)

			//WM_UPDATE_COMMAND_UI(ID_SELECTION_AMPLIFY, OnUpdateSelectionAmplify)
			//WM_UPDATE_COMMAND_UI(ID_SELECTION_REVERSE, OnUpdateSelectionReverse)
			//WM_UPDATE_COMMAND_UI(ID_SELECTION_FADEIN, OnUpdateSelectionFadein)
			//WM_UPDATE_COMMAND_UI(ID_SELECTION_FADEOUT, OnUpdateSelectionFadeout)
			//WM_UPDATE_COMMAND_UI(ID_SELECTION_NORMALIZE, OnUpdateSelectionNormalize)
			//WM_UPDATE_COMMAND_UI(ID_SELECTION_REMOVEDC, OnUpdateSelectionRemovedc)
			//WM_UPDATE_COMMAND_UI(ID_SELECCION_ZOOM, OnUpdateSeleccionZoom)
			//WM_UPDATE_COMMAND_UI(ID_SELECCION_ZOOMOUT, OnUpdateSeleccionZoomout)
			//WM_UPDATE_COMMAND_UI(ID_SELECTION_SHOWALL, OnUpdateSelectionShowall)
			//WM_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
			MSG_WM_DESTROYCLIPBOARD(OnDestroyClipboard)
			FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	 void OnSelectionZoom();
	 void OnSelectionZoomOut();
	 void OnSelectionFadeIn();
	 void OnSelectionFadeOut();	
	 void OnSelectionNormalize();
	 void OnSelectionRemoveDC();
	 void OnSelectionAmplify();
	 void OnSelectionReverse();
	 void OnSelectionShowall();
//	 void OnUpdateSelectionAmplify(CCmdUI* pCmdUI);
//	 void OnUpdateSelectionReverse(CCmdUI* pCmdUI);
//	 void OnUpdateSelectionFadein(CCmdUI* pCmdUI);
//	 void OnUpdateSelectionFadeout(CCmdUI* pCmdUI);
//	 void OnUpdateSelectionNormalize(CCmdUI* pCmdUI);
//	 void OnUpdateSelectionRemovedc(CCmdUI* pCmdUI);
//	 void OnUpdateSeleccionZoom(CCmdUI* pCmdUI);
//	 void OnUpdateSeleccionZoomout(CCmdUI* pCmdUI);
//	 void OnUpdateSelectionShowall(CCmdUI* pCmdUI);
	 void OnEditCopy();
//	 void OnUpdateEditCopy(CCmdUI* pCmdUI);
	 void OnEditCut();
//	 void OnUpdateEditCut(CCmdUI* pCmdUI);
	 void OnEditPaste();
//	 void OnUpdateEditPaste(CCmdUI* pCmdUI);
	 void OnEditDelete();
//	 void OnUpdateEditDelete(CCmdUI* pCmdUI);
	 void OnConvertMono();
//	 void OnUpdateConvertMono(CCmdUI* pCmdUI);
//	 void OnUpdateEditUndo(CCmdUI* pCmdUI);
	 void OnEditSelectAll();

private:

	 void OnPaint(HDC dc);
	 void OnRButtonDown(UINT nFlags, CPoint point);
	 void OnLButtonDown(UINT nFlags, CPoint point);
	 void OnLButtonDblClk( UINT nFlags, CPoint point );
	 void OnMouseMove(UINT nFlags, CPoint point);
	 void OnLButtonUp(UINT nFlags, CPoint point);
//	 void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	 void OnDestroyClipboard();

	Song *m_pSong;
	
	// Painting pens
	CPen m_PenLow;
	CPen m_PenMedium;
	CPen m_PenHigh;

	//Clipboard
	char*	m_pClipboardData;
	HGLOBAL m_hClipboardData, m_hPasteData;

	// Wave data
	signed short* m_WdLeft;
	signed short* m_WdRight;
	bool m_WdStereo;
	unsigned long m_WdLength;
	unsigned long m_WdLoopS;
	unsigned long m_WdLoopE;
	bool m_bWdLoop;

	// Display data
	unsigned long m_DiStart;
	unsigned long m_DiLength;
	unsigned long m_BlStart;
	unsigned long m_BlLength;
	bool m_bBlSelection;
	bool m_bWdWave;

	unsigned long m_SelX, m_SelX2;

	int m_WsInstrument;
	int m_WsWave;

	bool m_bDrawWave;

	unsigned long m_SelStart; // Selection start (point where left clicked);

	CWaveEdAmplifyDialog m_AmpDialog;

	CMainFrame* m_pParent;
	CWaveEdFrame* m_pWaveEdFrame;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEEDCHILDVIEW_H__266742FB_1A13_41EF_BE56_FAF1FB456280__INCLUDED_)
