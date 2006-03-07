/** @file PsycleWTLView.h 
 *  @brief interface of the DefaultPatternView class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.2 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define EDIT_COLUMNS_END 12
#define EDIT_COLUMNS_START 0

#include "Song.h"
#include "configuration.h"
#include "InputHandler.h"

#define MAX_DRAW_MESSAGES 32

class CGearTracker;
struct SF::IUndo;

enum 
{
	DMAll = 0,		// Repaints everything (means, slow). Used when switching views, or when a
					// whole update is needed (For example, when changing pattern Properties, or TPB)
	DMAllMacsRefresh, //Used to refresh all the machines, without refreshing the background/wires
	DMMacRefresh,	// Used to refresh the image of one machine (mac num in "m_UpdatePar")

	DMPattern,		// Use this when switching Patterns (changing from one to another)
	DMData,			// Data has Changed. Which data to update is indicated with DrawLineStart/End
					// and DrawTrackStart/End
					// Use it when editing and copy/pasting
	DMHScroll,		// Refresh called by the scrollbars or by mouse scrolling (when selecting).
					// New values in m_NTrackOffset and m_NLineOffset variables ( new_track_offset and new_line_offset);
	DMVScroll,		// Refresh called by the scrollbars or by mouse scrolling (when selecting).
					// New values in m_NTrackOffset and m_NLineOffset variables ( new_track_offset and new_line_offset);
//	DMResize,		// Indicates the Refresh is called from the "OnSize()" event.
	DMPlayback,		// Indicates it needs a refresh caused by Playback (update playback cursor)
	DMPlaybackChange,// Indicates that while playing, a pattern switch is needed.
	DMCursor,		// Indicates a movement of the cursor. update the values to "m_EditCursor" directly
					// and call this function.
					// this is arbitrary message as cursor is checked
	DMSelection,	// The selection has changed. use "m_BlockSelection" to indicate the values.
	DMTrackHeader,  // Track header refresh (mute/solo, Record updating)
//	DMPatternHeader,// Octave, Pattern name, Edit Mode on/off
	DMNone			// Do not use this one directly. It is used to detect refresh calls from the OS.

	// If you add any new method, please, add the proper code to "PreparePatternRefresh()" and to
	// "DrawPatternEditor()".
	// Note: Modes are sorted by priority. (although it is not really used)

	// !!!BIG ADVISE!!! : The execution of Repaint(mode) does not imply an instant refresh of
	//						the Screen, and what's worse, you might end calling Repaint(anothermode)
	//						previous of the first refresh. In PreparePatternRefresh() there's code
	//						to avoid problems when two modes do completely different things. On
	//						other cases, it still ends to wrong content being shown.
};

class CCursor
{
public:
	CCursor() { m_Line = 0; m_Column = 0; m_Track = 0; };
	
	void Line(const UINT value){m_Line = value;};
	const UINT Line(void){return m_Line;};
	
	const UINT Column(void){return m_Column;};
	void Column(const UINT value){m_Column = value;};

	const UINT Track(void) { return m_Track;};
	void Track(const UINT value){ m_Track = value;};

private:
	UINT m_Line;
	UINT m_Column;
	UINT m_Track;
};

class CSelection
{
public:
	CCursor start;	// Column not used. (It was harder to select!!!)
	CCursor end;	//
};

struct SPatternDraw
{
	int drawTrackStart;
	int drawTrackEnd;
	int drawLineStart;
	int drawLineEnd;
};

struct SSkinSource
{
	int x;
	int y;
	int width;
	int height;
};

struct SSkinDest
{
	int x;
	int y;
};

struct SPatternHeaderCoords
{
	SSkinSource sBackground;
	SSkinSource sNumber0;
	SSkinSource sRecordOn;
	SSkinSource sMuteOn;
	SSkinSource sSoloOn;
	SSkinDest dDigitX0;
	SSkinDest dDigit0X;
	SSkinDest dRecordOn;
	SSkinDest dMuteOn;
	SSkinDest dSoloOn;
	BOOL bHasTransparency;
	COLORREF cTransparency;
};

namespace configuration {

	class DefaultPatternView : public iCompornentConfiguration 
	{
	public:
		DefaultPatternView();
		~DefaultPatternView();
	
		/** Read Config File **/
		void Read(const string& settingFileName);
		/** Read Default Config**/
		void Read();
		
		/** Config File Read **/
		void Write(const string& settingFileName);
		/** Read Default Config **/
		void Write();

	private:

		COLORREF m_separator;
		COLORREF m_separator2;
		COLORREF m_background;
		COLORREF m_background2;
		COLORREF m_row4beat;
		COLORREF m_row4beat2;
		COLORREF m_rowbeat;
		COLORREF m_rowbeat2;
		COLORREF m_row;
		COLORREF m_row2;
		COLORREF m_font;
		COLORREF m_font2;
		COLORREF m_fontPlay;
		COLORREF m_fontPlay2;
		COLORREF m_fontCur;
		COLORREF m_fontCur2;
		COLORREF m_fontSel;
		COLORREF m_fontSel2;
		COLORREF m_selection;
		COLORREF m_selection2;
		COLORREF m_playbar;
		COLORREF m_playbar2;
		COLORREF m_cursor;
		COLORREF m_cursor2;

		TCHAR m_fontface[64];
		TCHAR m_header_skin[64];

		int m_font_point;
		int m_font_x;
		int m_font_y;

		UINT m_font_flags;
		UINT m_effect_font_flags;///< 

		bool m_draw_empty_data;///< データがないところも描画するか？
	
		bool _linenumbers;///< 行番号を表示するか
		bool _linenumbersHex;///< 行番号を16進数で表示するか
		bool _linenumbersCursor;///<　行番号にカーソル
		bool _followSong;///< Songに追従するか

		bool _wrapAround;///< カーソルラップアラウンド
		bool _centerCursor;///< カーソルを常に中心にするか
		bool _cursorAlwaysDown;///< カーソルはいつも下に移動するか

		bool _RecordNoteoff;///< ノートオフを記録するか
		bool _RecordTweaks;///< Tweakを記録するか
		bool _notesToEffects;///< EffectへのNoteを許可？？

	};
};
/////////////////////////////////////////////////////////////////////////////
// DefaultPatternView window
//typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS & ~WS_BORDER,WS_EX_CLIENTEDGE> DefaultPatternViewTraits;
typedef CWinTraits<WS_CHILD | WS_VISIBLE ,WS_EX_CLIENTEDGE> DefaultPatternViewTraits;
class DefaultPatternView : public CWindowImpl<DefaultPatternView,CWindow,DefaultPatternViewTraits>
{
public:
	DECLARE_WND_CLASS_EX(_T("DefaultPatternView"),CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,NULL)
	
