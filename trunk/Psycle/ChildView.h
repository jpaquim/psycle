// ChildView.h : interface of the CChildView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDVIEW_H__FE0D36EA_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_CHILDVIEW_H__FE0D36EA_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Song.h"
#include "Configuration.h"
#include "InputHandler.h"

#define MAX_DRAW_MESSAGES 32

class CMasterDlg;
class CGearPsychOsc;
class CGearDistort;
class CGearTracker;
class CGearDelay;
class CGearfilter;
class CGearGainer;
class CGearFlanger;
class CWireDlg;

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
	char col;
	char track;

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
	unsigned char* pData;
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
// CChildView window

class CChildView : public CWnd
{

public:
	CChildView();
	virtual ~CChildView();

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
	void KillUndo();
	void KillRedo();
	void SelectNextTrack();  // for armed tracks recording
	void SetTitleBarText();
	void RecalculateColourGrid();
	void RecalcMetrics();
	void LoadPatternHeaderSkin();
	void LoadMachineSkin();
	void KillWireDialogs();
	void patTrackMute();
	void patTrackSolo();
	void patTrackRecord();
	void KeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	void KeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	void NewMachine(int x = -1, int y = -1, int mac = -1);
	void DoMacPropDialog(int propMac);
	void CChildView::FileLoadsongNamed(char* fName);
	
public:

//	char m_appdir[_MAX_PATH];

	//RECENT!!!//
	void OnFileLoadsongNamed(char* fName, int fType);
	HMENU hRecentMenu;

	CFrameWnd* pParentFrame;
	Song* _pSong;
	bool useDoubleBuffer;
//	bool multiPattern;
	CMasterDlg * MasterMachineDialog;
	CGearPsychOsc * PsychMachineDialog;
	CGearDistort * DistortionMachineDialog;
	CGearTracker * SamplerMachineDialog;
	CGearDelay * DelayMachineDialog;
	CGearfilter * FilterMachineDialog;
	CGearGainer * GainerMachineDialog;
	CGearFlanger * FlangerMachineDialog;
	CWireDlg * WireDialog[MAX_WIRE_DIALOGS];

	bool blockSelected;
	bool blockStart;
	bool bScrollDetatch;
	CCursor editcur;	// Edit Cursor Position in Pattern.
	CCursor detatchpoint;
	bool bEditMode;		// in edit mode?
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
	int COLX[10];
	bool _outputActive;	// This variable indicates if the output (audio or midi) is active or not.
						// Its function is to prevent audio (and midi) operations while it is not
						// initialized, or while song is being modified (New(),Load()..).
						// 

	SPatternHeaderCoords PatHeaderCoords;
	SMachineCoords	MachineCoords;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

//////////////////////////////////////////////////////////////////////
// Private operations

private:

	//Recent Files!!!!//
	void AppendToRecent(char* fName);
	void CallOpenRecent(int pos);
	
	//Recent Files!!!!//

	void PreparePatternRefresh(int drawMode);
	void DrawPatEditor(CDC *devc);
	void DrawPatternData(CDC *devc,int tstart,int tend, int lstart, int lend);
//	void DrawMultiPatternData(CDC *devc,int tstart,int tend, int lstart, int lend);
	inline void OutNote(CDC *devc,int x,int y,int note);
	inline void OutData(CDC *devc,int x,int y,unsigned char data,bool trflag);
	inline void OutData4(CDC *devc,int x,int y,unsigned char data,bool trflag);
	inline void TXT(CDC *devc,char *txt, int x,int y,int w,int h);
	inline void TXTFLAT(CDC *devc,char *txt, int x,int y,int w,int h);
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


private:
	// GDI Stuff
	CBitmap patternheader;
	CBitmap patternheadermask;
	HBITMAP hbmPatHeader;
	CBitmap machineskin;
	CBitmap machineskinmask;
	HBITMAP hbmMachineSkin;
	CBitmap* bmpDC;
	int FLATSIZES[256];
	int VISLINES;
	
	int playpos;		// Play Cursor Position in Screen // left and right are unused
	int newplaypos;		// Play Cursor Position in Screen that is gonna be drawn.
	CRect selpos;		// Selection Block in Screen
	CRect newselpos;	// Selection Block in Screen that is gonna be drawn.
	CCursor editlast;	// Edit Cursor Position in Pattern.

