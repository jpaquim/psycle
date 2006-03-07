/** @file PsycleWTLView.h 
 *  @brief interface of the MachineView class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.2 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define EDIT_COLUMNS_END 12
#define EDIT_COLUMNS_START 0

#include "Song.h"
#include "Instrument.h"
#include "configuration.h"
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

namespace configuration {
	class MachineView : public iCompornentConfiguration
	{
	public:
		MachineView(){
			m_draw_mac_index = TRUE;
			m_draw_vus = TRUE;
			
			_tcscpy(m_generator_fontface,_T("MS UI Gothic"));
			m_generator_font_point = 90;
			_tcscpy(m_effect_fontface,_T("MS UI Gothic"));
			m_effect_font_point = 90;

			pattern_font_flags = 0;
			m_generator_font_flags = 0;
			m_effect_font_flags = 0;

			_tcscpy(m_machine_skin,DEFAULT_MACHINE_SKIN);

			m_colour =	0x009a887c;
			m_wirecolour =	0x00000000;
			m_polycolour =	0x00ffffff;
			m_m_generator_fontcolour = 0x00000000;
			m_m_effect_fontcolour = 0x00000000;
			m_wireaa = 1;
			m_triangle_size = 22;
			m_wirewidth = 1;
			m_wireaacolour = ((((m_wirecolour&0x00ff0000) + ((m_colour&0x00ff0000)*4))/5)&0x00ff0000) +
					((((m_wirecolour&0x00ff00) + ((m_colour&0x00ff00)*4))/5)&0x00ff00) +
					((((m_wirecolour&0x00ff) + ((m_colour&0x00ff)*4))/5)&0x00ff);

			m_wireaacolour2 = (((((m_wirecolour&0x00ff0000)) + ((m_colour&0x00ff0000)))/2)&0x00ff0000) +
					(((((m_wirecolour&0x00ff00)) + ((m_colour&0x00ff00)))/2)&0x00ff00) +
					(((((m_wirecolour&0x00ff)) + ((m_colour&0x00ff)))/2)&0x00ff);

		};

		~MachineView();
	
		/** Read Config File **/
		void Read(const string& settingFileName);
		/** Read Default Config**/
		void Read();
		
		/** Config File Read **/
		void Write(const string& settingFileName);
		/** Read Default Config **/
		void Write();
	private:
		COLORREF m_colour;
		COLORREF m_wirecolour;
		COLORREF m_wireaacolour;
		COLORREF m_wireaacolour2;
		COLORREF m_polycolour;
		COLORREF m_m_generator_fontcolour;
		COLORREF m_m_effect_fontcolour;
	
		int m_wireaa;
		int m_wirewidth;
		int m_triangle_size;

		bool m_bBmpBkg;
		TCHAR m_BmpBkgFilename[MAX_PATH];

		UINT m_m_generator_font_flags;
		bool m_draw_mac_index;
		bool m_draw_vus;

		TCHAR m_generator_fontface[64];
		int m_generator_font_point;
		TCHAR m_effect_fontface[64];
		int m_effect_font_point;

		TCHAR m_machine_skin[64];

		CFont m_GeneratorFont;
		CFont m_EffectFont;

	}

};


/////////////////////////////////////////////////////////////////////////////
// MachineView window
//typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS & ~WS_BORDER,WS_EX_CLIENTEDGE> MachineViewTraits;
typedef CWinTraits<WS_CHILD | WS_VISIBLE ,WS_EX_CLIENTEDGE> MachineViewTraits;
class MachineView : public CWindowImpl<MachineView,CWindow,MachineViewTraits>
{
public:
	DECLARE_WND_CLASS_EX(_T("MachineView"),CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,NULL)
	
	MachineView();///< コンストラクタ
	void Initialize();///< 初期化
	~MachineView();///< デストラクタ

	void SelectMachineUnderCursor(void);

	void ValidateParent();
	void EnableSound();
	void Repaint(int drawMode=DMAll);

	void KillUndo();
	void KillRedo();

	void LoadMachineSkin();
	void LoadMachineBackground();
	void KillWireDialogs();
	void NewMachine(int x = -1, int y = -1, int mac = -1);
	void DoMacPropDialog(int propMac);
	void UpdateUI();
	void DrawAllMachineVumeters(CDC *devc);	