	DefaultPatternView();///< コンストラクタ
	void Initialize();///< 初期化
	~DefaultPatternView();///< デストラクタ

	void SelectMachineUnderCursor(void);

	void ValidateParent();
	void EnableSound();
	void Repaint(int drawMode=DMAll);

	void ShowPatternDlg(void);
	void BlockInsChange(int x);
	void BlockGenChange(int x);
	int SongIncBpm(int x);
	void ShowSwingFillDlg(bool bTrackMode);

	void MidiPatternNote(int outnote , int velocity);	// called by the MIDI input to insert pattern notes
	void MidiPatternCommand(int command, int value); // called by midi to insert pattern commands
	void MidiPatternTweak(int command, int value); // called by midi to insert pattern commands
	void MidiPatternTweakSlide(int command, int value); // called by midi to insert pattern commands
	void MidiPatternMidiCommand(int command, int value); // called by midi to insert midi pattern commands
	void MidiPatternInstrument(int value); // called by midi to insert pattern commands
	void MousePatternTweak(int machine, int command, int value);
	void MousePatternTweakSlide(int machine, int command, int value);
	void EnterNote(int note, int velocity=127, bool bTranspose=true);
	void EnterNoteoffAny();
	bool MSBPut(int nChar);
	void PrevTrack(int x,bool wrap,bool updateDisplay=true);
	void AdvanceTrack(int x,bool wrap,bool updateDisplay=true);
	void PrevCol(bool wrap,bool updateDisplay=true);
	void NextCol(bool wrap,bool updateDisplay=true);
	void PrevLine(int x,bool wrap,bool updateDisplay=true);
	void AdvanceLine(int x,bool wrap,bool updateDisplay=true);
	void DeleteCurr();
	void InsertCurr();
	void ClearCurr();

	void PlayCurrentRow(void);
	void PlayCurrentNote(void);

