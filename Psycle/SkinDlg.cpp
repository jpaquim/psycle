// SkinDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "SkinDlg.h"
#include "Helpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_FONTS 512

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg property page

IMPLEMENT_DYNCREATE(CSkinDlg, CPropertyPage)

CSkinDlg::CSkinDlg() : CPropertyPage(CSkinDlg::IDD)
{
	//{{AFX_DATA_INIT(CSkinDlg)
	//}}AFX_DATA_INIT
}

CSkinDlg::~CSkinDlg()
{
}

void CSkinDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkinDlg)
	DDX_Control(pDX, IDC_DOUBLEBUFFER, m_gfxbuffer);
	DDX_Control(pDX, IDC_LINE_NUMBERS, m_linenumbers);
	DDX_Control(pDX, IDC_LINE_NUMBERS_HEX, m_linenumbersHex);
	DDX_Control(pDX, IDC_AAWIRE, m_wireaa);
	DDX_Control(pDX, IDC_WIRE_WIDTH, m_wirewidth);
	DDX_Control(pDX, IDC_PATTERN_FONTFACE, m_pattern_fontface);
	DDX_Control(pDX, IDC_PATTERN_FONT_POINT, m_pattern_font_point);
	DDX_Control(pDX, IDC_PATTERN_FONT_X, m_pattern_font_x);
	DDX_Control(pDX, IDC_PATTERN_FONT_Y, m_pattern_font_y);
	

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSkinDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CSkinDlg)
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
	ON_BN_CLICKED(IDC_LINE_NUMBERS, OnLineNumbers)
	ON_BN_CLICKED(IDC_LINE_NUMBERS_HEX, OnLineNumbersHex)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_IMPORTREG, OnImportReg)
	ON_BN_CLICKED(IDC_EXPORTREG, OnExportReg)
	ON_CBN_SELCHANGE(IDC_PATTERN_FONT_POINT, OnSelchangePatternFontPoint)
	ON_CBN_SELCHANGE(IDC_PATTERN_FONT_X, OnSelchangePatternFontX)
	ON_CBN_SELCHANGE(IDC_PATTERN_FONT_Y, OnSelchangePatternFontY)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg message handlers

void CSkinDlg::OnOK()
{
	//TODO: Add code here
	_wireaa = m_wireaa.GetCheck() >0?true:false;
	_wirewidth = m_wirewidth.GetCurSel()+1;
	m_pattern_fontface.GetLBText(m_pattern_fontface.GetCurSel(),_pattern_fontface);
		
	CDialog::OnOK();
}

void CSkinDlg::OnCancel() 
{
	CDialog::OnCancel();
}

int CALLBACK CSkinDlg::EnumFontFamExProc(
  ENUMLOGFONTEX *lpelfe,    // logical-font data
  NEWTEXTMETRICEX *lpntme,  // physical-font data
  DWORD FontType,           // type of font
  LPARAM lParam             // application-defined data
)
{
	// due to the stupid way ms font enum callback works, you have to find the family, and then enumerate that
	// again if you want everything to be listed
	LOGFONT lf;
	memset(&lf,0,sizeof(lf));
//	lf.lfCharSet = ANSI_CHARSET;
	lf.lfCharSet = DEFAULT_CHARSET;
	strcpy(lf.lfFaceName,(char*)lpelfe->elfFullName);
	char* p = strchr(lf.lfFaceName,32); // space
	if ((p) && (p!=lf.lfFaceName))
	{
		p[0] = 0;
	}

	EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamExProc2, 0, 0);
	if(pm_pattern_fontface->FindStringExact(0,(char*)lpelfe->elfFullName)==CB_ERR)
	{
		pm_pattern_fontface->AddString((char*)lpelfe->elfFullName);
//		numFonts++;
	}
	/*
	if (numFonts >= MAX_FONTS)
	{
		::MessageBox(0,"You have more than 512 fonts, you are crazy!","font enumeration error",0);
		return 0;
	}
	*/
	return 1;
}

