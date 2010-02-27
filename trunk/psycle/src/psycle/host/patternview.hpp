// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include "Psycle.hpp"

#include "MachineGui.hpp" ///\todo this shouldn't be here, it is for SSkin.

#include <psycle/core/machine.h>
#include <psycle/core/pattern.h>

#define MAX_DRAW_MESSAGES 32

namespace psycle { namespace host {
	
class CMainFrame;
class CChildView;

struct CCursor {
	CCursor() : line(0), col(0), track(0) {}
	int line;
	char col;
	char track;
};

struct CSelection {
	CCursor start;	// Column not used. (It was harder to select!!!)
	CCursor end;	//
};

struct SPatternDraw {
	int drawTrackStart;
	int drawTrackEnd;
	int drawLineStart;
	int drawLineEnd;
};

struct SPatternHeaderCoords {		
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


class PatternView {
	public:

		struct draw_modes { enum draw_mode {
			all, ///< Repaints everything (means, slow). Used when switching views, or when a
			///< whole update is needed (For example, when changing pattern Properties, or TPB)

			all_machines, ///< Used to refresh all the machines, without refreshing the background/wires

			machine, ///< Used to refresh the image of one machine (mac num in "updatePar")

			pattern, ///< Use this when switching Patterns (changing from one to another)

			data, ///< Data has Changed. Which data to update is indicated with DrawLineStart/End
			///< and DrawTrackStart/End
			///< Use it when editing and copy/pasting

			horizontal_scroll, ///< Refresh called by the scrollbars or by mouse scrolling (when selecting).
			///< New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);

			vertical_scroll, ///< Refresh called by the scrollbars or by mouse scrolling (when selecting).
			///< New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);

			//resize, ///< Indicates the Refresh is called from the "OnSize()" event.

			playback, ///< Indicates it needs a refresh caused by Playback (update playback cursor)

			playback_change, ///< Indicates that while playing, a pattern switch is needed.

			cursor, ///< Indicates a movement of the cursor. update the values to "editcur" directly
			///< and call this function.
			///< this is arbitrary message as cursor is checked

			selection, ///< The selection has changed. use "blockSel" to indicate the values.

			track_header, ///< Track header refresh (mute/solo, Record updating)

			//pattern_header, ///< Octave, Pattern name, Edit Mode on/off

			none ///< Do not use this one directly. It is used to detect refresh calls from the OS.

			// If you add any new method, please, add the proper code to "PreparePatternRefresh()" and to
			// "DrawPatternEditor()".
			// Note: Modes are sorted by priority. (although it is not really used)

			// !!!BIG ADVISE!!! : The execution of Repaint(mode) does not imply an instant refresh of
			//						the Screen, and what's worse, you might end calling Repaint(anothermode)
			//						previous of the first refresh. In PreparePatternRefresh() there's code
			//						to avoid problems when two modes do completely different things. On
			//						other cases, it still ends to wrong content being shown.
		};};

		PatternView(class Project* project);
		~PatternView() {}

		void SetParent(CChildView* parent, CMainFrame* main) {
			parent_ = parent;
			main_ = main;
		}

		Pattern* pattern() { return pattern_; }
		void SetPattern(Pattern* pattern) {
			pattern_ = pattern;
		}

		class Project* project();
		class Project* project() const;			
		CChildView* child_view() { return parent_; }
		CMainFrame* main() { return main_; }
		Song* song();

		psycle::core::Pattern::iterator GetEventOnCursor();
		psycle::core::Pattern::iterator GetEventOnPos(double pos, int track);