	void patCopy();
	void patPaste();
	void patMixPaste();
	void patCut();
	void patDelete();
	void patTranspose(int trp);
	void HalveLength();
	void DoubleLength();
	void BlockTranspose(int trp);
	void BlockParamInterpolate();
	void StartBlock(int track,int line, int col);
	void ChangeBlock(int track,int line, int col);	// This function allows a handier usage for Shift+Arrows and MouseSelection
										// Params: current track, line and col
										// Result: Update the selected region depending on the new and old values.
	void EndBlock(int track,int line, int col);
	void CopyBlock(bool cutit);
	void PasteBlock(int tx,int lx,bool mix);
	void DeleteBlock();
	void BlockUnmark(void);

	void DecCurPattern();
	void IncCurPattern();
	void IncPosition(bool bRepeat=false);
	void DecPosition();


	//void AddRedo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
	//void AddUndoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=TRUE, int counter=0);
	//void AddRedoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
	//void AddUndoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=TRUE, int counter=0);
	//void AddRedoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
	//void AddUndoSong(int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=TRUE, int counter=0);
	//void AddRedoSong(int edittrack, int editline, int editcol, int seqpos, int counter);

	void KillUndo();
	void KillRedo();
	void SelectNextTrack();  // for armed tracks recording
	void RecalculateColourGrid();
	void RecalcMetrics();
	void LoadPatternHeaderSkin();
	void patTrackMute();
	void patTrackSolo();
	void patTrackRecord();
	void KeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	void KeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	void UpdateUI();

	BOOL PreTranslateMessage(MSG* pMsg);


	void AddUndo(SF::IUndo * const pUndo,const bool bWipeRedo = false){
		m_UndoController.AddUndo(pUndo,bWipeRedo);
//		SetTitleBarText();
	};

	void AddRedo(SF::IUndo * const pUndo){
		m_UndoController.AddRedo(pUndo);
	};

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	// TODO: マシン描画に関するコードを除去する
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp( UINT nFlags, CPoint point );
	void OnMouseMove( UINT nFlags, CPoint point );
	void OnLButtonDblClk( UINT nFlags, CPoint point );

	void OnDestroy();
	void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );


	void OnPopCut();
	void OnPopCopy();
	void OnPopPaste();
	void OnPopMixpaste();
	void OnPopDelete();
	void OnPopInterpolate();
	void OnPopChangegenerator();
	void OnPopChangeinstrument();
	void OnPopTranspose1();
	void OnPopTranspose12();
	void OnPopTranspose_1();
	void OnPopTranspose_12();
	void OnPopPattenproperties();
	void OnPopBlockSwingfill();
	void OnPopTrackSwingfill();

	void OnSize(UINT nType, CSize &size);
	void OnContextMenu(HWND hWnd, CPoint& point);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);

	void OnEditUndo();
	void OnEditRedo();

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnMButtonDown( UINT nFlags, CPoint& point );

    void SaveBlock(FILE* file);
	void LoadBlock(FILE* file);
	
	::Song & Song(){return *m_pSong;}; 
	
	inline int _xtoCol(int pointpos);

	CCursor& EditCursor(){return m_EditCursor;};
	
	const int EditPosition(){return m_EditPosition;};
	void EditPosition(const int value){m_EditPosition = value;};

	void NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd);

private:
	
	//Recent Files!!!!//

	void PreparePatternRefresh(int drawMode);
	void DrawPatEditor(CDC * const devc);
	void DrawPatternData(CDC * const devc,int tstart,int tend, int lstart, int lend);
//	void DrawMultiPatternData(CDC *devc,int tstart,int tend, int lstart, int lend);
	inline void OutNote(CDC *devc,int x,int y,int note);
	inline void OutData(CDC *devc,int x,int y,BYTE data,bool trflag);
	inline void OutData4(CDC *devc,int x,int y,BYTE data,bool trflag);
	inline void TXT(CDC *devc,TCHAR *txt, int x,int y,int w,int h);
	inline void TXTFLAT(CDC *devc,TCHAR *txt, int x,int y,int w,int h);
	inline void BOX(CDC *devc,int x,int y, int w, int h);
	inline void BOX(CDC *devc,CRect rect);