int CALLBACK CSkinDlg::EnumFontFamExProc2(
  ENUMLOGFONTEX *lpelfe,    // logical-font data
  NEWTEXTMETRICEX *lpntme,  // physical-font data
  DWORD FontType,           // type of font
  LPARAM lParam             // application-defined data
)
{
	if(pm_pattern_fontface->FindStringExact(0,(char*)lpelfe->elfFullName)==CB_ERR)
	{
		pm_pattern_fontface->AddString((char*)lpelfe->elfFullName);
//		numFonts++;
	}
	/*
	if (numFonts >= MAX_FONTS)
	{
		return 0;
	}
	*/
	return 1;
}

CComboBox * CSkinDlg::pm_pattern_fontface;
HDC CSkinDlg::hDC;
//int CSkinDlg::numFonts;

BOOL CSkinDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_hWnd = GetParent()->m_hWnd;
	m_gfxbuffer.SetCheck(_gfxbuffer);
	m_wireaa.SetCheck(_wireaa);
	m_linenumbers.SetCheck(_linenumbers);
	m_linenumbersHex.SetCheck(_linenumbersHex);
	SetTimer(2345,50,0);

	char s[4];
	for (int i = 1; i <= 16; i++)
	{
		sprintf(s,"%2i",i);
		m_wirewidth.AddString(s);
	}
	m_wirewidth.SetCurSel(_wirewidth-1);
//	sprintf(s,"%2i",_wirewidth);
//	m_wirewidth.SelectString(0,s);

	LOGFONT lf;
	memset(&lf,0,sizeof(lf));
//	lf.lfCharSet = ANSI_CHARSET;
	lf.lfCharSet = DEFAULT_CHARSET;
	hDC = ::GetDC( NULL );

	pm_pattern_fontface = &m_pattern_fontface;
//	numFonts = 0;
//	EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamExProc2, 0, 0);
	EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamExProc, 0, 0);
	::ReleaseDC( NULL, hDC );

	int sel;
	sel = m_pattern_fontface.FindStringExact(0,_pattern_fontface);
	if (sel==CB_ERR)
	{
		MessageBox(_pattern_fontface,"Could not find this font!");
		sel = m_pattern_fontface.FindStringExact(0,"Tahoma");
		if (sel==CB_ERR)
		{
			sel = m_pattern_fontface.FindStringExact(0,"MS Sans Seriff");
			if (sel==CB_ERR)
			{
				sel=0;
			}
		}
	}
	m_pattern_fontface.SetCurSel(sel);

	for(i=4;i<=64;i++)
	{
		char s[4];
		sprintf(s,"%2i",i);
		m_pattern_font_x.AddString(s);
		m_pattern_font_y.AddString(s);
	}
	m_pattern_font_x.SetCurSel(_pattern_font_x-4);
	m_pattern_font_y.SetCurSel(_pattern_font_y-4);

	for (i = 50; i <= 320; i+=5)
	{
		char s[8];
		if (i < 100)
		{
			sprintf(s," %.1f",float(i)/10.0f);
		}
		else
		{
			sprintf(s,"%.1f",float(i)/10.0f);
		}
		m_pattern_font_point.AddString(s);
	}
	m_pattern_font_point.SetCurSel((_pattern_font_point-50)/5);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSkinDlg::OnColourMachine() 
{
	CColorDialog dlg(_machineViewColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MBG_CAN,_machineViewColor);
	}
}

void CSkinDlg::OnColourWire() 
{
	CColorDialog dlg(_machineViewWireColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewWireColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MWIRE_COL,_machineViewWireColor);
	}
}


void CSkinDlg::OnColourPoly() 
{
	CColorDialog dlg(_machineViewPolyColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewPolyColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MPOLY_COL,_machineViewPolyColor);
	}
}