		void Draw(CDC *devc, const CRgn& rgn);
		void OnSize(UINT nType, int cx, int cy);
		// key events
		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		bool OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		// mouse events
		void OnLButtonDown( UINT nFlags, CPoint point );
		void OnRButtonDown( UINT nFlags, CPoint point );
		void OnRButtonUp( UINT nFlags, CPoint point );
		void OnLButtonUp( UINT nFlags, CPoint point );
		void OnMouseMove( UINT nFlags, CPoint point );
		void OnLButtonDblClk( UINT nFlags, CPoint point );
		void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		void OnMButtonDown( UINT nFlags, CPoint point );
		void OnContextMenu(CWnd* pWnd, CPoint point);
		void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
					

		void ShowTransformPatternDlg(void);
		void ShowPatternDlg(void);
		void BlockInsChange(int x);
		void BlockGenChange(int x);
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
		void SelectNextTrack(); 
		
		void PlayCurrentRow();
		void PlayCurrentNote();
		void patCopy();
		void patPaste();
		void patMixPaste();
		void patCut();
		void patDelete();
		void patTranspose(int trp);
		void HalveLength();
		void DoubleLength();
		void BlockTranspose(int trp);
		void BlockParamInterpolate(int *points=0,int twktype=notecommands::empty);
		void StartBlock(int track,int line, int col);
		void ChangeBlock(int track,int line, int col);	// This function allows a handier usage for Shift+Arrows and MouseSelection
											// Params: current track, line and col
											// Result: Update the selected region depending on the new and old values.
		void EndBlock(int track,int line, int col);
		void CopyBlock(bool cutit);
		void PasteBlock(int tx,int lx,bool mix,bool save=true);
		void SwitchBlock(int tx, int lx);
		void DeleteBlock();
		void BlockUnmark(void);
		void SaveBlock(FILE* file);
		void LoadBlock(FILE* file);			
		void DecPosition();
		void SelectMachineUnderCursor(void);
		void EnterData(UINT nChar,UINT nFlags);

		bool CheckUnsavedSong();
		void LoadPatternHeaderSkin();
		void FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result);
		void RecalcMetrics();
		void PreparePatternRefresh(int drawMode);
		void RecalculateColourGrid();

		void PerformCmd(class CmdDef &cmd, BOOL brepeat);
		void patTrackMute();
		void patTrackSolo();
		void patTrackRecord();

		void Stop();
		void PlaySong();
		void PlayFromCur();

		void OnUpdateUndo(CCmdUI* pCmdUI);
		void OnUpdateRedo(CCmdUI* pCmdUI);
		void OnPopMixpaste();
		void OnPopBlockswitch();
		void OnPopPaste();
		void OnPopInterpolateCurve();

		void Repaint(draw_modes::draw_mode drawMode);

		//todo should be private
		bool isBlockCopied;
		bool patBufferCopy;
		int tOff;		// Track Offset (first track shown)

	private:
		void DrawPatEditor(CDC *devc);
		void DrawPatternData(CDC *devc,int tstart,int tend, int lstart, int lend);
		void NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd);			
		void TransparentBlt(CDC* pDC, int xStart,  int yStart, int wWidth,  int wHeight, CDC* pTmpDC, CBitmap* bmpMask, int xSource = 0, int ySource = 0);
		void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);						
		void RecalculateColour(COLORREF* pDest, COLORREF source1, COLORREF source2);
		COLORREF ColourDiffAdd(COLORREF base, COLORREF adjust, COLORREF add);
		

		CChildView* parent_;
		CMainFrame* main_;			

		bool bFT2HomeBehaviour;
		bool bShiftArrowsDoSelect;
		bool bDoingSelection;
		int maxt;		// num of tracks shown
		int maxl;		// num of lines shown
		int lOff;		// Line Offset (first line shown)
		int ntOff;		// These two variables are used for the DMScroll functino
		int nlOff;
		int rntOff;
		int rnlOff;
		// GDI Stuff
		CBitmap patternheader;
		CBitmap patternheadermask;
		HBITMAP hbmPatHeader;						
		int FLATSIZES[256];