	SPatternDraw pPatternDraw[MAX_DRAW_MESSAGES];
	int numPatternDraw;

	int maxt;		// num of tracks shown
	int maxl;		// num of lines shown
	int tOff;		// Track Offset (first track shown)
	int lOff;		// Line Offset (first line shown)
	int ntOff;		// These two variables are used for the DMScroll functino
	int nlOff;
	int rntOff;
	int rnlOff;

	char szBlankParam[2];
	char szBlankNote[4];

	CCursor iniSelec;
	CSelection blockSel;
	CCursor oldm;	// Indicates the previous track/line/col when selecting (used for mouse)
	CPoint MBStart; 

	bool isBlockCopied;
	unsigned char blockBufferData[5*MAX_LINES*MAX_TRACKS];
	int	blockNTracks;
	int	blockNLines;
	
	unsigned char patBufferData[5*MAX_LINES*MAX_TRACKS];
	int patBufferLines;
	bool patBufferCopy;

	SPatternUndo * pUndoList;
	SPatternUndo * pRedoList;

	int UndoCounter;
	int UndoSaved;

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


public:
	
	void SelectMachineUnderCursor(void);
	BOOL CheckUnsavedSong(char* szTitle);
	// Generated message map functions
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg void OnHelpPsycleenviromentinfo();
	afx_msg void OnMidiMonitorDlg();
	afx_msg void OnDestroy();
	afx_msg void OnAppExit();
	afx_msg void OnMachineview();
	afx_msg void OnPatternView();
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnBarplay();
	afx_msg void OnBarplayFromStart();
	afx_msg void OnBarrec();
	afx_msg void OnBarstop();
	afx_msg void OnRecordWav();
	afx_msg void OnTimer( UINT nIDEvent );
	afx_msg void OnUpdateRecordWav(CCmdUI* pCmdUI);
	afx_msg void OnFileNew();
	afx_msg BOOL OnFileSave(UINT id);
	afx_msg BOOL OnFileSaveAs(UINT id);
	afx_msg void OnFileLoadsong();
	afx_msg void OnFileRevert();
	afx_msg void OnHelpSaludos();
	afx_msg void OnUpdatePatternView(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMachineview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBarplay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBarplayFromStart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBarrec(CCmdUI* pCmdUI);
	afx_msg void OnFileSongproperties();
	afx_msg void OnViewInstrumenteditor();
	afx_msg void OnNewmachine();
	afx_msg void OnButtonplayseqblock();
	afx_msg void OnUpdateButtonplayseqblock(CCmdUI* pCmdUI);
	afx_msg void OnPopCut();
	afx_msg void OnUpdateCutCopy(CCmdUI* pCmdUI);
	afx_msg void OnPopCopy();
	afx_msg void OnPopPaste();
	afx_msg void OnUpdatePaste(CCmdUI* pCmdUI);
	afx_msg void OnPopMixpaste();
	afx_msg void OnPopDelete();
	afx_msg void OnPopInterpolate();
	afx_msg void OnPopChangegenerator();
	afx_msg void OnPopChangeinstrument();
	afx_msg void OnPopTranspose1();
	afx_msg void OnPopTranspose12();
	afx_msg void OnPopTranspose_1();
	afx_msg void OnPopTranspose_12();
	afx_msg void OnAutostop();
	afx_msg void OnUpdateAutostop(CCmdUI* pCmdUI);
	afx_msg void OnPopPattenproperties();
	afx_msg void OnPopBlockSwingfill();
	afx_msg void OnPopTrackSwingfill();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnConfigurationSettings();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFileImportXmfile();
	afx_msg void OnFileRecent_01();
	afx_msg void OnFileRecent_02();
	afx_msg void OnFileRecent_03();
	afx_msg void OnFileRecent_04();
	afx_msg void OnFileImportItfile();
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRedo(CCmdUI* pCmdUI);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnUpdatePatternCutCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePatternPaste(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveaudio();
	afx_msg void OnHelpKeybtxt();
	afx_msg void OnHelpReadme();
	afx_msg void OnHelpTweaking();
	afx_msg void OnHelpWhatsnew();
	afx_msg void OnConfigurationLoopplayback();
	afx_msg void OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Pattern data display functions
inline void CChildView::OutNote(CDC *devc,int x,int y,int note)
{
	int const srx=(TEXTWIDTH*3)+1;
	int const sry=TEXTHEIGHT;
	
	switch(note)
	{
	case 255: TXTFLAT(devc,szBlankNote,x,y,srx,sry);break;
//	case 255: TXTFLAT(devc,"   ",x,y,srx,sry);break;
	case 0:   TXTFLAT(devc,"C-0",x,y,srx,sry);break;
	case 1:   TXTFLAT(devc,"C#0",x,y,srx,sry);break;
	case 2:   TXTFLAT(devc,"D-0",x,y,srx,sry);break;
	case 3:   TXTFLAT(devc,"D#0",x,y,srx,sry);break;
	case 4:   TXTFLAT(devc,"E-0",x,y,srx,sry);break;
	case 5:   TXTFLAT(devc,"F-0",x,y,srx,sry);break;
	case 6:   TXTFLAT(devc,"F#0",x,y,srx,sry);break;
	case 7:   TXTFLAT(devc,"G-0",x,y,srx,sry);break;
	case 8:   TXTFLAT(devc,"G#0",x,y,srx,sry);break;
	case 9:   TXTFLAT(devc,"A-0",x,y,srx,sry);break;
	case 10:  TXTFLAT(devc,"A#0",x,y,srx,sry);break;
	case 11:  TXTFLAT(devc,"B-0",x,y,srx,sry);break;
	case 12:  TXTFLAT(devc,"C-1",x,y,srx,sry);break;
	case 13:  TXTFLAT(devc,"C#1",x,y,srx,sry);break;
	case 14:  TXTFLAT(devc,"D-1",x,y,srx,sry);break;
	case 15:  TXTFLAT(devc,"D#1",x,y,srx,sry);break;
	case 16:  TXTFLAT(devc,"E-1",x,y,srx,sry);break;
	case 17:  TXTFLAT(devc,"F-1",x,y,srx,sry);break;
	case 18:  TXTFLAT(devc,"F#1",x,y,srx,sry);break;
	case 19:  TXTFLAT(devc,"G-1",x,y,srx,sry);break;
	case 20:  TXTFLAT(devc,"G#1",x,y,srx,sry);break;
	case 21:  TXTFLAT(devc,"A-1",x,y,srx,sry);break;
	case 22:  TXTFLAT(devc,"A#1",x,y,srx,sry);break;
	case 23:  TXTFLAT(devc,"B-1",x,y,srx,sry);break;
	case 24:  TXTFLAT(devc,"C-2",x,y,srx,sry);break;
	case 25:  TXTFLAT(devc,"C#2",x,y,srx,sry);break;
	case 26:  TXTFLAT(devc,"D-2",x,y,srx,sry);break;
	case 27:  TXTFLAT(devc,"D#2",x,y,srx,sry);break;
	case 28:  TXTFLAT(devc,"E-2",x,y,srx,sry);break;
	case 29:  TXTFLAT(devc,"F-2",x,y,srx,sry);break;
	case 30:  TXTFLAT(devc,"F#2",x,y,srx,sry);break;
	case 31:  TXTFLAT(devc,"G-2",x,y,srx,sry);break;
	case 32:  TXTFLAT(devc,"G#2",x,y,srx,sry);break;
	case 33:  TXTFLAT(devc,"A-2",x,y,srx,sry);break;
	case 34:  TXTFLAT(devc,"A#2",x,y,srx,sry);break;
	case 35:  TXTFLAT(devc,"B-2",x,y,srx,sry);break;
	case 36:  TXTFLAT(devc,"C-3",x,y,srx,sry);break;
	case 37:  TXTFLAT(devc,"C#3",x,y,srx,sry);break;
	case 38:  TXTFLAT(devc,"D-3",x,y,srx,sry);break;
	case 39:  TXTFLAT(devc,"D#3",x,y,srx,sry);break;
	case 40:  TXTFLAT(devc,"E-3",x,y,srx,sry);break;
	case 41:  TXTFLAT(devc,"F-3",x,y,srx,sry);break;
	case 42:  TXTFLAT(devc,"F#3",x,y,srx,sry);break;
	case 43:  TXTFLAT(devc,"G-3",x,y,srx,sry);break;
	case 44:  TXTFLAT(devc,"G#3",x,y,srx,sry);break;
	case 45:  TXTFLAT(devc,"A-3",x,y,srx,sry);break;
	case 46:  TXTFLAT(devc,"A#3",x,y,srx,sry);break;
	case 47:  TXTFLAT(devc,"B-3",x,y,srx,sry);break;
	case 48:  TXTFLAT(devc,"C-4",x,y,srx,sry);break;
	case 49:  TXTFLAT(devc,"C#4",x,y,srx,sry);break;
	case 50:  TXTFLAT(devc,"D-4",x,y,srx,sry);break;
	case 51:  TXTFLAT(devc,"D#4",x,y,srx,sry);break;
	case 52:  TXTFLAT(devc,"E-4",x,y,srx,sry);break;
	case 53:  TXTFLAT(devc,"F-4",x,y,srx,sry);break;
	case 54:  TXTFLAT(devc,"F#4",x,y,srx,sry);break;
	case 55:  TXTFLAT(devc,"G-4",x,y,srx,sry);break;
	case 56:  TXTFLAT(devc,"G#4",x,y,srx,sry);break;
	case 57:  TXTFLAT(devc,"A-4",x,y,srx,sry);break;
	case 58:  TXTFLAT(devc,"A#4",x,y,srx,sry);break;
	case 59:  TXTFLAT(devc,"B-4",x,y,srx,sry);break;
	case 60:  TXTFLAT(devc,"C-5",x,y,srx,sry);break;
	case 61:  TXTFLAT(devc,"C#5",x,y,srx,sry);break;
	case 62:  TXTFLAT(devc,"D-5",x,y,srx,sry);break;
	case 63:  TXTFLAT(devc,"D#5",x,y,srx,sry);break;
	case 64:  TXTFLAT(devc,"E-5",x,y,srx,sry);break;
	case 65:  TXTFLAT(devc,"F-5",x,y,srx,sry);break;
	case 66:  TXTFLAT(devc,"F#5",x,y,srx,sry);break;
	case 67:  TXTFLAT(devc,"G-5",x,y,srx,sry);break;
	case 68:  TXTFLAT(devc,"G#5",x,y,srx,sry);break;
	case 69:  TXTFLAT(devc,"A-5",x,y,srx,sry);break;
	case 70:  TXTFLAT(devc,"A#5",x,y,srx,sry);break;
	case 71:  TXTFLAT(devc,"B-5",x,y,srx,sry);break;
	case 72:  TXTFLAT(devc,"C-6",x,y,srx,sry);break;
	case 73:  TXTFLAT(devc,"C#6",x,y,srx,sry);break;
	case 74:  TXTFLAT(devc,"D-6",x,y,srx,sry);break;
	case 75:  TXTFLAT(devc,"D#6",x,y,srx,sry);break;
	case 76:  TXTFLAT(devc,"E-6",x,y,srx,sry);break;
	case 77:  TXTFLAT(devc,"F-6",x,y,srx,sry);break;
	case 78:  TXTFLAT(devc,"F#6",x,y,srx,sry);break;
	case 79:  TXTFLAT(devc,"G-6",x,y,srx,sry);break;
	case 80:  TXTFLAT(devc,"G#6",x,y,srx,sry);break;
	case 81:  TXTFLAT(devc,"A-6",x,y,srx,sry);break;
	case 82:  TXTFLAT(devc,"A#6",x,y,srx,sry);break;
	case 83:  TXTFLAT(devc,"B-6",x,y,srx,sry);break;
	case 84:  TXTFLAT(devc,"C-7",x,y,srx,sry);break;
	case 85:  TXTFLAT(devc,"C#7",x,y,srx,sry);break;
	case 86:  TXTFLAT(devc,"D-7",x,y,srx,sry);break;
	case 87:  TXTFLAT(devc,"D#7",x,y,srx,sry);break;
	case 88:  TXTFLAT(devc,"E-7",x,y,srx,sry);break;
	case 89:  TXTFLAT(devc,"F-7",x,y,srx,sry);break;
	case 90:  TXTFLAT(devc,"F#7",x,y,srx,sry);break;
	case 91:  TXTFLAT(devc,"G-7",x,y,srx,sry);break;
	case 92:  TXTFLAT(devc,"G#7",x,y,srx,sry);break;
	case 93:  TXTFLAT(devc,"A-7",x,y,srx,sry);break;
	case 94:  TXTFLAT(devc,"A#7",x,y,srx,sry);break;
	case 95:  TXTFLAT(devc,"B-7",x,y,srx,sry);break;
	case 96:  TXTFLAT(devc,"C-8",x,y,srx,sry);break;
	case 97:  TXTFLAT(devc,"C#8",x,y,srx,sry);break;
	case 98:  TXTFLAT(devc,"D-8",x,y,srx,sry);break;
	case 99:  TXTFLAT(devc,"D#8",x,y,srx,sry);break;
	case 100: TXTFLAT(devc,"E-8",x,y,srx,sry);break;
	case 101: TXTFLAT(devc,"F-8",x,y,srx,sry);break;
	case 102: TXTFLAT(devc,"F#8",x,y,srx,sry);break;
	case 103: TXTFLAT(devc,"G-8",x,y,srx,sry);break;
	case 104: TXTFLAT(devc,"G#8",x,y,srx,sry);break;
	case 105: TXTFLAT(devc,"A-8",x,y,srx,sry);break;
	case 106: TXTFLAT(devc,"A#8",x,y,srx,sry);break;
	case 107: TXTFLAT(devc,"B-8",x,y,srx,sry);break;
	case 108: TXTFLAT(devc,"C-9",x,y,srx,sry);break;
	case 109: TXTFLAT(devc,"C#9",x,y,srx,sry);break;
	case 110: TXTFLAT(devc,"D-9",x,y,srx,sry);break;
	case 111: TXTFLAT(devc,"D#9",x,y,srx,sry);break;
	case 112: TXTFLAT(devc,"E-9",x,y,srx,sry);break;
	case 113: TXTFLAT(devc,"F-9",x,y,srx,sry);break;
	case 114: TXTFLAT(devc,"F#9",x,y,srx,sry);break;
	case 115: TXTFLAT(devc,"G-9",x,y,srx,sry);break;
	case 116: TXTFLAT(devc,"G#9",x,y,srx,sry);break;
	case 117: TXTFLAT(devc,"A-9",x,y,srx,sry);break;
	case 118: TXTFLAT(devc,"A#9",x,y,srx,sry);break;
	case 119: TXTFLAT(devc,"B-9",x,y,srx,sry);break;
	case 120: TXTFLAT(devc,"off",x,y,srx,sry);break;
	case cdefTweakM: TXTFLAT(devc,"twk",x,y,srx,sry);break;
	case cdefTweakE: TXTFLAT(devc,"twf",x,y,srx,sry);break;
	case cdefMIDICC: TXTFLAT(devc,"Mcm",x,y,srx,sry);break;
	case cdefTweakS: TXTFLAT(devc,"tws",x,y,srx,sry);break;
	}
}

inline void CChildView::OutData(CDC *devc,int x,int y,unsigned char data, bool trflag)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+TEXTWIDTH;
	Rect.bottom=y+TEXTHEIGHT;
	
	if (trflag)
	{
		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,FLATSIZES);
		Rect.left+=TEXTWIDTH; 
		Rect.right+=TEXTWIDTH;
		devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,FLATSIZES);

//		Rect.right+=10;
//		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"  ",FLATSIZES);
		return;
	}
	