void CSkinDlg::OnVuBarColor() 
{
	CColorDialog dlg(_vubColor);

	if(dlg.DoModal() == IDOK)
	{
		_vubColor = dlg.GetColor();
		UpdateCanvasColour(IDC_VU1_CAN,_vubColor);
	}
}

void CSkinDlg::OnVuBackColor() 
{
	CColorDialog dlg(_vugColor);

	if(dlg.DoModal() == IDOK)
	{
		_vugColor = dlg.GetColor();
		UpdateCanvasColour(IDC_VU2_CAN,_vugColor);
	}
}

void CSkinDlg::OnVuClipBar() 
{
	CColorDialog dlg(_vucColor);

	if(dlg.DoModal() == IDOK)
	{
		_vucColor = dlg.GetColor();
		UpdateCanvasColour(IDC_VU3_CAN,_vucColor);
	}
}

void CSkinDlg::OnButtonPattern() 
{
	CColorDialog dlg(_patternViewColor);

	if(dlg.DoModal() == IDOK)
	{
		_patternViewColor = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_CAN,_patternViewColor);
	}
}


void CSkinDlg::OnButtonPattern2() 
{
	CColorDialog dlg(_patternViewColor2);

	if(dlg.DoModal() == IDOK)
	{
		_patternViewColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_CAN2,_patternViewColor2);
	}
}


void CSkinDlg::OnButtonPatternSeparator() 
{
	CColorDialog dlg(_patternSeparatorColor);

	if(dlg.DoModal() == IDOK)
	{
		_patternSeparatorColor = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_SEPARATOR,_patternSeparatorColor);
	}
}

void CSkinDlg::OnButtonPatternSeparator2() 
{
	CColorDialog dlg(_patternSeparatorColor2);

	if(dlg.DoModal() == IDOK)
	{
		_patternSeparatorColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_SEPARATOR2,_patternSeparatorColor2);
	}
}


void CSkinDlg::OnRowc() 
{
	CColorDialog dlg(_rowColor);

	if(dlg.DoModal() == IDOK)
	{
		_rowColor = dlg.GetColor();
		UpdateCanvasColour(IDC_ROW_CAN,_rowColor);
	}
}

void CSkinDlg::OnRowc2() 
{
	CColorDialog dlg(_rowColor2);

	if(dlg.DoModal() == IDOK)
	{
		_rowColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_ROW_CAN2,_rowColor2);
	}
}

void CSkinDlg::OnFontc() 
{
	CColorDialog dlg(_fontColor);

	if(dlg.DoModal() == IDOK)
	{
		_fontColor = dlg.GetColor();
		UpdateCanvasColour(IDC_FONT_CAN,_fontColor);
	}
}

void CSkinDlg::OnFontc2() 
{
	CColorDialog dlg(_fontColor2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONT_CAN2,_fontColor2);
	}
}

void CSkinDlg::OnFontPlayc() 
{
	CColorDialog dlg(_fontColorPlay);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorPlay = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTPLAY_CAN,_fontColorPlay);
	}
}

void CSkinDlg::OnFontPlayc2() 
{
	CColorDialog dlg(_fontColorPlay2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorPlay2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTPLAY_CAN2,_fontColorPlay2);
	}
}

void CSkinDlg::OnFontSelc() 
{
	CColorDialog dlg(_fontColorSel);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorSel = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTSEL_CAN,_fontColorSel);
	}
}

void CSkinDlg::OnFontSelc2() 
{
	CColorDialog dlg(_fontColorSel2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorSel2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTSEL_CAN2,_fontColorSel2);
	}
}

void CSkinDlg::OnFontCursorc() 
{
	CColorDialog dlg(_fontColorCur);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorCur = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTCURSOR_CAN,_fontColorCur);
	}
}

void CSkinDlg::OnFontCursorc2() 
{
	CColorDialog dlg(_fontColorCur2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorCur2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTCURSOR_CAN2,_fontColorCur2);
	}
}