	BOOL PreTranslateMessage(MSG* pMsg);

	void AddUndo(SF::IUndo * const pUndo,const bool bWipeRedo = false){
		m_UndoController.AddUndo(pUndo,bWipeRedo);
		SetTitleBarText();
	};

	void AddRedo(SF::IUndo * const pUndo){
		m_UndoController.AddRedo(pUndo);
	};

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

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
	void OnPopDelete();
	void OnPopChangegenerator();
	void OnPopPattenproperties();

	void OnSize(UINT nType, CSize &size);
	void OnContextMenu(HWND hWnd, CPoint& point);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);

	void OnEditUndo();
	void OnEditRedo();

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnMButtonDown( UINT nFlags, CPoint& point );

	const ::Song& Song(){return *_pSong;}; 

	void MachineView::ShowMachineGui(const int tmac, const CPoint point);
	void CloseMacGui(const int mac,const bool closewiredialogs = true);
	void CloseAllMacGuis();

private:
	
	inline void BOX(CDC *devc,int x,int y, int w, int h);
	inline void BOX(CDC *devc,CRect rect);

	void DrawMachineVol(int c, CDC *devc);
	void DrawMachineVumeters(int c, CDC *devc);	
	void DrawMachineEditor(CDC *devc);
	void DrawMachine(int macnum, CDC *devc);
	void ClearMachineSpace(int macnum, CDC *devc);
	void amosDraw(CDC *devc, int oX,int oY,int dX,int dY);
	int GetMachine(CPoint point);
	void RecalculateColour(COLORREF* pDest, COLORREF source1, COLORREF source2);
	COLORREF ColourDiffAdd(COLORREF base, COLORREF adjust, COLORREF add);
	void FindMachineSkin(CString findDir, CString findName, BOOL *result);
	void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
	void TransparentBlt(CDC* pDC, int xStart,  int yStart, int wWidth,  int wHeight, CDC* pTmpDC, CBitmap* bmpMask, int xSource = 0, int ySource = 0);

	CMainFrame* pMainFrame;
	Song* _pSong;

	bool useDoubleBuffer;
//	bool multiPattern;
	CMasterDlg * MasterMachineDialog;

	CGearTracker * SamplerMachineDialog;
	SF::XMSamplerUI * XMSamplerMachineDialog;

	bool m_bGUIOpen[MAX_MACHINES];
	CWindow *m_pWndMac[MAX_MACHINES];
	bool m_bMacComboInitialized;


#ifndef CONVERT_INTERNAL_MACHINES
	CGearPsychOsc * PsychMachineDialog;
	CGearDistort * DistortionMachineDialog;
	CGearDelay * DelayMachineDialog;
	CGearfilter * FilterMachineDialog;
	CGearGainer * GainerMachineDialog;
	CGearFlanger * FlangerMachineDialog;
#endif

	CWireDlg * m_WireDialog[MAX_WIRE_DIALOGS];

	bool bEditMode;		// in edit mode?

	bool m_bPrepare;

	int CH;
	int CW;

	SMachineCoords	MachineCoords;

	// GDI Stuff
	WTL::CBitmap machineskin;
	CBitmap machineskinmask;
	CBitmap machinebkg;
	HBITMAP hbmMachineSkin;
	HBITMAP hbmMachineBkg;
	CBitmap* bmpDC;

	int bkgx;
	int bkgy;

	int triangle_size_tall;
	int triangle_size_center;
	int triangle_size_wide;
	
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

	int UndoMacCounter;
	int UndoMacSaved;

	int mcd_x;
	int mcd_y;

	DWORD m_elapsedTime;///< デバッグ用経過時間 


	// String Resource
	SF::CResourceString m_FmtPan;
	SF::CResourceString m_FmtPan1;

	SF::UndoController m_UndoController;

public:
	BEGIN_MSG_MAP_EX(MachineView)
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


inline void MachineView::BOX(CDC *devc,CRect rect)
{
	devc->Rectangle(rect);
}

inline void MachineView::BOX(CDC *devc,int x,int y, int w, int h)
{
	CRect rect;
	rect.left=x;
	rect.top=y;
	rect.right=x+w;
	rect.bottom=y+h;
	
	devc->Rectangle(rect);
}