	public:
		bool blockSelected;
		bool blockStart;
		bool blockswitch;
		int blockSelectBarState; //This is used to remember the state of the select bar function
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
		int updatePar;			// view_modes::pattern: Display update mode. view_modes::machine: Machine number to update.			
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
		int VISLINES;
		int COLX[10];

		SPatternHeaderCoords PatHeaderCoords;
		SMachineCoords	MachineCoords;

		bool maxView;	//maximise pattern state
		int textLeftEdge;



		int playpos;		// Play Cursor Position in Screen // left and right are unused
		int newplaypos;		// Play Cursor Position in Screen that is gonna be drawn.
		CRect selpos;		// Selection Block in Screen
		CRect newselpos;	// Selection Block in Screen that is gonna be drawn.
		CCursor editlast;	// Edit Cursor Position in Pattern.

		SPatternDraw pPatternDraw[MAX_DRAW_MESSAGES];
		int numPatternDraw;


		char szBlankParam[2];
		char szBlankNote[4];

		CCursor iniSelec;
		CSelection blockSel;
		CCursor oldm;	// Indicates the previous track/line/col when selecting (used for mouse)
		CPoint MBStart; 

	private:
		Project* project_;
		Pattern block_buffer_pattern_;
		Pattern* pattern_;
		double line_pos(int line, bool nearest) const;
		int	blockNTracks;
		int	blockNLines;
		CSelection blockLastOrigin;		
		int patBufferLines;

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

		// Pattern data display functions
		inline void OutNote(CDC *devc,int x,int y,int note) {
			int const srx=(TEXTWIDTH*3)+1;
			int const sry=TEXTHEIGHT;
			
			switch(note) {
				case notecommands::empty: TXTFLAT(devc,szBlankNote,x,y,srx,sry);break;
				//case 255: TXTFLAT(devc,"   ",x,y,srx,sry);break;
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
				case notecommands::release: TXTFLAT(devc,"off",x,y,srx,sry);break;
				case notecommands::tweak: TXTFLAT(devc,"twk",x,y,srx,sry);break;
				case notecommands::midi_cc: TXTFLAT(devc,"mcm" /* aka "mcc" or "cmd"? */,x,y,srx,sry);break;
				case notecommands::tweak_slide: TXTFLAT(devc,"tws",x,y,srx,sry);break;
			}
		}