void CSkinDlg::OnBeatc() 
{
	CColorDialog dlg(_beatColor);

	if(dlg.DoModal() == IDOK)
	{
		_beatColor = dlg.GetColor();
		UpdateCanvasColour(IDC_BEAT_CAN,_beatColor);
	}	
}

void CSkinDlg::OnBeatc2() 
{
	CColorDialog dlg(_beatColor2);

	if(dlg.DoModal() == IDOK)
	{
		_beatColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_BEAT_CAN2,_beatColor2);
	}	
}

void CSkinDlg::On4beat() 
{
	CColorDialog dlg(_4beatColor);

	if(dlg.DoModal() == IDOK)
	{
		_4beatColor = dlg.GetColor();
		UpdateCanvasColour(IDC_4BEAT_CAN,_4beatColor);
	}	
}

void CSkinDlg::On4beat2() 
{
	CColorDialog dlg(_4beatColor2);

	if(dlg.DoModal() == IDOK)
	{
		_4beatColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_4BEAT_CAN2,_4beatColor2);
	}	
}

void CSkinDlg::OnSelection() 
{
	CColorDialog dlg(_selectionColor);

	if(dlg.DoModal() == IDOK)
	{
		_selectionColor = dlg.GetColor();
		UpdateCanvasColour(IDC_SELECTION_CAN,_selectionColor);
	}	
}

void CSkinDlg::OnSelection2() 
{
	CColorDialog dlg(_selectionColor2);

	if(dlg.DoModal() == IDOK)
	{
		_selectionColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_SELECTION_CAN2,_selectionColor2);
	}	
}

void CSkinDlg::OnCursor() 
{
	CColorDialog dlg(_cursorColor);

	if(dlg.DoModal() == IDOK)
	{
		_cursorColor = dlg.GetColor();
		UpdateCanvasColour(IDC_CURSOR_CAN,_cursorColor);
	}	
}

void CSkinDlg::OnCursor2() 
{
	CColorDialog dlg(_cursorColor2);

	if(dlg.DoModal() == IDOK)
	{
		_cursorColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_CURSOR_CAN2,_cursorColor2);
	}	
}

void CSkinDlg::OnPlaybar() 
{
	CColorDialog dlg(_playbarColor);

	if(dlg.DoModal() == IDOK)
	{
		_playbarColor = dlg.GetColor();
		UpdateCanvasColour(IDC_PLAYBAR_CAN,_playbarColor);
	}	
}

void CSkinDlg::OnPlaybar2() 
{
	CColorDialog dlg(_playbarColor2);

	if(dlg.DoModal() == IDOK)
	{
		_playbarColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_PLAYBAR_CAN2,_playbarColor2);
	}	
}

void CSkinDlg::OnDoublebuffer() 
{
	_gfxbuffer = m_gfxbuffer.GetCheck() >0?true:false;
}

void CSkinDlg::OnLineNumbers() 
{
	_linenumbers = m_linenumbers.GetCheck() >0?true:false;
}

void CSkinDlg::OnLineNumbersHex() 
{
	_linenumbersHex = m_linenumbersHex.GetCheck() >0?true:false;
}