//	void NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd);

	void RecalculateColour(COLORREF* pDest, COLORREF source1, COLORREF source2);
	COLORREF ColourDiffAdd(COLORREF base, COLORREF adjust, COLORREF add);
	void FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result);
	void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
	void TransparentBlt(CDC* pDC, int xStart,  int yStart, int wWidth,  int wHeight, CDC* pTmpDC, CBitmap* bmpMask, int xSource = 0, int ySource = 0);

	CMainFrame* m_pMainFrame;
	::Song* m_pSong;

	bool m_bUseDoubleBuffer;

	bool m_BlockSelected;
	bool m_BlockStart;
	bool m_bScrollDetach;
	CCursor m_EditCursor;	// Edit Cursor Position in Pattern.
	CCursor m_DetatchPoint;
	bool m_bEditMode;		// in edit mode?
	bool m_bPrepare;

	int m_PatStep;

	int m_EditPosition;	// Position in the Sequence!
	int m_PrevEditPosition;

	int m_ChordModeOffs;
	int m_ChordModeLine;
	int m_ChordModeTrack;
	int m_UpdateMode;
	int m_UpdatePar;			// VMPattern: Display update mode. VMMachine: Machine number to update.

	int m_XOFFSET;
	int m_YOFFSET;
	int m_ROWWIDTH;
	int m_ROWHEIGHT; // textheight+1
	int m_TEXTWIDTH;
	int m_TEXTHEIGHT;
	int m_HEADER_INDENT;
	int m_HEADER_HEIGHT;
	int m_HEADER_ROWWIDTH;
	int m_CH;
	int m_CW;
	int m_VISTRACKS;
	int m_COLX[EDIT_COLUMNS_END + 2];

	SPatternHeaderCoords m_PatHeaderCoords;

	// GDI Stuff
	CBitmap m_PatternHeader;
	CBitmap m_PatternHeaderMask;
	HBITMAP m_hbmPatHeader;
	CBitmap* m_BmpDC;
	int m_FLATSIZES[256];
	int m_VISLINES;

	int m_BkgX;
	int m_BkgY;

	int m_PlayPosition;		// Play Cursor Position in Screen // left and right are unused
	int m_NewPlayPosition;		// Play Cursor Position in Screen that is gonna be drawn.

	CRect m_SelectionPosition;		// Selection Block in Screen
	CRect m_newm_Selpos;	// Selection Block in Screen that is gonna be drawn.
	CCursor m_EditLastPosition;	// Edit Cursor Position in Pattern.

	SPatternDraw m_pPatternDraw[MAX_DRAW_MESSAGES];
	int m_NumPatternDraw;

	int m_MaxTracks;		// num of tracks shown
	int m_MaxLines;		// num of lines shown
	int m_TrackOffset;		// Track Offset (first track shown)
	int m_LineOffset;		// Line Offset (first line shown)
	int m_NTrackOffset;		// These two variables are used for the DMScroll functino
	int m_NLineOffset;
	int m_RNTrackOffset;
	int m_RNLineOffset;

	TCHAR m_BlankParam[2];
	TCHAR m_BlankNote[4];

	CCursor m_IniSelection;
	CSelection m_BlockSelection;
	CCursor m_OldM;	// Indicates the previous track/line/col when selecting (used for mouse)
	CPoint m_MBStart; 

	bool m_bBlockCopied;
	BYTE m_BlockBufferData[ EVENT_SIZE * MAX_LINES * MAX_TRACKS];
	int	m_BlockNTracks;
	int	m_BlockNLines;
	
	BYTE m_PatBufferData[ EVENT_SIZE * MAX_LINES * MAX_TRACKS];
	int m_PatBufferLines;
	bool m_PatBufferCopy;

//	SPatternUndo * pUndoList;
//	SPatternUndo * pRedoList;

	int m_UndoCounter;
	int m_UndoSaved;

	int mcd_x;
	int mcd_y;

	COLORREF pvc_separator[MAX_TRACKS + 1];
	COLORREF pvc_background[MAX_TRACKS + 1];
	COLORREF pvc_row4beat[MAX_TRACKS + 1];
	COLORREF pvc_rowbeat[MAX_TRACKS + 1];
	COLORREF pvc_row[MAX_TRACKS + 1];
	COLORREF pvc_selection[MAX_TRACKS + 1];
	COLORREF pvc_playbar[MAX_TRACKS + 1];
	COLORREF pvc_cursor[MAX_TRACKS + 1];
	COLORREF pvc_font[MAX_TRACKS + 1];
	COLORREF pvc_fontPlay[MAX_TRACKS + 1];
	COLORREF pvc_fontCur[MAX_TRACKS + 1];
	COLORREF pvc_fontSel[MAX_TRACKS + 1];
	COLORREF pvc_selectionbeat[MAX_TRACKS + 1];
	COLORREF pvc_selection4beat[MAX_TRACKS + 1];

	DWORD m_elapsedTime;///< デバッグ用経過時間 


	// String Resource
	SF::CResourceString m_FmtPan;
	SF::CResourceString m_FmtPan1;

