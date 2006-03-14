/** @file PsycleWTLView.h 
 *  @brief interface of the CPsycleWTLView class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma unmanaged

#define EDIT_COLUMNS_END 12
#define EDIT_COLUMNS_START 0

#include "Song.h"
#include "Instrument.h"
#include "Configuration.h"
#include "InputHandler.h"

#define MAX_DRAW_MESSAGES 32

class CMasterDlg;
#ifndef CONVERT_INTERNAL_MACHINES
class CGearPsychOsc;
class CGearDistort;
class CGearDelay;
class CGearfilter;
class CGearGainer;
class CGearFlanger;
#endif
class CWireDlg;
class CGearTracker;

namespace SF {
	struct ISongLoader;
	class XMSamplerUI;
}

#define MAX_WIRE_DIALOGS 16

enum 
{
	DMAll = 0,		// Repaints everything (means, slow). Used when switching views, or when a
					// whole update is needed (For example, when changing pattern Properties, or TPB)
	DMAllMacsRefresh, //Used to refresh all the machines, without refreshing the background/wires
	DMMacRefresh,	// Used to refresh the image of one machine (mac num in "updatePar")

	DMPattern,		// Use this when switching Patterns (changing from one to another)
	DMData,			// Data has Changed. Which data to update is indicated with DrawLineStart/End
					// and DrawTrackStart/End
					// Use it when editing and copy/pasting
	DMHScroll,		// Refresh called by the scrollbars or by mouse scrolling (when selecting).
					// New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);
	DMVScroll,		// Refresh called by the scrollbars or by mouse scrolling (when selecting).
					// New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);
//	DMResize,		// Indicates the Refresh is called from the "OnSize()" event.
	DMPlayback,		// Indicates it needs a refresh caused by Playback (update playback cursor)
	DMPlaybackChange,// Indicates that while playing, a pattern switch is needed.
	DMCursor,		// Indicates a movement of the cursor. update the values to "editcur" directly
					// and call this function.
					// this is arbitrary message as cursor is checked
	DMSelection,	// The selection has changed. use "blockSel" to indicate the values.
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

//#define DMPatternChange DMPatternSwitch // Remove when finishing the graphics update.

enum
{
	VMMachine,
	VMPattern
};


class CCursor
{
public:
	int line;
	TCHAR col;
	TCHAR track;

	CCursor() { line=0; col=0; track=0; }
};

class CSelection
{
public:
	CCursor start;	// Column not used. (It was harder to select!!!)
	CCursor end;	//
};

struct SPatternUndo
{
	int type;
	SPatternUndo* pPrev;
	BYTE* pData;
	int pattern;
	int x;
	int y;
	int	tracks;
	int	lines;
	// store positional data plz
	int edittrack;
	int editline;
	int editcol;
	int seqpos;
	// counter for tracking, works like ID
	int counter;
};

enum {
	UNDO_PATTERN,
	UNDO_LENGTH,
	UNDO_SEQUENCE,
	UNDO_SONG,
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

struct SMachineCoords
{
	SSkinSource sMaster;
	SSkinSource sGenerator;
	SSkinSource sGeneratorVu0;
	SSkinSource sGeneratorVuPeak;
	SSkinSource sGeneratorPan;
	SSkinSource sGeneratorMute;
	SSkinSource sGeneratorSolo;
	SSkinSource sEffect;
	SSkinSource sEffectVu0;
	SSkinSource sEffectVuPeak;
	SSkinSource sEffectPan;
	SSkinSource sEffectMute;
	SSkinSource sEffectBypass;
	SSkinSource dGeneratorVu;
	SSkinSource dGeneratorPan;
	SSkinDest dGeneratorMute;
	SSkinDest dGeneratorSolo;
	SSkinDest dGeneratorName;
	SSkinSource dEffectVu;
	SSkinSource dEffectPan;
	SSkinDest dEffectMute;
	SSkinDest dEffectBypass;
	SSkinDest dEffectName;
	BOOL bHasTransparency;
	COLORREF cTransparency;
};


/////////////////////////////////////////////////////////////////////////////
// CPsycleWTLView window
//typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS & ~WS_BORDER,WS_EX_CLIENTEDGE> CPsycleWTLViewTraits;
typedef CWinTraits<WS_CHILD | WS_VISIBLE ,WS_EX_CLIENTEDGE> CPsycleWTLViewTraits;
class CPsycleWTLView : public CWindowImpl<CPsycleWTLView,CWindow,CPsycleWTLViewTraits>
{
public:
	CPsycleWTLView();
	void Initialize();
	~CPsycleWTLView();
	DECLARE_WND_CLASS_EX(_T("CPsycleWTLView"),CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,NULL)

	void SelectMachineUnderCursor(void);
	BOOL CheckUnsavedSong(TCHAR* szTitle);

	void InitTimer();
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
    void SaveBlock(FILE* file);
	void LoadBlock(FILE* file);

	void DecCurPattern();
	void IncCurPattern();
	void IncPosition(bool bRepeat=false);
	void DecPosition();

	void AddUndo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=TRUE, int counter=0);
	void AddRedo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
	void AddUndoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=TRUE, int counter=0);
	void AddRedoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
	void AddUndoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=TRUE, int counter=0);
	void AddRedoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
	void AddUndoSong(int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=TRUE, int counter=0);
	void AddRedoSong(int edittrack, int editline, int editcol, int seqpos, int counter);

	void AddMacViewUndo(); // place holder

	void KillUndo();
	void KillRedo();
	void SelectNextTrack();  // for armed tracks recording
	void SetTitleBarText();
	void RecalculateColourGrid();
	void RecalcMetrics();
	void LoadPatternHeaderSkin();
	void LoadMachineSkin();
	void LoadMachineBackground();
	void KillWireDialogs();
	void patTrackMute();
	void patTrackSolo();
	void patTrackRecord();
	void KeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	void KeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	void NewMachine(int x = -1, int y = -1, int mac = -1);
	void DoMacPropDialog(int propMac);
	void CPsycleWTLView::FileLoadsongNamed(const TCHAR* fName);
	void UpdateUI();

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP_EX(CPsycleWTLView)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_KEYDOWN2(OnKeyDown)
		MSG_WM_KEYUP2(OnKeyUp)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_MBUTTONDOWN(OnMButtonDown)
		FORWARD_NOTIFICATIONS();
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	void OnLButtonUp( UINT nFlags, CPoint point );
	void OnMouseMove( UINT nFlags, CPoint point );
	void OnLButtonDblClk( UINT nFlags, CPoint point );

	void OnDestroy();
	void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	void OnBarplay();
	void OnBarplayFromStart();
	void OnBarrec();
	void OnBarstop();
	void OnRecordWav();
	void OnTimer( UINT nIDEvent,TIMERPROC pTimerProc = NULL );
//	void OnUpdateRecordWav(CCmdUI* pCmdUI);
	void OnFileNew();
	BOOL OnFileSave();
	BOOL OnFileSaveAs();
	void OnFileLoadsong();
	void OnFileRevert();
	void OnHelpSaludos();
//	void OnUpdatePatternView(CCmdUI* pCmdUI);
//	void OnUpdateMachineview(CCmdUI* pCmdUI);
//	void OnUpdateBarplay(CCmdUI* pCmdUI);
//	void OnUpdateBarplayFromStart(CCmdUI* pCmdUI);
//	void OnUpdateBarrec(CCmdUI* pCmdUI);
	void OnFileSongproperties();
	void OnViewInstrumenteditor();
	void OnNewmachine();
	void OnButtonplayseqblock();
//	void OnUpdateButtonplayseqblock(CCmdUI* pCmdUI);
	void OnPopCut();
//	void OnUpdateCutCopy(CCmdUI* pCmdUI);
	void OnPopCopy();
	void OnPopPaste();
//	void OnUpdatePaste(CCmdUI* pCmdUI);
	void OnPopMixpaste();
	void OnPopDelete();
	void OnPopInterpolate();
	void OnPopChangegenerator();
	void OnPopChangeinstrument();
	void OnPopTranspose1();
	void OnPopTranspose12();
	void OnPopTranspose_1();
	void OnPopTranspose_12();
	void OnAutostop();
//	void OnUpdateAutostop(CCmdUI* pCmdUI);
	void OnPopPattenproperties();
	void OnPopBlockSwingfill();
	void OnPopTrackSwingfill();
	void OnSize(UINT nType, CSize &size);
	void OnConfigurationSettings();
	void OnContextMenu(HWND hWnd, CPoint& point);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnFileRecent_01();
	void OnFileRecent_02();
	void OnFileRecent_03();
	void OnFileRecent_04();
	void OnEditUndo();
	void OnEditRedo();
//	void OnUpdateUndo(CCmdUI* pCmdUI);
//	void OnUpdateRedo(CCmdUI* pCmdUI);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnMButtonDown( UINT nFlags, CPoint& point );
//	void OnUpdatePatternCutCopy(CCmdUI* pCmdUI);
//	void OnUpdatePatternPaste(CCmdUI* pCmdUI);
	void OnFileSaveaudio();
	void OnHelpKeybtxt();
	void OnHelpReadme();
	void OnHelpTweaking();
	void OnHelpWhatsnew();
	void OnConfigurationLoopplayback();
//	void OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI);

	//	TCHAR m_appdir[_MAX_PATH];

	//RECENT!!!//
	void OnFileLoadsongNamed(TCHAR* fName, int fType);
	HMENU hRecentMenu;

	CMainFrame* pMainFrame;
	::Song* _pSong;
	bool useDoubleBuffer;
//	bool multiPattern;
	CMasterDlg * MasterMachineDialog;

	CGearTracker * SamplerMachineDialog;
	SF::XMSamplerUI * XMSamplerMachineDialog;
#ifndef CONVERT_INTERNAL_MACHINES
	CGearPsychOsc * PsychMachineDialog;
	CGearDistort * DistortionMachineDialog;
	CGearDelay * DelayMachineDialog;
	CGearfilter * FilterMachineDialog;
	CGearGainer * GainerMachineDialog;
	CGearFlanger * FlangerMachineDialog;
#endif
	CWireDlg * WireDialog[MAX_WIRE_DIALOGS];

	bool blockSelected;
	bool blockStart;
	bool bScrollDetatch;
	CCursor editcur;	// Edit Cursor Position in Pattern.
	CCursor detatchpoint;
	bool bEditMode;		// in edit mode?
	bool m_bPrepare;

	int patStep;

	int editPosition;	// Position in the Sequence!
	int prevEditPosition;

	int ChordModeOffs;
	int ChordModeLine;
	int ChordModeTrack;
	int updateMode;
	int updatePar;			// VMPattern: Display update mode. VMMachine: Machine number to update.
	int viewMode;
	int XOFFSET;
	int YOFFSET;
	int ROWWIDTH;
	int ROWHEIGHT; // textheight+1
	int TEXTWIDTH;
	int TEXTHEIGHT;
	int HEADER_INDENT;
	int HEADER_HEIGHT;
	int HEADER_ROWWIDTH;
	int CH;
	int CW;
	int VISTRACKS;
	int COLX[EDIT_COLUMNS_END + 2];
	bool _outputActive;	// This variable indicates if the output (audio or midi) is active or not.
						// Its function is to prevent audio (and midi) operations while it is not
						// initialized, or while song is being modified (New(),Load()..).
						// 

	SPatternHeaderCoords PatHeaderCoords;
	SMachineCoords	MachineCoords;

private:
		//Recent Files!!!!//
	void AppendToRecent(const TCHAR* fName);
	void CallOpenRecent(int pos);
	
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
	void DrawMachineVol(int c, CDC *devc);
	void DrawMachineVumeters(int c, CDC *devc);	
	void DrawAllMachineVumeters(CDC *devc);	
	void DrawMachineEditor(CDC *devc);
	void DrawMachine(int macnum, CDC *devc);
	void ClearMachineSpace(int macnum, CDC *devc);
	void amosDraw(CDC *devc, int oX,int oY,int dX,int dY);
	int GetMachine(CPoint point);
	void NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd);
	void RecalculateColour(COLORREF* pDest, COLORREF source1, COLORREF source2);
	COLORREF ColourDiffAdd(COLORREF base, COLORREF adjust, COLORREF add);
	void FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result);
	void FindMachineSkin(CString findDir, CString findName, BOOL *result);
	void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
	void TransparentBlt(CDC* pDC, int xStart,  int yStart, int wWidth,  int wHeight, CDC* pTmpDC, CBitmap* bmpMask, int xSource = 0, int ySource = 0);

	inline int _ps();
	inline unsigned char * _ptrack(int ps, int track);
	inline unsigned char * _ptrack(int ps);
	inline unsigned char * _ptrack();
	inline unsigned char * _ptrackline(int ps, int track, int line);
	inline unsigned char * _ptrackline(int ps);
	inline unsigned char * _ptrackline();
	inline unsigned char * _ppattern(int ps);
	inline unsigned char * _ppattern();
	inline int _xtoCol(int pointpos);

	void SetSongDirAndFileName(const TCHAR *  pFilePath,const TCHAR *  extention = _T(".psy"));
	void InitializeBeforeLoading();
	void ProcessAfterLoading();

private:
	// GDI Stuff
	CBitmap patternheader;
	CBitmap patternheadermask;
	HBITMAP hbmPatHeader;
	WTL::CBitmap machineskin;
	CBitmap machineskinmask;
	CBitmap machinebkg;
	HBITMAP hbmMachineSkin;
	HBITMAP hbmMachineBkg;
	CBitmap* bmpDC;
	int FLATSIZES[256];
	int VISLINES;

	int bkgx;
	int bkgy;

	int triangle_size_tall;
	int triangle_size_center;
	int triangle_size_wide;
	
	int playpos;		// Play Cursor Position in Screen // left and right are unused
	int newplaypos;		// Play Cursor Position in Screen that is gonna be drawn.
	CRect selpos;		// Selection Block in Screen
	CRect newselpos;	// Selection Block in Screen that is gonna be drawn.
	CCursor editlast;	// Edit Cursor Position in Pattern.

	SPatternDraw pPatternDraw[MAX_DRAW_MESSAGES];
	int numPatternDraw;

	// Enviroment variables
	int smac;
	int smacmode;
	int wiresource;
	int wiredest;
	int wiremove;
	int wireSX;
	int wireSY;
	int wireDX;
	int wireDY;

	int maxt;		// num of tracks shown
	int maxl;		// num of lines shown
	int tOff;		// Track Offset (first track shown)
	int lOff;		// Line Offset (first line shown)
	int ntOff;		// These two variables are used for the DMScroll functino
	int nlOff;
	int rntOff;
	int rnlOff;

	TCHAR szBlankParam[2];
	TCHAR szBlankNote[4];

	CCursor iniSelec;
	CSelection blockSel;
	CCursor oldm;	// Indicates the previous track/line/col when selecting (used for mouse)
	CPoint MBStart; 

	bool isBlockCopied;
	BYTE blockBufferData[5*MAX_LINES*MAX_TRACKS];
	int	blockNTracks;
	int	blockNLines;
	
	BYTE patBufferData[5*MAX_LINES*MAX_TRACKS];
	int patBufferLines;
	bool patBufferCopy;

	SPatternUndo * pUndoList;
	SPatternUndo * pRedoList;

	int UndoCounter;
	int UndoSaved;

	int UndoMacCounter;
	int UndoMacSaved;

	int mcd_x;
	int mcd_y;

	COLORREF pvc_separator[MAX_TRACKS+1];
	COLORREF pvc_background[MAX_TRACKS+1];
	COLORREF pvc_row4beat[MAX_TRACKS+1];
	COLORREF pvc_rowbeat[MAX_TRACKS+1];
	COLORREF pvc_row[MAX_TRACKS+1];
	COLORREF pvc_selection[MAX_TRACKS+1];
	COLORREF pvc_playbar[MAX_TRACKS+1];
	COLORREF pvc_cursor[MAX_TRACKS+1];
	COLORREF pvc_font[MAX_TRACKS+1];
	COLORREF pvc_fontPlay[MAX_TRACKS+1];
	COLORREF pvc_fontCur[MAX_TRACKS+1];
	COLORREF pvc_fontSel[MAX_TRACKS+1];
	COLORREF pvc_selectionbeat[MAX_TRACKS+1];
	COLORREF pvc_selection4beat[MAX_TRACKS+1];

	DWORD m_elapsedTime;///< デバッグ用経過時間 

	//
	typedef std::map<SF::string,SF::ISongLoader *> SongLoaderMap;
	SongLoaderMap m_SongLoaderMap; 
	// String Resource
	SF::CResourceString m_FmtPan;
	SF::CResourceString m_FmtPan1;

public:
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
//	LRESULT OnNcRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//////////////////////////////////////////////////////////////////////
// Pattern data display functions
inline void CPsycleWTLView::OutNote(CDC *devc,int x,int y,int note)
{
	int const srx = (TEXTWIDTH * 3) + 1;
	int const sry = TEXTHEIGHT;
	
	switch(note)
	{
	case 255: TXTFLAT(devc,szBlankNote,x,y,srx,sry);break;
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

inline void CPsycleWTLView::OutData(CDC *devc,int x,int y,BYTE data, bool trflag)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+TEXTWIDTH;
	Rect.bottom=y+TEXTHEIGHT;
	
	if (trflag)
	{
		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,sizeof(szBlankParam) / sizeof(TCHAR) - 1,FLATSIZES);

		Rect.left+=TEXTWIDTH; 
		Rect.right+=TEXTWIDTH;
		devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,sizeof(szBlankParam) / sizeof(TCHAR) - 1,FLATSIZES);


//		Rect.right+=10;
//		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("  "),FLATSIZES);
		return;
	}
	
	switch(data>>4)
	{
	case 0x0: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("0"),1,FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("1"),1,FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("2"),1,FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("3"),1,FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("4"),1,FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("5"),1,FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("6"),1,FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("7"),1,FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("8"),1,FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("9"),1,FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("A"),1,FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("B"),1,FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("C"),1,FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("D"),1,FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("E"),1,FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("F"),1,FLATSIZES);break;
	}
	
	Rect.left+=TEXTWIDTH;
	Rect.right+=TEXTWIDTH;
	
	switch(data&0xf)
	{
	case 0x0: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("0"),1,FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("1"),1,FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("2"),1,FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("3"),1,FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("4"),1,FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("5"),1,FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("6"),1,FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("7"),1,FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("8"),1,FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("9"),1,FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("A"),1,FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("B"),1,FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("C"),1,FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("D"),1,FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("E"),1,FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("F"),1,FLATSIZES);break;
	}
}

inline void CPsycleWTLView::OutData4(CDC *devc,int x,int y,BYTE data, bool trflag)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+TEXTWIDTH;
	Rect.bottom=y+TEXTHEIGHT;

	
	if (trflag)
	{
		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,sizeof(szBlankParam) / sizeof(TCHAR) - 1,FLATSIZES);

		return;
	}
	
	switch(data&0xf)
	{
	case 0x0: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("0"),1,FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("1"),1,FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("2"),1,FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("3"),1,FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("4"),1,FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("5"),1,FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("6"),1,FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("7"),1,FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("8"),1,FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("9"),1,FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("A"),1,FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("B"),1,FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("C"),1,FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("D"),1,FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("E"),1,FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,_T("F"),1,FLATSIZES);break;
	}
}


inline void CPsycleWTLView::BOX(CDC *devc,CRect rect)
{
	devc->Rectangle(rect);
}

inline void CPsycleWTLView::BOX(CDC *devc,int x,int y, int w, int h)
{
	CRect rect;
	rect.left=x;
	rect.top=y;
	rect.right=x+w;
	rect.bottom=y+h;
	
	devc->Rectangle(rect);
}

inline void CPsycleWTLView::TXTFLAT(CDC *devc,TCHAR *txt, int x,int y,int w,int h)
{
	RECT _Rect;
	_Rect.left=x;
	_Rect.top=y;
	_Rect.right=x+w;
	_Rect.bottom=y+h;
	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,(sizeof(txt) - 1) * sizeof(TCHAR),FLATSIZES);
//	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,-1,FLATSIZES);

}


inline void CPsycleWTLView::TXT(CDC *devc,TCHAR *txt, int x,int y,int w,int h)
{
	RECT _Rect;
	_Rect.left=x;
	_Rect.top=y;
	_Rect.right=x+w;
	_Rect.bottom=y+h;
	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,(sizeof(txt) - 1) * sizeof(TCHAR),NULL);
//	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,&_Rect,txt,-1,NULL);

}

// song data

inline int CPsycleWTLView::_ps()
{
	// retrieves the pattern index
	return _pSong->PlayOrder(editPosition);
}

// ALWAYS USE THESE MACROS BECAUSE THEY TEST TO SEE IF THE PATTERN HAS BEEN ALLOCATED!
// if you don't you might get an exception!

inline BYTE * CPsycleWTLView::_ptrack(int ps, int track)
{
	return _pSong->_ptrack(ps,track);
}	

inline BYTE * CPsycleWTLView::_ptrack(int ps)
{
	return _pSong->_ptrack(ps,editcur.track);
}	

inline BYTE * CPsycleWTLView::_ptrack()
{
	return _pSong->_ptrack(_ps(),editcur.track);
}	

inline BYTE * CPsycleWTLView::_ptrackline(int ps, int track, int line)
{
	return _pSong->_ptrackline(ps,track,line);
}

inline BYTE * CPsycleWTLView::_ptrackline(int ps)
{
	return _pSong->_ptrackline(ps,editcur.track,editcur.line);
}

inline BYTE * CPsycleWTLView::_ptrackline()
{
	return _pSong->_ptrackline(_ps(),editcur.track,editcur.line);
}

inline BYTE * CPsycleWTLView::_ppattern(int ps)
{
	return _pSong->_ppattern(ps);
}

inline BYTE * CPsycleWTLView::_ppattern()
{
	return _pSong->_ppattern(_ps());
}


inline int CPsycleWTLView::_xtoCol(int pointpos)
{
	if ( pointpos < COLX[1] ) return 0;
	else if ( pointpos < COLX[2] ) return 1;
	else if ( pointpos < COLX[3] ) return 2;
	else if ( pointpos < COLX[4] ) return 3;
	else if ( pointpos < COLX[5] ) return 4;
	else if ( pointpos < COLX[6] ) return 5;
	else if ( pointpos < COLX[7] ) return 6;
	else if ( pointpos < COLX[8] ) return 7;
	else if ( pointpos < COLX[9] ) return 8;
	else if ( pointpos < COLX[10] ) return 9;
	else if ( pointpos < COLX[11] ) return 10;
	else if ( pointpos < COLX[12] ) return 11;
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