		void OutData(CDC *devc, int x, int y, unsigned char data, bool trflag) {
			CRect Rect;
			Rect.left = x;
			Rect.top = y;
			Rect.right = x + TEXTWIDTH;
			Rect.bottom = y + TEXTHEIGHT;
			
			if(trflag) {
				devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,FLATSIZES);
				Rect.left+=TEXTWIDTH; 
				Rect.right+=TEXTWIDTH;
				devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,FLATSIZES);
				return;
			}
			
			switch(data >> 4) {
				case 0x0: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"0",FLATSIZES);break;
				case 0x1: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"1",FLATSIZES);break;
				case 0x2: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"2",FLATSIZES);break;
				case 0x3: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"3",FLATSIZES);break;
				case 0x4: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"4",FLATSIZES);break;
				case 0x5: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"5",FLATSIZES);break;
				case 0x6: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"6",FLATSIZES);break;
				case 0x7: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"7",FLATSIZES);break;
				case 0x8: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"8",FLATSIZES);break;
				case 0x9: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"9",FLATSIZES);break;
				case 0xA: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"A",FLATSIZES);break;
				case 0xB: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"B",FLATSIZES);break;
				case 0xC: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"C",FLATSIZES);break;
				case 0xD: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"D",FLATSIZES);break;
				case 0xE: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"E",FLATSIZES);break;
				case 0xF: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"F",FLATSIZES);break;
			}
			
			Rect.left += TEXTWIDTH;
			Rect.right += TEXTWIDTH;
			
			switch(data & 0xf) {
				case 0x0: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"0",FLATSIZES);break;
				case 0x1: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"1",FLATSIZES);break;
				case 0x2: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"2",FLATSIZES);break;
				case 0x3: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"3",FLATSIZES);break;
				case 0x4: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"4",FLATSIZES);break;
				case 0x5: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"5",FLATSIZES);break;
				case 0x6: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"6",FLATSIZES);break;
				case 0x7: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"7",FLATSIZES);break;
				case 0x8: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"8",FLATSIZES);break;
				case 0x9: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"9",FLATSIZES);break;
				case 0xA: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"A",FLATSIZES);break;
				case 0xB: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"B",FLATSIZES);break;
				case 0xC: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"C",FLATSIZES);break;
				case 0xD: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"D",FLATSIZES);break;
				case 0xE: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"E",FLATSIZES);break;
				case 0xF: devc->ExtTextOut(x+TEXTWIDTH+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"F",FLATSIZES);break;
			}
		}

		void OutData4(CDC *devc,int x,int y,unsigned char data, bool trflag) {
			CRect Rect;
			Rect.left = x;
			Rect.top = y;
			Rect.right = x + TEXTWIDTH;
			Rect.bottom = y + TEXTHEIGHT;
			
			if(trflag) {
				devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,szBlankParam,FLATSIZES);
				return;
			}
			
			switch(data & 0xf) {
				case 0x0: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"0",FLATSIZES);break;
				case 0x1: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"1",FLATSIZES);break;
				case 0x2: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"2",FLATSIZES);break;
				case 0x3: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"3",FLATSIZES);break;
				case 0x4: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"4",FLATSIZES);break;
				case 0x5: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"5",FLATSIZES);break;
				case 0x6: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"6",FLATSIZES);break;
				case 0x7: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"7",FLATSIZES);break;
				case 0x8: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"8",FLATSIZES);break;
				case 0x9: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"9",FLATSIZES);break;
				case 0xA: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"A",FLATSIZES);break;
				case 0xB: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"B",FLATSIZES);break;
				case 0xC: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"C",FLATSIZES);break;
				case 0xD: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"D",FLATSIZES);break;
				case 0xE: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"E",FLATSIZES);break;
				case 0xF: devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,"F",FLATSIZES);break;
			}
		}

		inline void BOX(CDC *devc,CRect rect) { devc->Rectangle(rect); }

		inline void BOX(CDC *devc,int x,int y, int w, int h) {
			CRect rect;
			rect.left=x;
			rect.top=y;
			rect.right=x+w;
			rect.bottom=y+h;
			
			devc->Rectangle(rect);
		}

		void TXTFLAT(CDC *devc,char const *txt, int x,int y,int w,int h) {
			CRect Rect;
			Rect.left=x;
			Rect.top=y;
			Rect.right=x+w;
			Rect.bottom=y+h;
			devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,txt,FLATSIZES);
		}

		void TXT(CDC *devc,char const *txt, int x,int y,int w,int h) {
			CRect Rect;
			Rect.left=x;
			Rect.top=y;
			Rect.right=x+w;
			Rect.bottom=y+h;
			devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,txt,NULL);
		}

		int _xtoCol(int pointpos) {
			if ( pointpos < COLX[1] ) return 0;
			else if ( pointpos < COLX[2] ) return 1;
			else if ( pointpos < COLX[3] ) return 2;
			else if ( pointpos < COLX[4] ) return 3;
			else if ( pointpos < COLX[5] ) return 4;
			else if ( pointpos < COLX[6] ) return 5;
			else if ( pointpos < COLX[7] ) return 6;
			else if ( pointpos < COLX[8] ) return 7;
			else return 8;
		}

		bool InRect(int _x,int _y,SSkinDest _src,SSkinSource _src2,int _offs=0) {
			return (_x >= _offs+_src.x) && (_x < _offs+_src.x+_src2.width) && 
				(_y >= _src.y) && (_y < _src.y+_src2.height);
		}
};

}}
