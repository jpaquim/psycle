#if !defined(AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
#define AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_

/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// SkinDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg dialog




class CSkinDlg : public CPropertyPageImpl<CSkinDlg>
//class CSkinDlg : public CDialogImpl<CSkinDlg>
{
	//DECLARE_DYNCREATE(CSkinDlg)

// Construction
public:
	COLORREF _patternViewColor;
	COLORREF _patternViewColor2;
	COLORREF _patternSeparatorColor;
	COLORREF _patternSeparatorColor2;
	COLORREF _playbarColor;
	COLORREF _playbarColor2;
	COLORREF _selectionColor;
	COLORREF _selectionColor2;
	COLORREF _cursorColor;
	COLORREF _cursorColor2;
	COLORREF _fontColor;
	COLORREF _fontColor2;
	COLORREF _fontColorPlay;
	COLORREF _fontColorPlay2;
	COLORREF _fontColorCur;
	COLORREF _fontColorCur2;
	COLORREF _fontColorSel;
	COLORREF _fontColorSel2;
	COLORREF _rowColor;
	COLORREF _rowColor2;
	COLORREF _beatColor;
	COLORREF _beatColor2;
	COLORREF _4beatColor;
	COLORREF _4beatColor2;
	COLORREF _machineViewColor;
	COLORREF _machineViewWireColor;
	COLORREF _machineViewPolyColor;
	COLORREF _machineViewGeneratorFontColor;
	COLORREF _machineViewEffectFontColor;
	COLORREF _vubColor;
	COLORREF _vugColor;
	COLORREF _vucColor;
	bool _gfxbuffer;
	int _wireaa;
	bool _linenumbers;
	bool _linenumbersHex;
	bool _linenumbersCursor;
	int _wirewidth;
	TCHAR _pattern_fontface[64];
	TCHAR _pattern_header_skin[64];
	int _pattern_font_point;
	int _pattern_font_x;
	int _pattern_font_y;
	int _triangle_size;
	bool _pattern_draw_empty_data;
	bool _draw_mac_index;
	bool _draw_vus;

	UINT _pattern_font_flags;
	UINT _effect_font_flags;
	UINT _generator_font_flags;

	TCHAR _machine_skin[64];
	TCHAR _generator_fontface[64];
	int _generator_font_point;
	TCHAR _effect_fontface[64];
	int _effect_font_point;

	CSkinDlg();
	~CSkinDlg();

	TCHAR _skinPathBuf[MAX_PATH];

	bool bBmpBkg;
	TCHAR szBmpBkgFilename[MAX_PATH];

//BEGIN_MESSAGE_MAP(CSkinDlg, CPropertyPage)
	BEGIN_MSG_MAP(CSkinDlg)
/*	//{{AFX_MSG_MAP(CSkinDlg)
	ON_BN_CLICKED(IDC_BG_COLOUR, OnColourMachine)
	ON_BN_CLICKED(IDC_WIRE_COLOUR, OnColourWire)
	ON_BN_CLICKED(IDC_POLY_COLOUR, OnColourPoly)
	ON_BN_CLICKED(IDC_BUTTON23, OnVuBarColor)
	ON_BN_CLICKED(IDC_BUTTON24, OnVuBackColor)
	ON_BN_CLICKED(IDC_BUTTON25, OnVuClipBar)
	ON_BN_CLICKED(IDC_PATTERNBACKC, OnButtonPattern)
	ON_BN_CLICKED(IDC_PATTERNBACKC2, OnButtonPattern2)
	ON_BN_CLICKED(IDC_PATSEPARATORC, OnButtonPatternSeparator)
	ON_BN_CLICKED(IDC_PATSEPARATORC2, OnButtonPatternSeparator2)
	ON_BN_CLICKED(IDC_ROWC, OnRowc)
	ON_BN_CLICKED(IDC_ROWC2, OnRowc2)
	ON_BN_CLICKED(IDC_FONTC, OnFontc)
	ON_BN_CLICKED(IDC_FONTC2, OnFontc2)
	ON_BN_CLICKED(IDC_FONTSELC, OnFontSelc)
	ON_BN_CLICKED(IDC_FONTSELC2, OnFontSelc2)
	ON_BN_CLICKED(IDC_FONTPLAYC, OnFontPlayc)
	ON_BN_CLICKED(IDC_FONTPLAYC2, OnFontPlayc2)
	ON_BN_CLICKED(IDC_FONTCURSORC, OnFontCursorc)
	ON_BN_CLICKED(IDC_FONTCURSORC2, OnFontCursorc2)
	ON_BN_CLICKED(IDC_BEATC, OnBeatc)
	ON_BN_CLICKED(IDC_BEATC2, OnBeatc2)
	ON_BN_CLICKED(IDC_4BEAT, On4beat)
	ON_BN_CLICKED(IDC_4BEAT2, On4beat2)
	ON_BN_CLICKED(IDC_PLAYBARC, OnPlaybar)
	ON_BN_CLICKED(IDC_PLAYBARC2, OnPlaybar2)
	ON_BN_CLICKED(IDC_SELECTIONC, OnSelection)
	ON_BN_CLICKED(IDC_SELECTIONC2, OnSelection2)
	ON_BN_CLICKED(IDC_CURSORC, OnCursor)
	ON_BN_CLICKED(IDC_CURSORC2, OnCursor2)
	ON_BN_CLICKED(IDC_DOUBLEBUFFER, OnDoublebuffer)
	ON_BN_CLICKED(IDC_CHECK_VUS, OnCheckVus)
	ON_BN_CLICKED(IDC_LINE_NUMBERS, OnLineNumbers)
	ON_BN_CLICKED(IDC_LINE_NUMBERS_HEX, OnLineNumbersHex)
	ON_BN_CLICKED(IDC_LINE_NUMBERS_CURSOR, OnLineNumbersCursor)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_IMPORTREG, OnImportReg)
	ON_BN_CLICKED(IDC_EXPORTREG, OnExportReg)
	ON_CBN_SELCHANGE(IDC_PATTERN_FONT_POINT, OnSelchangePatternFontPoint)
	ON_CBN_SELCHANGE(IDC_PATTERN_FONT_X, OnSelchangePatternFontX)
	ON_CBN_SELCHANGE(IDC_PATTERN_FONT_Y, OnSelchangePatternFontY)
	ON_BN_CLICKED(IDC_PATTERN_FONTFACE, OnPatternFontFace)
	ON_CBN_SELCHANGE(IDC_PATTERN_HEADER_SKIN, OnSelchangePatternHeaderSkin)
	ON_CBN_SELCHANGE(IDC_WIRE_WIDTH, OnSelchangeWireWidth)
	ON_CBN_SELCHANGE(IDC_MACHINE_SKIN, OnSelchangeMachineSkin)
	ON_CBN_SELCHANGE(IDC_WIREAA, OnSelchangeWireAA)
	ON_CBN_SELCHANGE(IDC_MACHINE_FONT_POINT, OnSelchangeGeneratorFontPoint)
	ON_BN_CLICKED(IDC_MACHINE_FONTFACE, OnGeneratorFontFace)
	ON_BN_CLICKED(IDC_MV_FONT_COLOUR, OnMVGeneratorFontColour)
	ON_CBN_SELCHANGE(IDC_MACHINE_FONT_POINT2, OnSelchangeEffectFontPoint)
	ON_BN_CLICKED(IDC_MACHINE_FONTFACE2, OnEffectFontFace)
	ON_BN_CLICKED(IDC_MV_FONT_COLOUR2, OnMVEffectFontColour)
	ON_BN_CLICKED(IDC_DRAW_EMPTY_DATA, OnDrawEmptyData)
	ON_BN_CLICKED(IDC_DRAW_MAC_INDEX, OnDrawMacIndex)
	ON_BN_CLICKED(IDC_MACHINE_BITMAP, OnMachineBitmap)
	ON_CBN_SELCHANGE(IDC_TRIANGLESIZE, OnSelchangeTrianglesize)

	//}}AFX_MSG_MAP
*/
	COMMAND_HANDLER(IDC_DRAW_EMPTY_DATA, BN_CLICKED, OnBnClickedDrawEmptyData)
	COMMAND_HANDLER(IDC_PATTERN_FONTFACE, BN_CLICKED, OnBnClickedPatternFontface)
	COMMAND_HANDLER(IDC_PATTERN_FONT_POINT, CBN_SELCHANGE, OnCbnSelchangePatternFontPoint)
	COMMAND_HANDLER(IDC_PATTERN_FONT_X, CBN_SELCHANGE, OnCbnSelchangePatternFontX)
	COMMAND_HANDLER(IDC_PATTERN_FONT_Y, CBN_SELCHANGE, OnCbnSelchangePatternFontY)
	COMMAND_HANDLER(IDC_PATTERN_HEADER_SKIN, CBN_SELCHANGE, OnCbnSelchangePatternHeaderSkin)
	COMMAND_HANDLER(IDC_MACHINE_FONT_POINT, CBN_SELCHANGE, OnCbnSelchangeMachineFontPoint)
	COMMAND_HANDLER(IDC_MACHINE_FONT_POINT2, CBN_SELCHANGE, OnCbnSelchangeMachineFontPoint2)
	COMMAND_HANDLER(IDC_WIRE_WIDTH, CBN_SELCHANGE, OnCbnSelchangeWireWidth)
	COMMAND_HANDLER(IDC_WIREAA, CBN_SELCHANGE, OnCbnSelchangeWireaa)
	COMMAND_HANDLER(IDC_TRIANGLESIZE, CBN_SELCHANGE, OnCbnSelchangeTrianglesize)
	COMMAND_HANDLER(IDC_LINE_NUMBERS, BN_CLICKED, OnBnClickedLineNumbers)
	COMMAND_HANDLER(IDC_LINE_NUMBERS_CURSOR, BN_CLICKED, OnBnClickedLineNumbersCursor)
	COMMAND_HANDLER(IDC_LINE_NUMBERS_HEX, BN_CLICKED, OnBnClickedLineNumbersHex)
	COMMAND_HANDLER(IDC_FONTC, BN_CLICKED, OnBnClickedFontc)
	COMMAND_HANDLER(IDC_FONTC2, BN_CLICKED, OnBnClickedFontc2)
	COMMAND_HANDLER(IDC_FONTPLAYC, BN_CLICKED, OnBnClickedFontplayc)
	COMMAND_HANDLER(IDC_FONTPLAYC2, BN_CLICKED, OnBnClickedFontplayc2)
	COMMAND_HANDLER(IDC_FONTSELC, BN_CLICKED, OnBnClickedFontselc)
	COMMAND_HANDLER(IDC_FONTSELC2, BN_CLICKED, OnBnClickedFontselc2)
	COMMAND_HANDLER(IDC_FONTCURSORC, BN_CLICKED, OnBnClickedFontcursorc)
	COMMAND_HANDLER(IDC_FONTCURSORC2, BN_CLICKED, OnBnClickedFontcursorc2)
	COMMAND_HANDLER(IDC_PATSEPARATORC, BN_CLICKED, OnBnClickedPatseparatorc)
	COMMAND_HANDLER(IDC_PATSEPARATORC2, BN_CLICKED, OnBnClickedPatseparatorc2)
	COMMAND_HANDLER(IDC_PATTERNBACKC, BN_CLICKED, OnBnClickedPatternbackc)
	COMMAND_HANDLER(IDC_PATTERNBACKC2, BN_CLICKED, OnBnClickedPatternbackc2)
	COMMAND_HANDLER(IDC_ROWC, BN_CLICKED, OnBnClickedRowc)
	COMMAND_HANDLER(IDC_ROWC2, BN_CLICKED, OnBnClickedRowc2)
	COMMAND_HANDLER(IDC_BEATC, BN_CLICKED, OnBnClickedBeatc)
	COMMAND_HANDLER(IDC_BEATC2, BN_CLICKED, OnBnClickedBeatc2)
	COMMAND_HANDLER(IDC_4BEAT, BN_CLICKED, OnBnClicked4beat)
	COMMAND_HANDLER(IDC_4BEAT2, BN_CLICKED, OnBnClicked4beat2)
	COMMAND_HANDLER(IDC_PLAYBARC, BN_CLICKED, OnBnClickedPlaybarc)
	COMMAND_HANDLER(IDC_PLAYBARC2, BN_CLICKED, OnBnClickedPlaybarc2)
	COMMAND_HANDLER(IDC_SELECTIONC, BN_CLICKED, OnBnClickedSelectionc)
	COMMAND_HANDLER(IDC_SELECTIONC2, BN_CLICKED, OnBnClickedSelectionc2)
	COMMAND_HANDLER(IDC_CURSORC, BN_CLICKED, OnBnClickedCursorc)
	COMMAND_HANDLER(IDC_CURSORC2, BN_CLICKED, OnBnClickedCursorc2)
	COMMAND_HANDLER(IDC_MACHINE_FONTFACE, BN_CLICKED, OnBnClickedMachineFontface)
	COMMAND_HANDLER(IDC_MV_FONT_COLOUR, BN_CLICKED, OnBnClickedMvFontColour)
	COMMAND_HANDLER(IDC_MACHINE_FONTFACE2, BN_CLICKED, OnBnClickedMachineFontface2)
	COMMAND_HANDLER(IDC_MV_FONT_COLOUR2, BN_CLICKED, OnBnClickedMvFontColour2)
	COMMAND_HANDLER(IDC_BG_COLOUR, BN_CLICKED, OnBnClickedBgColour)
	COMMAND_HANDLER(IDC_WIRE_COLOUR, BN_CLICKED, OnBnClickedWireColour)
	COMMAND_HANDLER(IDC_POLY_COLOUR, BN_CLICKED, OnBnClickedPolyColour)
	COMMAND_HANDLER(IDC_MACHINE_BITMAP, BN_CLICKED, OnBnClickedMachineBitmap)
	COMMAND_HANDLER(IDC_BUTTON24, BN_CLICKED, OnBnClickedButton24)
	COMMAND_HANDLER(IDC_CHECK_VUS, BN_CLICKED, OnBnClickedCheckVus)
	COMMAND_HANDLER(IDC_BUTTON23, BN_CLICKED, OnBnClickedButton23)
	COMMAND_HANDLER(IDC_BUTTON25, BN_CLICKED, OnBnClickedButton25)
	COMMAND_HANDLER(IDC_DOUBLEBUFFER, BN_CLICKED, OnBnClickedDoublebuffer)
	COMMAND_HANDLER(IDC_IMPORTREG, BN_CLICKED, OnBnClickedImportreg)
	COMMAND_HANDLER(IDC_EXPORTREG, BN_CLICKED, OnBnClickedExportreg)
	COMMAND_HANDLER(IDC_MACHINE_SKIN, CBN_SELCHANGE, OnCbnSelchangeMachineSkin)
	COMMAND_HANDLER(IDC_DRAW_MAC_INDEX, BN_CLICKED, OnBnClickedDrawMacIndex)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_CLOSE, OnClose)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
