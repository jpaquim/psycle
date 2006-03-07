#if !defined(AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
#define AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkinDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg dialog


class CSkinDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CSkinDlg)

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
	char _pattern_fontface[64];
	char _pattern_header_skin[64];
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

	char _machine_skin[64];
	char _generator_fontface[64];
	int _generator_font_point;
	char _effect_fontface[64];
	int _effect_font_point;

	CSkinDlg();
	~CSkinDlg();

	char _skinPathBuf[MAX_PATH];

	bool bBmpBkg;
	char szBmpBkgFilename[MAX_PATH];

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


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSkinDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool BrowseTo(char *rpath);
	static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe,  NEWTEXTMETRICEX *lpntme, DWORD FontType,  LPARAM lParam);
	static int CALLBACK EnumFontFamExProc2(ENUMLOGFONTEX *lpelfe,  NEWTEXTMETRICEX *lpntme, DWORD FontType,  LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CSkinDlg)
	virtual void OnOK( );
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnColourMachine();
	afx_msg void OnColourWire();
	afx_msg void OnColourPoly();
	afx_msg void OnVuBarColor();
	afx_msg void OnVuBackColor();
	afx_msg void OnVuClipBar();
	afx_msg void OnButtonPattern();
	afx_msg void OnButtonPattern2();
	afx_msg void OnButtonPatternSeparator();
	afx_msg void OnButtonPatternSeparator2();
	afx_msg void OnRowc();
	afx_msg void OnRowc2();
	afx_msg void OnFontc();
	afx_msg void OnFontc2();
	afx_msg void OnFontSelc();
	afx_msg void OnFontSelc2();
	afx_msg void OnFontPlayc();
	afx_msg void OnFontPlayc2();
	afx_msg void OnFontCursorc();
	afx_msg void OnFontCursorc2();
	afx_msg void OnBeatc();
	afx_msg void OnBeatc2();
	afx_msg void On4beat();
	afx_msg void On4beat2();
	afx_msg void OnPlaybar();
	afx_msg void OnPlaybar2();
	afx_msg void OnSelection();
	afx_msg void OnSelection2();
	afx_msg void OnCursor();
	afx_msg void OnCursor2();
	afx_msg void OnDoublebuffer();
	afx_msg void OnLineNumbers();
	afx_msg void OnLineNumbersHex();
	afx_msg void OnLineNumbersCursor();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnImportReg();
	afx_msg void OnExportReg();
	afx_msg void OnSelchangePatternFontPoint();
	afx_msg void OnSelchangePatternFontX();
	afx_msg void OnSelchangePatternFontY();
	afx_msg void OnPatternFontFace();
	afx_msg void OnSelchangePatternHeaderSkin();
	afx_msg void OnSelchangeWireWidth();
	afx_msg void OnSelchangeMachineSkin();
	afx_msg void OnSelchangeWireAA();
	afx_msg void OnSelchangeGeneratorFontPoint();
	afx_msg void OnGeneratorFontFace();
	afx_msg void OnMVGeneratorFontColour();
	afx_msg void OnSelchangeEffectFontPoint();
	afx_msg void OnEffectFontFace();
	afx_msg void OnMVEffectFontColour();
	afx_msg void OnDrawEmptyData();
	afx_msg void OnDrawMacIndex();
	afx_msg void OnMachineBitmap();
	afx_msg void OnSelchangeTrianglesize();
	afx_msg void OnCheckVus();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void RepaintAllCanvas();
	void UpdateCanvasColour(int id,COLORREF col);
	void FindSkinsInDir(CString findDir);
	void SetFontNames();
};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINDLG_H__0F38D786_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