	switch(data>>4)
	{
	case 0x0: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"0",FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"1",FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"2",FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"3",FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"4",FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"5",FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"6",FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"7",FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"8",FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"9",FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"A",FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"B",FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"C",FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"D",FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"E",FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"F",FLATSIZES);break;
	}
	
	Rect.left+=TEXTWIDTH;
	Rect.right+=TEXTWIDTH;
	
	switch(data&0xf)
	{
	case 0x0: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"0",FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"1",FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"2",FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"3",FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"4",FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"5",FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"6",FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"7",FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"8",FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"9",FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"A",FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"B",FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"C",FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"D",FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"E",FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+TEXTWIDTH+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"F",FLATSIZES);break;
	}
}

inline void CChildView::OutData4(CDC *devc,int x,int y,unsigned char data, bool trflag)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+TEXTWIDTH;
	Rect.bottom=y+TEXTHEIGHT;
	
	if (trflag)
	{
		devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,FLATSIZES);
		return;
	}
	
	switch(data&0xf)
	{
	case 0x0: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"0",FLATSIZES);break;
	case 0x1: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"1",FLATSIZES);break;
	case 0x2: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"2",FLATSIZES);break;
	case 0x3: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"3",FLATSIZES);break;
	case 0x4: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"4",FLATSIZES);break;
	case 0x5: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"5",FLATSIZES);break;
	case 0x6: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"6",FLATSIZES);break;
	case 0x7: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"7",FLATSIZES);break;
	case 0x8: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"8",FLATSIZES);break;
	case 0x9: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"9",FLATSIZES);break;
	case 0xA: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"A",FLATSIZES);break;
	case 0xB: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"B",FLATSIZES);break;
	case 0xC: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"C",FLATSIZES);break;
	case 0xD: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"D",FLATSIZES);break;
	case 0xE: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"E",FLATSIZES);break;
	case 0xF: devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"F",FLATSIZES);break;
	}
}