// Dialog Data
	//{{AFX_DATA(CSkinDlg)
	enum { IDD = IDD_SKIN };
	CButton	m_machine_background_bitmap;
	CComboBox	m_triangle_size;
	CComboBox	m_wirewidth;
	CButton	m_pattern_fontface;
	CComboBox	m_pattern_font_point;
	CComboBox	m_pattern_font_x;
	CComboBox	m_pattern_font_y;
	CComboBox	m_pattern_header_skin;
	CComboBox	m_machine_skin;
	CButton	m_generator_fontface;
	CComboBox	m_generator_font_point;
	CButton	m_effect_fontface;
	CComboBox	m_effect_font_point;
	CButton	m_gfxbuffer;
	CButton	m_draw_vus;
	CButton	m_linenumbers;
	CButton	m_linenumbersHex;
	CButton	m_linenumbersCursor;
	CButton	m_pattern_draw_empty_data;
	CButton	m_draw_mac_index;
	CComboBox	m_wireaa;
	//}}AFX_DATA

// Implementation
protected:
	bool BrowseTo(char *rpath);
	static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe,  NEWTEXTMETRICEX *lpntme, DWORD FontType,  LPARAM lParam);
	static int CALLBACK EnumFontFamExProc2(ENUMLOGFONTEX *lpelfe,  NEWTEXTMETRICEX *lpntme, DWORD FontType,  LPARAM lParam);
private:
	void RepaintAllCanvas();
	void UpdateCanvasColour(int id,COLORREF col);
	void FindSkinsInDir(CString findDir);
	void SetFontNames();
public:
	LRESULT OnBnClickedDrawEmptyData(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPatternFontface(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangePatternFontPoint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangePatternFontX(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangePatternFontY(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangePatternHeaderSkin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeMachineFontPoint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeMachineFontPoint2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeWireWidth(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeWireaa(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeTrianglesize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLineNumbers(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLineNumbersCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLineNumbersHex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontplayc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontplayc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontselc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontselc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontcursorc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFontcursorc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPatseparatorc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPatseparatorc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPatternbackc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPatternbackc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRowc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRowc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBeatc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBeatc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClicked4beat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClicked4beat2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPlaybarc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPlaybarc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSelectionc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSelectionc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCursorc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCursorc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMachineFontface(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMvFontColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMachineFontface2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMvFontColour2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBgColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedWireColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPolyColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMachineBitmap(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton24(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheckVus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton23(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton25(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDoublebuffer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedImportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedExportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeMachineSkin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDrawMacIndex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