public:
	BEGIN_MSG_MAP_EX(DefaultPatternView)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_KEYDOWN2(OnKeyDown)
		MSG_WM_KEYUP2(OnKeyUp)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_MBUTTONDOWN(OnMButtonDown)
		FORWARD_NOTIFICATIONS();
	END_MSG_MAP()
//	LRESULT OnNcRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//////////////////////////////////////////////////////////////////////
// Pattern data display functions
inline void DefaultPatternView::OutNote(CDC *devc,int x,int y,int note)
{
	int const srx = (m_TEXTWIDTH * 3) + 1;
	int const sry = m_TEXTHEIGHT;
	
	switch(note)
	{
	case 255: TXTFLAT(devc,m_BlankNote,x,y,srx,sry);break;
//	case 255: TXTFLAT(devc,_T("   "),x,y,srx,sry);break;
	case 0:   TXTFLAT(devc,_T("C-0"),x,y,srx,sry);break;
	case 1:   TXTFLAT(devc,_T("C#0"),x,y,srx,sry);break;
	case 2:   TXTFLAT(devc,_T("D-0"),x,y,srx,sry);break;
	case 3:   TXTFLAT(devc,_T("D#0"),x,y,srx,sry);break;
	case 4:   TXTFLAT(devc,_T("E-0"),x,y,srx,sry);break;
	case 5:   TXTFLAT(devc,_T("F-0"),x,y,srx,sry);break;
	case 6:   TXTFLAT(devc,_T("F#0"),x,y,srx,sry);break;
	case 7:   TXTFLAT(devc,_T("G-0"),x,y,srx,sry);break;
	case 8:   TXTFLAT(devc,_T("G#0"),x,y,srx,sry);break;
	case 9:   TXTFLAT(devc,_T("A-0"),x,y,srx,sry);break;
	case 10:  TXTFLAT(devc,_T("A#0"),x,y,srx,sry);break;
	case 11:  TXTFLAT(devc,_T("B-0"),x,y,srx,sry);break;
	case 12:  TXTFLAT(devc,_T("C-1"),x,y,srx,sry);break;
	case 13:  TXTFLAT(devc,_T("C#1"),x,y,srx,sry);break;
	case 14:  TXTFLAT(devc,_T("D-1"),x,y,srx,sry);break;
	case 15:  TXTFLAT(devc,_T("D#1"),x,y,srx,sry);break;
	case 16:  TXTFLAT(devc,_T("E-1"),x,y,srx,sry);break;
	case 17:  TXTFLAT(devc,_T("F-1"),x,y,srx,sry);break;
	case 18:  TXTFLAT(devc,_T("F#1"),x,y,srx,sry);break;
	case 19:  TXTFLAT(devc,_T("G-1"),x,y,srx,sry);break;
	case 20:  TXTFLAT(devc,_T("G#1"),x,y,srx,sry);break;
	case 21:  TXTFLAT(devc,_T("A-1"),x,y,srx,sry);break;
	case 22:  TXTFLAT(devc,_T("A#1"),x,y,srx,sry);break;
	case 23:  TXTFLAT(devc,_T("B-1"),x,y,srx,sry);break;
	case 24:  TXTFLAT(devc,_T("C-2"),x,y,srx,sry);break;
	case 25:  TXTFLAT(devc,_T("C#2"),x,y,srx,sry);break;
	case 26:  TXTFLAT(devc,_T("D-2"),x,y,srx,sry);break;
	case 27:  TXTFLAT(devc,_T("D#2"),x,y,srx,sry);break;
	case 28:  TXTFLAT(devc,_T("E-2"),x,y,srx,sry);break;
	case 29:  TXTFLAT(devc,_T("F-2"),x,y,srx,sry);break;
	case 30:  TXTFLAT(devc,_T("F#2"),x,y,srx,sry);break;
	case 31:  TXTFLAT(devc,_T("G-2"),x,y,srx,sry);break;
	case 32:  TXTFLAT(devc,_T("G#2"),x,y,srx,sry);break;
	case 33:  TXTFLAT(devc,_T("A-2"),x,y,srx,sry);break;
	case 34:  TXTFLAT(devc,_T("A#2"),x,y,srx,sry);break;
	case 35:  TXTFLAT(devc,_T("B-2"),x,y,srx,sry);break;
	case 36:  TXTFLAT(devc,_T("C-3"),x,y,srx,sry);break;
	case 37:  TXTFLAT(devc,_T("C#3"),x,y,srx,sry);break;
	case 38:  TXTFLAT(devc,_T("D-3"),x,y,srx,sry);break;
	case 39:  TXTFLAT(devc,_T("D#3"),x,y,srx,sry);break;
	case 40:  TXTFLAT(devc,_T("E-3"),x,y,srx,sry);break;
	case 41:  TXTFLAT(devc,_T("F-3"),x,y,srx,sry);break;
	case 42:  TXTFLAT(devc,_T("F#3"),x,y,srx,sry);break;
	case 43:  TXTFLAT(devc,_T("G-3"),x,y,srx,sry);break;
	case 44:  TXTFLAT(devc,_T("G#3"),x,y,srx,sry);break;
	case 45:  TXTFLAT(devc,_T("A-3"),x,y,srx,sry);break;
	case 46:  TXTFLAT(devc,_T("A#3"),x,y,srx,sry);break;
	case 47:  TXTFLAT(devc,_T("B-3"),x,y,srx,sry);break;
	case 48:  TXTFLAT(devc,_T("C-4"),x,y,srx,sry);break;
	case 49:  TXTFLAT(devc,_T("C#4"),x,y,srx,sry);break;
	case 50:  TXTFLAT(devc,_T("D-4"),x,y,srx,sry);break;
	case 51:  TXTFLAT(devc,_T("D#4"),x,y,srx,sry);break;
	case 52:  TXTFLAT(devc,_T("E-4"),x,y,srx,sry);break;
	case 53:  TXTFLAT(devc,_T("F-4"),x,y,srx,sry);break;
	case 54:  TXTFLAT(devc,_T("F#4"),x,y,srx,sry);break;
	case 55:  TXTFLAT(devc,_T("G-4"),x,y,srx,sry);break;
	case 56:  TXTFLAT(devc,_T("G#4"),x,y,srx,sry);break;
	case 57:  TXTFLAT(devc,_T("A-4"),x,y,srx,sry);break;
	case 58:  TXTFLAT(devc,_T("A#4"),x,y,srx,sry);break;
	case 59:  TXTFLAT(devc,_T("B-4"),x,y,srx,sry);break;
	case 60:  TXTFLAT(devc,_T("C-5"),x,y,srx,sry);break;
	case 61:  TXTFLAT(devc,_T("C#5"),x,y,srx,sry);break;
	case 62:  TXTFLAT(devc,_T("D-5"),x,y,srx,sry);break;
	case 63:  TXTFLAT(devc,_T("D#5"),x,y,srx,sry);break;
	case 64:  TXTFLAT(devc,_T("E-5"),x,y,srx,sry);break;
	case 65:  TXTFLAT(devc,_T("F-5"),x,y,srx,sry);break;
	case 66:  TXTFLAT(devc,_T("F#5"),x,y,srx,sry);break;
	case 67:  TXTFLAT(devc,_T("G-5"),x,y,srx,sry);break;
	case 68:  TXTFLAT(devc,_T("G#5"),x,y,srx,sry);break;
	case 69:  TXTFLAT(devc,_T("A-5"),x,y,srx,sry);break;
	case 70:  TXTFLAT(devc,_T("A#5"),x,y,srx,sry);break;
	case 71:  TXTFLAT(devc,_T("B-5"),x,y,srx,sry);break;
	case 72:  TXTFLAT(devc,_T("C-6"),x,y,srx,sry);break;
	case 73:  TXTFLAT(devc,_T("C#6"),x,y,srx,sry);break;
	case 74:  TXTFLAT(devc,_T("D-6"),x,y,srx,sry);break;
	case 75:  TXTFLAT(devc,_T("D#6"),x,y,srx,sry);break;
	case 76:  TXTFLAT(devc,_T("E-6"),x,y,srx,sry);break;
	case 77:  TXTFLAT(devc,_T("F-6"),x,y,srx,sry);break;
	case 78:  TXTFLAT(devc,_T("F#6"),x,y,srx,sry);break;
	case 79:  TXTFLAT(devc,_T("G-6"),x,y,srx,sry);break;
	case 80:  TXTFLAT(devc,_T("G#6"),x,y,srx,sry);break;
	case 81:  TXTFLAT(devc,_T("A-6"),x,y,srx,sry);break;
	case 82:  TXTFLAT(devc,_T("A#6"),x,y,srx,sry);break;
	case 83:  TXTFLAT(devc,_T("B-6"),x,y,srx,sry);break;
	case 84:  TXTFLAT(devc,_T("C-7"),x,y,srx,sry);break;
	case 85:  TXTFLAT(devc,_T("C#7"),x,y,srx,sry);break;
	case 86:  TXTFLAT(devc,_T("D-7"),x,y,srx,sry);break;
	case 87:  TXTFLAT(devc,_T("D#7"),x,y,srx,sry);break;
	case 88:  TXTFLAT(devc,_T("E-7"),x,y,srx,sry);break;
	case 89:  TXTFLAT(devc,_T("F-7"),x,y,srx,sry);break;
	case 90:  TXTFLAT(devc,_T("F#7"),x,y,srx,sry);break;
	case 91:  TXTFLAT(devc,_T("G-7"),x,y,srx,sry);break;
	case 92:  TXTFLAT(devc,_T("G#7"),x,y,srx,sry);break;
	case 93:  TXTFLAT(devc,_T("A-7"),x,y,srx,sry);break;
	case 94:  TXTFLAT(devc,_T("A#7"),x,y,srx,sry);break;
	case 95:  TXTFLAT(devc,_T("B-7"),x,y,srx,sry);break;
	case 96:  TXTFLAT(devc,_T("C-8"),x,y,srx,sry);break;
	case 97:  TXTFLAT(devc,_T("C#8"),x,y,srx,sry);break;
	case 98:  TXTFLAT(devc,_T("D-8"),x,y,srx,sry);break;
	case 99:  TXTFLAT(devc,_T("D#8"),x,y,srx,sry);break;
	case 100: TXTFLAT(devc,_T("E-8"),x,y,srx,sry);break;
	case 101: TXTFLAT(devc,_T("F-8"),x,y,srx,sry);break;
	case 102: TXTFLAT(devc,_T("F#8"),x,y,srx,sry);break;
	case 103: TXTFLAT(devc,_T("G-8"),x,y,srx,sry);break;
	case 104: TXTFLAT(devc,_T("G#8"),x,y,srx,sry);break;
	case 105: TXTFLAT(devc,_T("A-8"),x,y,srx,sry);break;
	case 106: TXTFLAT(devc,_T("A#8"),x,y,srx,sry);break;
	case 107: TXTFLAT(devc,_T("B-8"),x,y,srx,sry);break;
	case 108: TXTFLAT(devc,_T("C-9"),x,y,srx,sry);break;
	case 109: TXTFLAT(devc,_T("C#9"),x,y,srx,sry);break;
	case 110: TXTFLAT(devc,_T("D-9"),x,y,srx,sry);break;
	case 111: TXTFLAT(devc,_T("D#9"),x,y,srx,sry);break;
	case 112: TXTFLAT(devc,_T("E-9"),x,y,srx,sry);break;
	case 113: TXTFLAT(devc,_T("F-9"),x,y,srx,sry);break;
	case 114: TXTFLAT(devc,_T("F#9"),x,y,srx,sry);break;
	case 115: TXTFLAT(devc,_T("G-9"),x,y,srx,sry);break;
	case 116: TXTFLAT(devc,_T("G#9"),x,y,srx,sry);break;
	case 117: TXTFLAT(devc,_T("A-9"),x,y,srx,sry);break;
	case 118: TXTFLAT(devc,_T("A#9"),x,y,srx,sry);break;
	case 119: TXTFLAT(devc,_T("B-9"),x,y,srx,sry);break;
	case 120: TXTFLAT(devc,_T("off"),x,y,srx,sry);break;
	case cdefTweakM: TXTFLAT(devc,_T("twk"),x,y,srx,sry);break;
	case cdefTweakE: TXTFLAT(devc,_T("twf"),x,y,srx,sry);break;
	case cdefMIDICC: TXTFLAT(devc,_T("Mcm"),x,y,srx,sry);break;
	case cdefTweakS: TXTFLAT(devc,_T("tws"),x,y,srx,sry);break;
	}
}

