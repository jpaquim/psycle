// SkinDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "SkinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	DDX_Control(pDX, IDC_PRESETSCOMBO, m_cpresets);
	DDX_Control(pDX, IDC_DOUBLEBUFFER, m_gfxbuffer);
	DDX_Control(pDX, IDC_LINE_NUMBERS, m_linenumbers);
	DDX_Control(pDX, IDC_AAWIRE, m_wireaa);
	DDX_Control(pDX, IDC_WIRE_WIDTH, m_wirewidth);
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
	ON_BN_CLICKED(IDC_CURSORC, OnCursor)
	ON_BN_CLICKED(IDC_CURSORC2, OnCursor2)
	ON_BN_CLICKED(IDC_PLAYBARC, OnPlaybar)
	ON_BN_CLICKED(IDC_PLAYBARC2, OnPlaybar2)
	ON_BN_CLICKED(IDC_SELECTIONC, OnSelection)
	ON_BN_CLICKED(IDC_SELECTIONC2, OnSelection2)
	ON_BN_CLICKED(IDC_DOUBLEBUFFER, OnDoublebuffer)
	ON_BN_CLICKED(IDC_LINE_NUMBERS, OnLineNumbers)
	ON_CBN_SELENDOK(IDC_PRESETSCOMBO, OnSelendokPresetscombo)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg message handlers

void CSkinDlg::OnOK()
{
	//TODO: Add code here
	_wireaa = m_wireaa.GetCheck() >0?true:false;
	_wirewidth = m_wirewidth.GetCurSel()+1;
	CDialog::OnOK();
}

void CSkinDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CSkinDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_gfxbuffer.SetCheck(_gfxbuffer);
	m_wireaa.SetCheck(_wireaa);
	m_linenumbers.SetCheck(_linenumbers);
	SetTimer(2345,50,0);

	char s[4];
	for (int i = 1; i < 17; i++)
	{
		_snprintf(s,4,"%2i",i);
		m_wirewidth.AddString(s);
	}
	_snprintf(s,4,"%2i",_wirewidth);
	m_wirewidth.SelectString(0,s);

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

void CSkinDlg::OnSelendokPresetscombo() 
{
	switch( m_cpresets.GetCurSel() )
	{
	case 0: // Old theme
		{
			_machineViewColor =	0x0077AA99;
			_machineViewWireColor =	0x00000000;
			_machineViewPolyColor =	0x00ffffff;
			_patternViewColor = 0x00AADDCC;
			_4beatColor = 0x00CCCCCC;
			_beatColor = 0x00BACBCA;
			_rowColor = 0x00A9CAC8;
			_fontColor = 0x00000000;
			_wireaa = false;
			_wirewidth = 1;

			_vubColor = 0x0000FF00;
			_vugColor = 0x00000000;
			_vucColor = 0x000000FF;
			break;
		}
	case 1: // Iced theme
		{
			_machineViewColor =	0x00bfa880;
			_machineViewWireColor =	0x00000000;
			_machineViewPolyColor =	0x00ffffff;
			_patternViewColor = 0x00decaab;
			_4beatColor = 0x00dec9ab;
			_beatColor = 0x00ebddcb;
			_rowColor = 0x00f3ebe0;
			_fontColor = 0x00000000;
			_wireaa = false;
			_wirewidth = 1;

			_vubColor = 0x00d6c6a9;
			_vugColor = 0x00000000;
			_vucColor = 0x00d6c6a9;	
			break;
		}
	case 2: //clarify
		{
			_machineViewColor =	0x00b0bdbd;
			_machineViewWireColor =	0x00000000;
			_machineViewPolyColor =	0x00dddddd;
			_patternViewColor = 0x009a8d7e;
			_4beatColor = 0x00cbc5be;
			_beatColor = 0x00d5d0ca;
			_rowColor = 0x00c4cece;
			_fontColor = 0x00000000;
			_wireaa = false;
			_wirewidth = 2;

			_vubColor = 0x0025cd36;
			_vugColor = 0x00332f28;
			_vucColor = 0x000000c4;
			break;
		}
	case 3: // bluegrey
		{
			_machineViewColor =	0x009a887c;
			_machineViewWireColor =	0x003a281c;
			_machineViewPolyColor =	0x00dac8bc;
			_patternViewColor = 0x009a887c;
			_4beatColor = 0x00d5ccc6;
			_beatColor = 0x00c9beb8;
			_rowColor = 0x00c1b5aa;
			_fontColor = 0x00000000;
			_wireaa = false;
			_wirewidth = 2;

			_vubColor = 0x00f1c992;
			_vugColor = 0x00403731;
			_vucColor = 0x00262bd7;
			break;
		}
	default:
		return;
		break;
	}
	m_wireaa.SetCheck(_wireaa);
	char s[4];
	_snprintf(s,4,"%2i",_wirewidth);
	m_wirewidth.SelectString(0,s);
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