void CSkinDlg::RepaintAllCanvas()
{
	UpdateCanvasColour(IDC_MBG_CAN,_machineViewColor);
	UpdateCanvasColour(IDC_MWIRE_COL,_machineViewWireColor);
	UpdateCanvasColour(IDC_MPOLY_COL,_machineViewPolyColor);
	UpdateCanvasColour(IDC_VU1_CAN,_vubColor);
	UpdateCanvasColour(IDC_VU2_CAN,_vugColor);
	UpdateCanvasColour(IDC_VU3_CAN,_vucColor);

	UpdateCanvasColour(IDC_PBG_CAN,_patternViewColor);
	UpdateCanvasColour(IDC_PBG_CAN2,_patternViewColor2);
	UpdateCanvasColour(IDC_PBG_SEPARATOR,_patternSeparatorColor);
	UpdateCanvasColour(IDC_PBG_SEPARATOR2,_patternSeparatorColor2);
	UpdateCanvasColour(IDC_4BEAT_CAN,_4beatColor);
	UpdateCanvasColour(IDC_4BEAT_CAN2,_4beatColor2);
	UpdateCanvasColour(IDC_BEAT_CAN,_beatColor);
	UpdateCanvasColour(IDC_BEAT_CAN2,_beatColor2);
	UpdateCanvasColour(IDC_ROW_CAN,_rowColor);
	UpdateCanvasColour(IDC_ROW_CAN2,_rowColor2);
	UpdateCanvasColour(IDC_FONT_CAN,_fontColor);
	UpdateCanvasColour(IDC_FONT_CAN2,_fontColor2);
	UpdateCanvasColour(IDC_FONTPLAY_CAN,_fontColorPlay);
	UpdateCanvasColour(IDC_FONTPLAY_CAN2,_fontColorPlay2);
	UpdateCanvasColour(IDC_FONTCURSOR_CAN,_fontColorCur);
	UpdateCanvasColour(IDC_FONTCURSOR_CAN2,_fontColorCur2);
	UpdateCanvasColour(IDC_FONTSEL_CAN,_fontColorSel);
	UpdateCanvasColour(IDC_FONTSEL_CAN2,_fontColorSel2);
	UpdateCanvasColour(IDC_CURSOR_CAN,_cursorColor);
	UpdateCanvasColour(IDC_CURSOR_CAN2,_cursorColor2);
	UpdateCanvasColour(IDC_PLAYBAR_CAN,_playbarColor);
	UpdateCanvasColour(IDC_PLAYBAR_CAN2,_playbarColor2);
	UpdateCanvasColour(IDC_SELECTION_CAN,_selectionColor);
	UpdateCanvasColour(IDC_SELECTION_CAN2,_selectionColor2);

}

void CSkinDlg::UpdateCanvasColour(int id,COLORREF col)
{
	CStatic *obj=(CStatic *)GetDlgItem(id);
	CClientDC can(obj);
	can.FillSolidRect(0,0,16,13,col);
}


void CSkinDlg::OnClose() 
{
	KillTimer(2345);
	
	CPropertyPage::OnClose();
}

void CSkinDlg::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 2345 )
	{
		RepaintAllCanvas();
	}
	
	CPropertyPage::OnTimer(nIDEvent);
}


void CSkinDlg::OnImportReg() 
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Psycle Display Presets\0*.psv\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = _skinPathBuf;
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		FILE* hfile;
		if ((hfile=fopen(szFile,"rw")) == NULL )
		{
			MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
			return;
		}
		char buf[512];
		while (fgets(buf, 512, hfile))
		{
			if (strstr(buf,"\"pattern_fontface\"=\""))
			{
				char *q = strchr(buf,61); // =
				if (q)
				{
					q+=2;
					char *p = strrchr(q,34); // "
					if (p)
					{
						p[0]=0;
						strcpy(_pattern_fontface,q);
					}
				}
			}
			else if (strstr(buf,"\"pattern_font_point\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_pattern_font_point=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pattern_font_x\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_pattern_font_x=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pattern_font_y\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_pattern_font_y=_httoi(q+1);
				}
			}
			/*
			else if (strstr(buf,"\"DisplayLineNumbers\"=hex:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_linenumbers=_httoi(q+1)?1:0;
				}
			}
			else if (strstr(buf,"\"DisplayLineNumbersHex\"=hex:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_linenumbersHex=_httoi(q+1)?1:0;
				}
			}
			*/
			else if (strstr(buf,"\"pvc_separator\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternSeparatorColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_separator2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternSeparatorColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_background\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternViewColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_background2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternViewColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_font\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_font2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_fontCur\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorCur=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_fontCur2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorCur2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_fontSel\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorSel=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_fontSel2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorSel2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_fontPlay\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorPlay=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_fontPlay2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorPlay2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_row\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_rowColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_row2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_rowColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_rowbeat\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_beatColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_rowbeat2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_beatColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_row4beat\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_4beatColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_row4beat2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_4beatColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_selection\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_selectionColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_selection2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_selectionColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_playbar\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_playbarColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_playbar2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_playbarColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_cursor\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_cursorColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"pvc_cursor2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_cursorColor2=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"vu1\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_vubColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"vu2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_vugColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"vu3\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_vucColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"mv_colour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"mv_wirecolour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewWireColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"mv_polycolour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewPolyColor=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"mv_wirewidth\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_wirewidth=_httoi(q+1);
				}
			}
			else if (strstr(buf,"\"mv_wirewidth\"=hex:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_wireaa=_httoi(q+1)?1:0;
				}
			}
		}
		fclose(hfile);
		m_wireaa.SetCheck(_wireaa);
