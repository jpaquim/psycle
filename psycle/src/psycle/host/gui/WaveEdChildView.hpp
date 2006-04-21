///\file
///\brief interface file for psycle::host::CWaveEdChildView.
#pragma once
#include <psycle/host/gui/WaveEdAmplifyDialog.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		class CMainFrame;
		class Song;

		/// wave editor window.
		class CWaveEdChildView : public CWnd
		{
		public:
			CWaveEdChildView();
			void SetSong(Song*);
			void SetParent(CMainFrame*);
			virtual ~CWaveEdChildView();
			void GenerateAndShow();
			void SetViewData(int ins);

			unsigned long GetSelectionLength();
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWaveEdChildView)
			protected:
			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
		protected:
			//{{AFX_MSG(CWaveEdChildView)
			afx_msg void OnPaint();
			afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnSelectionZoomIn();
			afx_msg void OnSelectionZoomSel();
			afx_msg void OnSelectionZoomOut();
			afx_msg void OnSelectionFadeIn();
			afx_msg void OnSelectionFadeOut();	
			afx_msg void OnSelectionNormalize();
			afx_msg void OnSelectionRemoveDC();
			afx_msg void OnSelectionAmplify();
			afx_msg void OnSelectionReverse();
			afx_msg void OnSelectionShowall();
			afx_msg void OnUpdateSelectionAmplify(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSelectionReverse(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSelectionFadein(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSelectionFadeout(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSelectionNormalize(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSelectionRemovedc(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSeleccionZoomIn(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSelectionZoomSel(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSeleccionZoomOut(CCmdUI* pCmdUI);
			afx_msg void OnUpdateSelectionShowall(CCmdUI* pCmdUI);
			afx_msg void OnEditCopy();
			afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
			afx_msg void OnEditCut();
			afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
			afx_msg void OnEditCrop();
			afx_msg void OnUpdateEditCrop(CCmdUI* pCmdUI);
			afx_msg void OnEditPaste();
			afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
			afx_msg void OnEditDelete();
			afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
			afx_msg void OnConvertMono();
			afx_msg void OnUpdateConvertMono(CCmdUI* pCmdUI);
			afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
			afx_msg void OnEditSelectAll();
			afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
			afx_msg void OnDestroyClipboard();
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnSize(UINT nType, int cx, int cy);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		private:
			Song *_pSong;
			
			// Painting pens
			CPen cpen_lo;
			CPen cpen_me;
			CPen cpen_hi;
			CPen cpen_white;

			HCURSOR hResizeLR;		//left/right arrows cursor for sizing selection
			HCURSOR hIBeam;			//i beam cursor

			//Clipboard
			char*	pClipboardData;
			HGLOBAL hClipboardData, hPasteData;

			// Wave data
			signed short* wdLeft;
			signed short* wdRight;
			bool wdStereo;
			unsigned long wdLength;
			unsigned long wdLoopS;
			unsigned long wdLoopE;
			bool wdLoop;

			// Display data
			unsigned long diStart;		//first sample in current window
			unsigned long diLength;		//number of samples in window
			unsigned long blStart;		//first sample of selection
			unsigned long blLength;		//number of samples selected
			bool blSelection;			//whether data is selected currently
			bool wdWave;				//whether we have a wave to display

			unsigned long selx, selx2;

			int wsInstrument;

			bool drawwave;

			unsigned long SelStart;		//the end of the selection -not- being moved 

			CWaveEdAmplifyDialog AmpDialog;

			CScrollBar hScroll;

			CMainFrame* pParent;
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