inline void CChildView::BOX(CDC *devc,CRect rect)
{
	devc->Rectangle(rect);
}

inline void CChildView::BOX(CDC *devc,int x,int y, int w, int h)
{
	CRect rect;
	rect.left=x;
	rect.top=y;
	rect.right=x+w;
	rect.bottom=y+h;
	
	devc->Rectangle(rect);
}

inline void CChildView::TXTFLAT(CDC *devc,char *txt, int x,int y,int w,int h)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+w;
	Rect.bottom=y+h;
	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,txt,FLATSIZES);
}

inline void CChildView::TXT(CDC *devc,char *txt, int x,int y,int w,int h)
{
	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+w;
	Rect.bottom=y+h;
	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,txt,NULL);
}

// song data

inline int CChildView::_ps()
{
	// retrieves the pattern index
	return _pSong->playOrder[editPosition];
}

// ALWAYS USE THESE MACROS BECAUSE THEY TEST TO SEE IF THE PATTERN HAS BEEN ALLOCATED!
// if you don't you might get an exception!

inline unsigned char * CChildView::_ptrack(int ps, int track)
{
	return _pSong->_ptrack(ps,track);
}	

inline unsigned char * CChildView::_ptrack(int ps)
{
	return _pSong->_ptrack(ps,editcur.track);
}	