inline void DefaultPatternView::OutData(CDC *devc,int x,int y,BYTE data, bool trflag)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+m_TEXTWIDTH;
	Rect.bottom=y+m_TEXTHEIGHT;
	
	if (trflag)
	{
		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,m_BlankParam,sizeof(m_BlankParam) / sizeof(TCHAR) - 1,m_FLATSIZES);

		Rect.left+=m_TEXTWIDTH; 
		Rect.right+=m_TEXTWIDTH;
		devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,m_BlankParam,sizeof(m_BlankParam) / sizeof(TCHAR) - 1,m_FLATSIZES);


//		Rect.right+=10;
//		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("  "),m_FLATSIZES);
		return;
	}
	
	switch(data>>4)
	{
	case 0x0: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("0"),1,m_FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("1"),1,m_FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("2"),1,m_FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("3"),1,m_FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("4"),1,m_FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("5"),1,m_FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("6"),1,m_FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("7"),1,m_FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("8"),1,m_FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("9"),1,m_FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("A"),1,m_FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("B"),1,m_FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("C"),1,m_FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("D"),1,m_FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("E"),1,m_FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("F"),1,m_FLATSIZES);break;
	}
	
	Rect.left+=m_TEXTWIDTH;
	Rect.right+=m_TEXTWIDTH;
	
	switch(data&0xf)
	{
	case 0x0: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("0"),1,m_FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("1"),1,m_FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("2"),1,m_FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("3"),1,m_FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("4"),1,m_FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("5"),1,m_FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("6"),1,m_FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("7"),1,m_FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("8"),1,m_FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("9"),1,m_FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("A"),1,m_FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("B"),1,m_FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("C"),1,m_FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("D"),1,m_FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("E"),1,m_FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+m_TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("F"),1,m_FLATSIZES);break;
	}
}