//		m_linenumbers.SetCheck(_linenumbers);
//		m_linenumbersHex.SetCheck(_linenumbersHex);
		_snprintf(buf,4,"%2i",_wirewidth);
		m_wirewidth.SelectString(0,buf);
		RepaintAllCanvas();

		int sel;
		sel = m_pattern_fontface.FindStringExact(0,_pattern_fontface);
		if (sel==CB_ERR)
		{
			MessageBox(_pattern_fontface,"Could not find this font!");
			sel = m_pattern_fontface.FindStringExact(0,"Tahoma");
			if (sel==CB_ERR)
			{
				sel = m_pattern_fontface.FindStringExact(0,"MS Sans Seriff");
				if (sel==CB_ERR)
				{
					sel=0;
				}
			}
		}
		m_pattern_fontface.SetCurSel(sel);
		m_pattern_font_x.SetCurSel(_pattern_font_x-4);
		m_pattern_font_y.SetCurSel(_pattern_font_y-4);
		m_pattern_font_point.SetCurSel((_pattern_font_point-50)/5);
	}
}

void CSkinDlg::OnExportReg() 
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[_MAX_PATH];       // buffer for file name
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent()->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Psycle Display Presets\0*.psv\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST;	

	if (GetSaveFileName(&ofn)==TRUE)
	{
		FILE* hfile;

		_wireaa = m_wireaa.GetCheck() >0?true:false;
		_wirewidth = m_wirewidth.GetCurSel()+1;
		m_pattern_fontface.GetLBText(m_pattern_fontface.GetCurSel(),_pattern_fontface);

		CString str = szFile;
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(".psv") != 0 ) str.Insert(str.GetLength(),".psv");
		sprintf(szFile,str);
		DeleteFile(szFile);

		if ((hfile=fopen(szFile,"wa")) == NULL ) // file does not exist.
		{
			MessageBox("Couldn't open File for Writing. Operation Aborted","File Save Error",MB_OK);
			return;
		}

		fprintf(hfile,"[Psycle Display Presets v1.0]\n\n");

		fprintf(hfile,"\"pattern_fontface\"=\"%s\"\n",_pattern_fontface);
		fprintf(hfile,"\"pattern_font_point\"=dword:%.8X\n",_pattern_font_point);
		fprintf(hfile,"\"pattern_font_x\"=dword:%.8X\n",_pattern_font_x);
		fprintf(hfile,"\"pattern_font_y\"=dword:%.8X\n",_pattern_font_y);