inline unsigned char * CChildView::_ptrack()
{
	return _pSong->_ptrack(_ps(),editcur.track);
}	

inline unsigned char * CChildView::_ptrackline(int ps, int track, int line)
{
	return _pSong->_ptrackline(ps,track,line);
}

inline unsigned char * CChildView::_ptrackline(int ps)
{
	return _pSong->_ptrackline(ps,editcur.track,editcur.line);
}

inline unsigned char * CChildView::_ptrackline()
{
	return _pSong->_ptrackline(_ps(),editcur.track,editcur.line);
}

inline unsigned char * CChildView::_ppattern(int ps)
{
	return _pSong->_ppattern(ps);
}

inline unsigned char * CChildView::_ppattern()
{
	return _pSong->_ppattern(_ps());
}


inline int CChildView::_xtoCol(int pointpos)
{
	if ( pointpos < COLX[1] ) return 0;
	else if ( pointpos < COLX[2] ) return 1;
	else if ( pointpos < COLX[3] ) return 2;
	else if ( pointpos < COLX[4] ) return 3;
	else if ( pointpos < COLX[5] ) return 4;
	else if ( pointpos < COLX[6] ) return 5;
	else if ( pointpos < COLX[7] ) return 6;
	else if ( pointpos < COLX[8] ) return 7;
	else return 8;
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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDVIEW_H__FE0D36EA_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