inline void DefaultPatternView::OutData4(CDC *devc,int x,int y,BYTE data, bool trflag)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+m_TEXTWIDTH;
	Rect.bottom=y+m_TEXTHEIGHT;

	
	if (trflag)
	{
		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,m_BlankParam,sizeof(m_BlankParam) / sizeof(TCHAR) - 1,m_FLATSIZES);

		return;
	}
	
	switch(data&0xf)
	{
	case 0x0: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("0"),1,m_FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("1"),1,m_FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("2"),1,m_FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("3"),1,m_FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("4"),1,m_FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("5"),1,m_FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("6"),1,m_FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("7"),1,m_FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("8"),1,m_FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("9"),1,m_FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("A"),1,m_FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("B"),1,m_FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("C"),1,m_FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("D"),1,m_FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("E"),1,m_FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("F"),1,m_FLATSIZES);break;
	}
}


inline void DefaultPatternView::BOX(CDC *devc,CRect rect)
{
	devc->Rectangle(rect);
}

inline void DefaultPatternView::BOX(CDC *devc,int x,int y, int w, int h)
{
	CRect rect;
	rect.left=x;
	rect.top=y;
	rect.right=x+w;
	rect.bottom=y+h;
	
	devc->Rectangle(rect);
}

inline void DefaultPatternView::TXTFLAT(CDC *devc,TCHAR *txt, int x,int y,int w,int h)
{
	RECT _Rect;
	_Rect.left=x;
	_Rect.top=y;
	_Rect.right=x+w;
	_Rect.bottom=y+h;
	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,(sizeof(txt) - 1) * sizeof(TCHAR),m_FLATSIZES);