//		fprintf(hfile,"\"DisplayLineNumbers\"=hex:%.2X\n",_linenumbers?1:0);
//		fprintf(hfile,"\"DisplayLineNumbersHex\"=hex:%.2X\n",_linenumbersHex?1:0);
		fprintf(hfile,"\"pvc_separator\"=dword:%.8X\n",_patternSeparatorColor);
		fprintf(hfile,"\"pvc_separator2\"=dword:%.8X\n",_patternSeparatorColor2);
		fprintf(hfile,"\"pvc_background\"=dword:%.8X\n",_patternViewColor);
		fprintf(hfile,"\"pvc_background2\"=dword:%.8X\n",_patternViewColor2);
		fprintf(hfile,"\"pvc_font\"=dword:%.8X\n",_fontColor);
		fprintf(hfile,"\"pvc_font2\"=dword:%.8X\n",_fontColor2);
		fprintf(hfile,"\"pvc_fontCur\"=dword:%.8X\n",_fontColorCur);
		fprintf(hfile,"\"pvc_fontCur2\"=dword:%.8X\n",_fontColorCur2);
		fprintf(hfile,"\"pvc_fontSel\"=dword:%.8X\n",_fontColorSel);
		fprintf(hfile,"\"pvc_fontSel2\"=dword:%.8X\n",_fontColorSel2);
		fprintf(hfile,"\"pvc_fontPlay\"=dword:%.8X\n",_fontColorPlay);
		fprintf(hfile,"\"pvc_fontPlay2\"=dword:%.8X\n",_fontColorPlay2);
		fprintf(hfile,"\"pvc_row\"=dword:%.8X\n",_rowColor);
		fprintf(hfile,"\"pvc_row2\"=dword:%.8X\n",_rowColor2);
		fprintf(hfile,"\"pvc_rowbeat\"=dword:%.8X\n",_beatColor);
		fprintf(hfile,"\"pvc_rowbeat2\"=dword:%.8X\n",_beatColor2);
		fprintf(hfile,"\"pvc_row4beat\"=dword:%.8X\n",_4beatColor);
		fprintf(hfile,"\"pvc_row4beat2\"=dword:%.8X\n",_4beatColor2);
		fprintf(hfile,"\"pvc_selection\"=dword:%.8X\n",_selectionColor);
		fprintf(hfile,"\"pvc_selection2\"=dword:%.8X\n",_selectionColor2);
		fprintf(hfile,"\"pvc_playbar\"=dword:%.8X\n",_playbarColor);
		fprintf(hfile,"\"pvc_playbar2\"=dword:%.8X\n",_playbarColor2);
		fprintf(hfile,"\"pvc_cursor\"=dword:%.8X\n",_cursorColor);
		fprintf(hfile,"\"pvc_cursor2\"=dword:%.8X\n",_cursorColor2);
		fprintf(hfile,"\"vu1\"=dword:%.8X\n",_vubColor);
		fprintf(hfile,"\"vu2\"=dword:%.8X\n",_vugColor);
		fprintf(hfile,"\"vu3\"=dword:%.8X\n",_vucColor);
		fprintf(hfile,"\"mv_colour\"=dword:%.8X\n",_machineViewColor);
		fprintf(hfile,"\"mv_wirecolour\"=dword:%.8X\n",_machineViewWireColor);
		fprintf(hfile,"\"mv_polycolour\"=dword:%.8X\n",_machineViewPolyColor);
		fprintf(hfile,"\"mv_wirewidth\"=dword:%.8X\n",_wirewidth);
		fprintf(hfile,"\"mv_wireaa\"=hex:%.2X\n",_wireaa?1:0);

		fclose(hfile);
	}
}

void CSkinDlg::OnSelchangePatternFontPoint() 
{
	// TODO: Add your control notification handler code here
	_pattern_font_point=(m_pattern_font_point.GetCurSel()*5)+50;
}

void CSkinDlg::OnSelchangePatternFontX() 
{
	// TODO: Add your control notification handler code here
	_pattern_font_x=m_pattern_font_x.GetCurSel()+4;
	
}

void CSkinDlg::OnSelchangePatternFontY() 
{
	// TODO: Add your control notification handler code here
	_pattern_font_y=m_pattern_font_y.GetCurSel()+4;
	
}