//	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,-1,m_FLATSIZES);

}


inline void DefaultPatternView::TXT(CDC *devc,TCHAR *txt, int x,int y,int w,int h)
{
	RECT _Rect;
	_Rect.left=x;
	_Rect.top=y;
	_Rect.right=x+w;
	_Rect.bottom=y+h;
	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,(sizeof(txt) - 1) * sizeof(TCHAR),NULL);
//	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,-1,NULL);

}

inline int DefaultPatternView::_xtoCol(int pointpos)
{
	if ( pointpos < m_COLX[1] ) return 0;
	else if ( pointpos < m_COLX[2] ) return 1;
	else if ( pointpos < m_COLX[3] ) return 2;
	else if ( pointpos < m_COLX[4] ) return 3;
	else if ( pointpos < m_COLX[5] ) return 4;
	else if ( pointpos < m_COLX[6] ) return 5;
	else if ( pointpos < m_COLX[7] ) return 6;
	else if ( pointpos < m_COLX[8] ) return 7;
	else if ( pointpos < m_COLX[9] ) return 8;
	else if ( pointpos < m_COLX[10] ) return 9;
	else if ( pointpos < m_COLX[11] ) return 10;
	else if ( pointpos < m_COLX[12] ) return 11;
	else return 12;
/*	if ( pointpos < 28 ) return 0;
	else if ( pointpos < 40 ) return 1;
	else if ( pointpos < 48 ) return 2;
	else if ( pointpos < 60 ) return 3;
	else if ( pointpos < 70 ) return 4;
	else if ( pointpos < 83 ) return 5;
	else if ( pointpos < 91 ) return 6;
	else if ( pointpos < 100 ) return 7;
*/

}
