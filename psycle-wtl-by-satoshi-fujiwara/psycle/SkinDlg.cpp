/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

// SkinDlg.cpp : implementation file
//

#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "SkinDlg.h"
#include "Helpers.h"
#include "configuration.h"

#define MAX_FONTS 256


/////////////////////////////////////////////////////////////////////////////
// CSkinDlg property page

//IMPLEMENT_DYNCREATE(CSkinDlg, CPropertyPage)

CSkinDlg::CSkinDlg() 
{
	//{{AFX_DATA_INIT(CSkinDlg)
	//}}AFX_DATA_INIT
}

CSkinDlg::~CSkinDlg()
{
}
/*
void CSkinDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkinDlg)
	DDX_Control(pDX, IDC_MACHINE_BITMAP, m_machine_background_bitmap);
	DDX_Control(pDX, IDC_TRIANGLESIZE, m_triangle_size);
	DDX_Control(pDX, IDC_WIRE_WIDTH, m_wirewidth);
	DDX_Control(pDX, IDC_PATTERN_FONTFACE, m_pattern_fontface);
	DDX_Control(pDX, IDC_PATTERN_FONT_POINT, m_pattern_font_point);
	DDX_Control(pDX, IDC_PATTERN_FONT_X, m_pattern_font_x);
	DDX_Control(pDX, IDC_PATTERN_FONT_Y, m_pattern_font_y);
	DDX_Control(pDX, IDC_PATTERN_HEADER_SKIN, m_pattern_header_skin);
	DDX_Control(pDX, IDC_MACHINE_SKIN, m_machine_skin);
	DDX_Control(pDX, IDC_MACHINE_FONTFACE, m_generator_fontface);
	DDX_Control(pDX, IDC_MACHINE_FONT_POINT, m_generator_font_point);
	DDX_Control(pDX, IDC_MACHINE_FONTFACE2, m_effect_fontface);
	DDX_Control(pDX, IDC_MACHINE_FONT_POINT2, m_effect_font_point);
	DDX_Control(pDX, IDC_DOUBLEBUFFER, m_gfxbuffer);
	DDX_Control(pDX, IDC_CHECK_VUS, m_draw_vus);
	DDX_Control(pDX, IDC_LINE_NUMBERS, m_linenumbers);
	DDX_Control(pDX, IDC_LINE_NUMBERS_HEX, m_linenumbersHex);
	DDX_Control(pDX, IDC_LINE_NUMBERS_CURSOR, m_linenumbersCursor);
	DDX_Control(pDX, IDC_DRAW_EMPTY_DATA, m_pattern_draw_empty_data);
	DDX_Control(pDX, IDC_DRAW_MAC_INDEX, m_draw_mac_index);
	DDX_Control(pDX, IDC_WIREAA, m_wireaa);
	//}}AFX_DATA_MAP
}
*/



/////////////////////////////////////////////////////////////////////////////
// CSkinDlg message handlers
/*
void CSkinDlg::OnOK()
{
	//TODO: Add code here

	CDialogImpl::OnOK();
}

void CSkinDlg::OnCancel() 
{
	CDialogImpl::OnCancel();
}
*/

void CSkinDlg::FindSkinsInDir(CString findDir)
{
	CFindFile finder;


	int loop = finder.FindFile(findDir + _T("\\*."));	// check for subfolders.
	while (loop) 
	{								// Note: Subfolders with dots won't work.
		loop = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindSkinsInDir(finder.GetFilePath());
		}
	}
	finder.Close();

	loop = finder.FindFile(findDir + _T("\\*.psh")); // check if the directory is empty
	while (loop)
	{
		if (!finder.IsDirectory())
		{
			CString sName, tmpPath;
			sName = finder.GetFileName();
			// ok so we have a .psh, does it have a valid matching .bmp?
			TCHAR szBmpName[MAX_PATH];
			TCHAR* pExt = _tcsrchr(sName.GetBuffer(),46);// last .
			pExt[0] = 0;
			_stprintf(szBmpName,_T("%s\\%s.bmp"),findDir,sName);
			HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szBmpName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hBitmap)
			{
				m_pattern_header_skin.AddString(sName);
			}
			DeleteObject(hBitmap);
		}
		loop = finder.FindNextFile();

	}

	loop = finder.FindFile(findDir + _T("\\*.psm")); // check if the directory is empty
	while (loop)
	{
		if (!finder.IsDirectory())
		{
			CString sName, tmpPath;
			sName = finder.GetFileName();
			// ok so we have a .psh, does it have a valid matching .bmp?
			TCHAR szBmpName[MAX_PATH];
			TCHAR* pExt = _tcsrchr(sName.GetBuffer(),46);// last .
			pExt[0] = 0;
			_stprintf(szBmpName,_T("%s\\%s.bmp"),findDir,sName);
			HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szBmpName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hBitmap)
			{
				m_machine_skin.AddString(sName);
			}
			DeleteObject(hBitmap);
		}
		loop = finder.FindNextFile();
	}
	finder.Close();
}

void CSkinDlg::RepaintAllCanvas()
{
	UpdateCanvasColour(IDC_MBG_CAN,_machineViewColor);
	UpdateCanvasColour(IDC_MWIRE_COL,_machineViewWireColor);
	UpdateCanvasColour(IDC_MPOLY_COL,_machineViewPolyColor);
	UpdateCanvasColour(IDC_MBG_MV_FONT,_machineViewGeneratorFontColor);
	UpdateCanvasColour(IDC_MBG_MV_FONT2,_machineViewEffectFontColor);
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
	//CStatic obj;
	//obj = GetDlgItem(id);
	CClientDC can(GetDlgItem(id));
	can.FillSolidRect(0,0,16,13,col);
}

void CSkinDlg::SetFontNames()
{
	TCHAR buf[256];
	_tcscpy(buf,_pattern_fontface);
	if (_pattern_font_flags & 1)
	{
		_tcscat(buf,_T(" Bold"));
	}
	if (_pattern_font_flags & 2)
	{
		_tcscat(buf,_T(" Italic"));
	}
	m_pattern_fontface.SetWindowText(buf);
	
	_tcscpy(buf,_generator_fontface);
	if (_generator_font_flags & 1)
	{
		_tcscat(buf,_T(" Bold"));
	}
	if (_generator_font_flags & 2)
	{
		_tcscat(buf,_T(" Italic"));
	}
	m_generator_fontface.SetWindowText(buf);

	_tcscpy(buf,_effect_fontface);
	if (_effect_font_flags & 1)
	{
		_tcscat(buf,_T(" Bold"));
	}
	if (_effect_font_flags & 2)
	{
		_tcscat(buf,_T(" Italic"));
	}
	m_effect_fontface.SetWindowText(buf);

	m_pattern_font_point.SetCurSel((_pattern_font_point-50)/5);
	m_generator_font_point.SetCurSel((_generator_font_point-50)/5);
	m_effect_font_point.SetCurSel((_effect_font_point-50)/5);
}

LRESULT CSkinDlg::OnBnClickedDrawEmptyData(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pattern_draw_empty_data = m_pattern_draw_empty_data.GetCheck() >0?true:false;

	return 0;
}

LRESULT CSkinDlg::OnBnClickedPatternFontface(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	CClientDC dc((*this));
	lf.lfHeight = -MulDiv(_pattern_font_point/10, dc.GetDeviceCaps(LOGPIXELSY), 72);
	_tcscpy(lf.lfFaceName, _pattern_fontface);
	if (_pattern_font_flags&1)
	{
		lf.lfWeight = FW_BOLD;
	}
	lf.lfItalic = (_pattern_font_flags&2)?true:false;
	
	CFontDialog dlg(&lf,CF_SCREENFONTS);
	if (dlg.DoModal() == IDOK)
	{
		_tcscpy(_pattern_fontface,dlg.GetFaceName());
		_pattern_font_flags = 0;
		if (dlg.IsBold())
		{
			_pattern_font_flags |= 1;
		}
		if (dlg.IsItalic())
		{
			_pattern_font_flags |= 2;
		}
		_pattern_font_point = dlg.GetSize();
		if (_pattern_font_point > 320)
		{
			_pattern_font_point = 320;
		}
		// get size, colour too
		SetFontNames();
	}
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangePatternFontPoint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pattern_font_point=(m_pattern_font_point.GetCurSel()*5)+50;
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangePatternFontX(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pattern_font_x=m_pattern_font_x.GetCurSel()+4;
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangePatternFontY(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pattern_font_y=m_pattern_font_y.GetCurSel()+4;

	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangePatternHeaderSkin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pattern_header_skin.GetLBText(m_pattern_header_skin.GetCurSel(),_pattern_header_skin);

	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangeMachineFontPoint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_generator_font_point=(m_generator_font_point.GetCurSel()*5)+50;
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangeMachineFontPoint2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_effect_font_point=(m_effect_font_point.GetCurSel()*5)+50;
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangeWireWidth(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_wirewidth = m_wirewidth.GetCurSel()+1;
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangeWireaa(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_wireaa = m_wireaa.GetCurSel();
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangeTrianglesize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_triangle_size=m_triangle_size.GetCurSel()+8;
	return 0;
}

LRESULT CSkinDlg::OnBnClickedLineNumbers(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_linenumbers = m_linenumbers.GetCheck() >0?true:false;

	return 0;
}

LRESULT CSkinDlg::OnBnClickedLineNumbersCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_linenumbersCursor = m_linenumbersCursor.GetCheck() >0?true:false;

	return 0;
}

LRESULT CSkinDlg::OnBnClickedLineNumbersHex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_linenumbersHex = m_linenumbersHex.GetCheck() >0?true:false;
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColor);

	if(dlg.DoModal() == IDOK)
	{
		_fontColor = dlg.GetColor();
		UpdateCanvasColour(IDC_FONT_CAN,_fontColor);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColor2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONT_CAN2,_fontColor2);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontplayc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColorPlay);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorPlay = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTPLAY_CAN,_fontColorPlay);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontplayc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColorPlay2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorPlay2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTPLAY_CAN2,_fontColorPlay2);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontselc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColorSel);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorSel = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTSEL_CAN,_fontColorSel);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontselc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColorSel2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorSel2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTSEL_CAN2,_fontColorSel2);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontcursorc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColorCur);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorCur = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTCURSOR_CAN,_fontColorCur);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedFontcursorc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_fontColorCur2);

	if(dlg.DoModal() == IDOK)
	{
		_fontColorCur2 = dlg.GetColor();
		UpdateCanvasColour(IDC_FONTCURSOR_CAN2,_fontColorCur2);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedPatseparatorc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_patternSeparatorColor);

	if(dlg.DoModal() == IDOK)
	{
		_patternSeparatorColor = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_SEPARATOR,_patternSeparatorColor);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedPatseparatorc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
		CColorDialog dlg(_patternSeparatorColor2);

	if(dlg.DoModal() == IDOK)
	{
		_patternSeparatorColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_SEPARATOR2,_patternSeparatorColor2);
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedPatternbackc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_patternViewColor);

	if(dlg.DoModal() == IDOK)
	{
		_patternViewColor = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_CAN,_patternViewColor);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedPatternbackc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_patternViewColor2);

	if(dlg.DoModal() == IDOK)
	{
		_patternViewColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_PBG_CAN2,_patternViewColor2);
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedRowc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_rowColor);

	if(dlg.DoModal() == IDOK)
	{
		_rowColor = dlg.GetColor();
		UpdateCanvasColour(IDC_ROW_CAN,_rowColor);
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedRowc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_rowColor2);

	if(dlg.DoModal() == IDOK)
	{
		_rowColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_ROW_CAN2,_rowColor2);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedBeatc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_beatColor);

	if(dlg.DoModal() == IDOK)
	{
		_beatColor = dlg.GetColor();
		UpdateCanvasColour(IDC_BEAT_CAN,_beatColor);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClickedBeatc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_beatColor2);

	if(dlg.DoModal() == IDOK)
	{
		_beatColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_BEAT_CAN2,_beatColor2);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClicked4beat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_4beatColor);

	if(dlg.DoModal() == IDOK)
	{
		_4beatColor = dlg.GetColor();
		UpdateCanvasColour(IDC_4BEAT_CAN,_4beatColor);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClicked4beat2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_4beatColor2);

	if(dlg.DoModal() == IDOK)
	{
		_4beatColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_4BEAT_CAN2,_4beatColor2);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClickedPlaybarc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_playbarColor);

	if(dlg.DoModal() == IDOK)
	{
		_playbarColor = dlg.GetColor();
		UpdateCanvasColour(IDC_PLAYBAR_CAN,_playbarColor);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClickedPlaybarc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_playbarColor2);

	if(dlg.DoModal() == IDOK)
	{
		_playbarColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_PLAYBAR_CAN2,_playbarColor2);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClickedSelectionc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_selectionColor);

	if(dlg.DoModal() == IDOK)
	{
		_selectionColor = dlg.GetColor();
		UpdateCanvasColour(IDC_SELECTION_CAN,_selectionColor);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClickedSelectionc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_selectionColor2);

	if(dlg.DoModal() == IDOK)
	{
		_selectionColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_SELECTION_CAN2,_selectionColor2);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClickedCursorc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_cursorColor);

	if(dlg.DoModal() == IDOK)
	{
		_cursorColor = dlg.GetColor();
		UpdateCanvasColour(IDC_CURSOR_CAN,_cursorColor);
	}	


	return 0;
}

LRESULT CSkinDlg::OnBnClickedCursorc2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_cursorColor2);

	if(dlg.DoModal() == IDOK)
	{
		_cursorColor2 = dlg.GetColor();
		UpdateCanvasColour(IDC_CURSOR_CAN2,_cursorColor2);
	}	
	return 0;
}

LRESULT CSkinDlg::OnBnClickedMachineFontface(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
		LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	CClientDC dc(*this);
	lf.lfHeight = -MulDiv(_generator_font_point/10, dc.GetDeviceCaps(LOGPIXELSY), 72);
	_tcscpy(lf.lfFaceName, _generator_fontface);
	if (_generator_font_flags&1)
	{
		lf.lfWeight = FW_BOLD;
	}
	lf.lfItalic = (_generator_font_flags&2)?true:false;
	
	CFontDialog dlg(&lf,CF_SCREENFONTS);
	if (dlg.DoModal() == IDOK)
	{
		_tcscpy(_generator_fontface,dlg.GetFaceName());
		_generator_font_flags = 0;
		if (dlg.IsBold())
		{
			_generator_font_flags |= 1;
		}
		if (dlg.IsItalic())
		{
			_generator_font_flags |= 2;
		}
		_generator_font_point = dlg.GetSize();
		if (_generator_font_point > 320)
		{
			_generator_font_point = 320;
		}
		// get size, colour too
		SetFontNames();
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedMvFontColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_machineViewGeneratorFontColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewGeneratorFontColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MBG_MV_FONT,_machineViewGeneratorFontColor);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedMachineFontface2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	CClientDC dc(*this);
	lf.lfHeight = -MulDiv(_effect_font_point/10, dc.GetDeviceCaps(LOGPIXELSY), 72);
	_tcscpy(lf.lfFaceName, _effect_fontface);
	if (_effect_font_flags&1)
	{
		lf.lfWeight = FW_BOLD;
	}
	lf.lfItalic = (_effect_font_flags&2)?true:false;
	
	CFontDialog dlg(&lf,CF_SCREENFONTS);
	if (dlg.DoModal() == IDOK)
	{
		_tcscpy(_effect_fontface,dlg.GetFaceName());
		_effect_font_flags = 0;
		if (dlg.IsBold())
		{
			_effect_font_flags |= 1;
		}
		if (dlg.IsItalic())
		{
			_effect_font_flags |= 2;
		}
		_effect_font_point = dlg.GetSize();
		if (_effect_font_point > 320)
		{
			_effect_font_point = 320;
		}
		// get size, colour too
		SetFontNames();
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedMvFontColour2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_machineViewEffectFontColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewEffectFontColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MBG_MV_FONT2,_machineViewEffectFontColor);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedBgColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_machineViewColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MBG_CAN,_machineViewColor);
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedWireColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_machineViewWireColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewWireColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MWIRE_COL,_machineViewWireColor);
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedPolyColour(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_machineViewPolyColor);

	if(dlg.DoModal() == IDOK)
	{
		_machineViewPolyColor = dlg.GetColor();
		UpdateCanvasColour(IDC_MPOLY_COL,_machineViewPolyColor);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedMachineBitmap(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	TCHAR szPath[_MAX_PATH];       // buffer for file name
	szFile[0]='\0';
	szPath[0]='\0';

	CString str1(szBmpBkgFilename);
	int i = str1.ReverseFind('\\')+1;
	CString str2 = str1.Mid(i);
	_tcscpy(szFile,str2);

	_tcscpy(szPath,str1);
	szPath[i]=0;


	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Bitmaps (*.bmp)\0*.bmp\0");
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = szPath;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		_tcscpy(szBmpBkgFilename,szFile);
		bBmpBkg = TRUE;

		CString str1(szBmpBkgFilename);
		int i = str1.ReverseFind('\\')+1;
		CString str2 = str1.Mid(i);
		m_machine_background_bitmap.SetWindowText(str2);
	}
	else
	{
		bBmpBkg = FALSE;
		m_machine_background_bitmap.SetWindowText(SF::CResourceString(IDS_MSG0070));
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedButton24(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_vugColor);

	if(dlg.DoModal() == IDOK)
	{
		_vugColor = dlg.GetColor();
		UpdateCanvasColour(IDC_VU2_CAN,_vugColor);
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedCheckVus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_draw_vus = m_draw_vus.GetCheck() >0?true:false;
	return 0;
}

LRESULT CSkinDlg::OnBnClickedButton23(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_vubColor);

	if(dlg.DoModal() == IDOK)
	{
		_vubColor = dlg.GetColor();
		UpdateCanvasColour(IDC_VU1_CAN,_vubColor);
	}


	return 0;
}

LRESULT CSkinDlg::OnBnClickedButton25(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(_vucColor);

	if(dlg.DoModal() == IDOK)
	{
		_vucColor = dlg.GetColor();
		UpdateCanvasColour(IDC_VU3_CAN,_vucColor);
	}

	return 0;
}

LRESULT CSkinDlg::OnBnClickedDoublebuffer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_gfxbuffer = m_gfxbuffer.GetCheck() >0?true:false;

	return 0;
}

LRESULT CSkinDlg::OnBnClickedImportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)(this->GetParent());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Psycle Display Presets\0*.psv\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = _skinPathBuf;
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		FILE* hfile;
		if ((hfile=_tfopen(szFile,_T("rw"))) == NULL )
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0070)
				,SF::CResourceString(IDS_ERR_MSG0071),MB_OK);
			return FALSE;
		}
		_pattern_font_flags = 0;
		_generator_font_flags = 0;
		_effect_font_flags = 0;

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
						_tcscpy(_pattern_fontface,CA2T(q));
					}
				}
			}
			else if (strstr(buf,"\"pattern_font_point\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_pattern_font_point = _httoi(CA2T(q + 1));
				}
			}
			else if (strstr(buf,"\"pattern_font_flags\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_pattern_font_flags = _httoi(CA2T(q + 1));
				}
			}
			else if (strstr(buf,"\"pattern_font_x\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_pattern_font_x=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pattern_font_y\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_pattern_font_y=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pattern_header_skin\"=\""))
			{
				char *q = strchr(buf,61); // =
				if (q)
				{
					q+=2;
					char *p = strrchr(q,34); // "
					if (p)
					{
						p[0]=0;
						_tcscpy(_pattern_header_skin,CA2T(q));
					}
				}
			}
			else if (strstr(buf,"\"generator_fontface\"=\""))
			{
				char *q = strchr(buf,61); // =
				if (q)
				{
					q+=2;
					char *p = strrchr(q,34); // "
					if (p)
					{
						p[0]=0;
						_tcscpy(_generator_fontface,CA2T(q));
					}
				}
			}
			else if (strstr(buf,"\"generator_font_point\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_generator_font_point = _httoi(CA2T(q + 1));
				}
			}
			else if (strstr(buf,"\"generator_font_flags\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_generator_font_flags=_httoi(CA2T(q + 1));
				}
			}
			else if (strstr(buf,"\"effect_fontface\"=\""))
			{
				char *q = strchr(buf,61); // =
				if (q)
				{
					q+=2;
					char *p = strrchr(q,34); // "
					if (p)
					{
						p[0]=0;
						_tcscpy(_effect_fontface,CA2T(q));
					}
				}
			}
			else if (strstr(buf,"\"effect_font_point\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_effect_font_point=_httoi(CA2T(q + 1));
				}
			}
			else if (strstr(buf,"\"effect_font_flags\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_effect_font_flags=_httoi(CA2T(q + 1));
				}
			}
			else if (strstr(buf,"\"machine_skin\"=\""))
			{
				char *q = strchr(buf,61); // =
				if (q)
				{
					q+=2;
					char *p = strrchr(q,34); // "
					if (p)
					{
						p[0]=0;
						_tcscpy(_machine_skin,CA2T(q));
					}
				}
			}
			else if (strstr(buf,"\"machine_background\"=\""))
			{
				char *q = strchr(buf,61); // =
				if (q)
				{
					q+=2;
					char *p = strrchr(q,34); // "
					if (p)
					{
						p[0]=0;
						_tcscpy(szBmpBkgFilename,CA2T(q));
						// check for no \ in which case search for it?
						bBmpBkg = TRUE;
					}
				}
			}
			/*
			else if (strstr(buf,"\"DisplayLineNumbers\"=hex:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_linenumbers=_httoi(CA2T(q+1))?1:0;
				}
			}
			else if (strstr(buf,"\"DisplayLineNumbersHex\"=hex:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_linenumbersHex=_httoi(CA2T(q+1))?1:0;
				}
			}
			*/
			else if (strstr(buf,"\"pvc_separator\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternSeparatorColor=_httoi(CA2T(q + 1));
				}
			}
			else if (strstr(buf,"\"pvc_separator2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternSeparatorColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_background\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternViewColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_background2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_patternViewColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_font\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_font2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_fontCur\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorCur=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_fontCur2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorCur2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_fontSel\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorSel=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_fontSel2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorSel2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_fontPlay\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorPlay=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_fontPlay2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_fontColorPlay2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_row\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_rowColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_row2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_rowColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_rowbeat\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_beatColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_rowbeat2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_beatColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_row4beat\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_4beatColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_row4beat2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_4beatColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_selection\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_selectionColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_selection2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_selectionColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_playbar\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_playbarColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_playbar2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_playbarColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_cursor\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_cursorColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"pvc_cursor2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_cursorColor2=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"vu1\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_vubColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"vu2\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_vugColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"vu3\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_vucColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_colour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_wirecolour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewWireColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_polycolour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewPolyColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_wirewidth\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_wirewidth=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_wireaa\"=hex:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_wireaa=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_generator_fontcolour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewGeneratorFontColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_effect_fontcolour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewEffectFontColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"mv_triangle_size\"=hex:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_triangle_size=_httoi(CA2T(q+1));
				}
			}

			//
			//
			//
			// legacy...
			//
			//
			//
			else if (strstr(buf,"\"mv_fontcolour\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_machineViewGeneratorFontColor=_httoi(CA2T(q+1));
					_machineViewEffectFontColor=_httoi(CA2T(q+1));
				}
			}
			else if (strstr(buf,"\"machine_fontface\"=\""))
			{
				char *q = strchr(buf,61); // =
				if (q)
				{
					q+=2;
					char *p = strrchr(q,34); // "
					if (p)
					{
						p[0]=0;
						_tcscpy(_generator_fontface,CA2T(q));
						_tcscpy(_effect_fontface,CA2T(q));
					}
				}
			}
			else if (strstr(buf,"\"machine_font_point\"=dword:"))
			{
				char *q = strchr(buf,58); // :
				if (q)
				{
					_generator_font_point=_httoi(CA2T(q+1));
					_effect_font_point=_httoi(CA2T(q+1));
				}
			}
		}
		fclose(hfile);
//		m_linenumbers.SetCheck(_linenumbers);
//		m_linenumbersHex.SetCheck(_linenumbersHex);
//		_snprintf(buf,4,"%2i",_wirewidth);
//		m_wirewidth.SelectString(0,buf);
		m_wirewidth.SetCurSel(_wirewidth-1);
		m_wireaa.SetCurSel(_wireaa);
		RepaintAllCanvas();

		SetFontNames();

		m_pattern_font_x.SetCurSel(_pattern_font_x-4);
		m_pattern_font_y.SetCurSel(_pattern_font_y-4);

		int sel = m_pattern_header_skin.FindStringExact(0,_pattern_header_skin);
		if (sel==CB_ERR)
		{
			sel = m_pattern_header_skin.FindStringExact(0,DEFAULT_PATTERN_HEADER_SKIN);
		}
		m_pattern_header_skin.SetCurSel(sel);

		sel = m_machine_skin.FindStringExact(0,_machine_skin);
		if (sel==CB_ERR)
		{
			sel = m_machine_skin.FindStringExact(0,DEFAULT_MACHINE_SKIN);
		}
		m_machine_skin.SetCurSel(sel);

		if (bBmpBkg)
		{
			CString str1(szBmpBkgFilename);
			int i = str1.ReverseFind('\\')+1;
			CString str2 = str1.Mid(i);
			m_machine_background_bitmap.SetWindowText(str2);
		}
		else
		{
			m_machine_background_bitmap.SetWindowText(SF::CResourceString(IDS_ERR_MSG0095));
		}

		m_triangle_size.SetCurSel(_triangle_size - 8);
	}
	return 0;
}

LRESULT CSkinDlg::OnBnClickedExportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)GetParent();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Psycle Display Presets\0*.psv\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;	
	ofn.lpstrInitialDir = _skinPathBuf;

	if (GetSaveFileName(&ofn)==TRUE)
	{
		FILE* hfile;

		CString str = szFile;
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(_T(".psv")) != 0 ) str.Insert(str.GetLength(),_T(".psv"));
		_stprintf(szFile,str);
		DeleteFile(szFile);

		if ((hfile=_tfopen(szFile,_T("wa"))) == NULL ) // file does not exist.
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0074)
				,SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
			return FALSE;
		}

		fprintf(hfile,"[Psycle Display Presets v1.0]\n\n");

		fprintf(hfile,"\"pattern_fontface\"=\"%s\"\n",CT2A(_pattern_fontface));
		fprintf(hfile,"\"pattern_font_point\"=dword:%.8X\n",_pattern_font_point);
		fprintf(hfile,"\"pattern_font_flags\"=dword:%.8X\n",_pattern_font_flags);
		fprintf(hfile,"\"pattern_font_x\"=dword:%.8X\n",_pattern_font_x);
		fprintf(hfile,"\"pattern_font_y\"=dword:%.8X\n",_pattern_font_y);
		fprintf(hfile,"\"pattern_header_skin\"=\"%s\"\n",_pattern_header_skin);
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
		fprintf(hfile,"\"generator_fontface\"=\"%s\"\n",CT2A(_generator_fontface));
		fprintf(hfile,"\"generator_font_point\"=dword:%.8X\n",_generator_font_point);
		fprintf(hfile,"\"generator_font_flags\"=dword:%.8X\n",_generator_font_flags);
		fprintf(hfile,"\"effect_fontface\"=\"%s\"\n",CT2A(_effect_fontface));
		fprintf(hfile,"\"effect_font_point\"=dword:%.8X\n",_effect_font_point);
		fprintf(hfile,"\"effect_font_flags\"=dword:%.8X\n",_effect_font_flags);
		fprintf(hfile,"\"machine_skin\"=\"%s\"\n",_machine_skin);
		fprintf(hfile,"\"mv_colour\"=dword:%.8X\n",_machineViewColor);
		fprintf(hfile,"\"mv_wirecolour\"=dword:%.8X\n",_machineViewWireColor);
		fprintf(hfile,"\"mv_polycolour\"=dword:%.8X\n",_machineViewPolyColor);
		fprintf(hfile,"\"mv_generator_fontcolour\"=dword:%.8X\n",_machineViewGeneratorFontColor);
		fprintf(hfile,"\"mv_effect_fontcolour\"=dword:%.8X\n",_machineViewEffectFontColor);
		fprintf(hfile,"\"mv_wirewidth\"=dword:%.8X\n",_wirewidth);
		fprintf(hfile,"\"mv_wireaa\"=hex:%.2X\n",_wireaa);
		fprintf(hfile,"\"machine_background\"=\"%s\"\n",CT2A(szBmpBkgFilename));
		fprintf(hfile,"\"mv_triangle_size\"=hex:%.2X\n",_triangle_size);

		fclose(hfile);
	}
	return 0;
}

LRESULT CSkinDlg::OnCbnSelchangeMachineSkin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_machine_skin.GetLBText(m_machine_skin.GetCurSel(),_machine_skin);
	return 0;
}

LRESULT CSkinDlg::OnBnClickedDrawMacIndex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_draw_mac_index = m_draw_mac_index.GetCheck() >0?true:false;
	return 0;

}

LRESULT CSkinDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ( (UINT)wParam == 2345 )
	{
		RepaintAllCanvas();
	}
	
	//OnTimer(nIDEvent);

	return 0;
}

LRESULT CSkinDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	KillTimer(2345);
	//CPropertyPage::OnClose();
	return 0;
}

LRESULT CSkinDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_machine_background_bitmap.Attach(GetDlgItem(IDC_MACHINE_BITMAP));
	m_triangle_size.Attach(GetDlgItem(IDC_TRIANGLESIZE));
	m_wirewidth.Attach(GetDlgItem(IDC_WIRE_WIDTH));
	m_pattern_fontface.Attach(GetDlgItem(IDC_PATTERN_FONTFACE));
	m_pattern_font_point.Attach(GetDlgItem(IDC_PATTERN_FONT_POINT));
	m_pattern_font_x.Attach(GetDlgItem(IDC_PATTERN_FONT_X));
	m_pattern_font_y.Attach(GetDlgItem(IDC_PATTERN_FONT_Y));
	m_pattern_header_skin.Attach(GetDlgItem(IDC_PATTERN_HEADER_SKIN));
	m_machine_skin.Attach(GetDlgItem(IDC_MACHINE_SKIN));
	m_generator_fontface.Attach(GetDlgItem(IDC_MACHINE_FONTFACE));
	m_generator_font_point.Attach(GetDlgItem(IDC_MACHINE_FONT_POINT));
	m_effect_fontface.Attach(GetDlgItem(IDC_MACHINE_FONTFACE2));
	m_effect_font_point.Attach(GetDlgItem(IDC_MACHINE_FONT_POINT2));
	m_gfxbuffer.Attach(GetDlgItem(IDC_DOUBLEBUFFER));
	m_draw_vus.Attach(GetDlgItem(IDC_CHECK_VUS));
	m_linenumbers.Attach(GetDlgItem(IDC_LINE_NUMBERS));
	m_linenumbersHex.Attach(GetDlgItem(IDC_LINE_NUMBERS_HEX));
	m_linenumbersCursor.Attach(GetDlgItem(IDC_LINE_NUMBERS_CURSOR));
	m_pattern_draw_empty_data.Attach(GetDlgItem(IDC_DRAW_EMPTY_DATA));
	m_draw_mac_index.Attach(GetDlgItem(IDC_DRAW_MAC_INDEX));
	m_wireaa.Attach(GetDlgItem(IDC_WIREAA));

	m_gfxbuffer.SetCheck(_gfxbuffer);
	m_draw_vus.SetCheck(_draw_vus);
	m_linenumbers.SetCheck(_linenumbers);
	m_linenumbersHex.SetCheck(_linenumbersHex);
	m_linenumbersCursor.SetCheck(_linenumbersCursor);
	m_pattern_draw_empty_data.SetCheck(_pattern_draw_empty_data);
	m_draw_mac_index.SetCheck(_draw_mac_index);
	SetTimer(2345,50,0);

	TCHAR s[4];
	m_wireaa.AddString(_T("off"));
	for (int i = 1; i <= 16; i++)
	{
		_stprintf(s,_T("%2i"),i);
		m_wirewidth.AddString(s);
		m_wireaa.AddString(s);
	}
	m_wirewidth.SetCurSel(_wirewidth-1);
	m_wireaa.SetCurSel(_wireaa);

	for(i=4;i<=64;i++)
	{
		TCHAR s[4];
		_stprintf(s,_T("%2i"),i);
		m_pattern_font_x.AddString(s);
		m_pattern_font_y.AddString(s);
	}
	m_pattern_font_x.SetCurSel(_pattern_font_x-4);
	m_pattern_font_y.SetCurSel(_pattern_font_y-4);

	for (i = 50; i <= 320; i+=5)
	{
		TCHAR s[8];
		if (i < 100)
		{
			_stprintf(s,_T(" %.1f"),float(i)/10.0f);
		}
		else
		{
			_stprintf(s,_T("%.1f"),float(i)/10.0f);
		}
		m_pattern_font_point.AddString(s);
		m_generator_font_point.AddString(s);
		m_effect_font_point.AddString(s);
	}

	SetFontNames();

	m_pattern_header_skin.AddString(DEFAULT_PATTERN_HEADER_SKIN);
	m_machine_skin.AddString(DEFAULT_MACHINE_SKIN);
	
	// ok now browse our folder for skins

	FindSkinsInDir(Global::pConfig->GetInitialSkinDir());

	int sel = m_pattern_header_skin.FindStringExact(0,_pattern_header_skin);
	if (sel==CB_ERR)
	{
		sel = m_pattern_header_skin.FindStringExact(0,DEFAULT_PATTERN_HEADER_SKIN);
	}
	m_pattern_header_skin.SetCurSel(sel);

	sel = m_machine_skin.FindStringExact(0,_machine_skin);
	if (sel==CB_ERR)
	{
		sel = m_machine_skin.FindStringExact(0,DEFAULT_MACHINE_SKIN);
	}
	m_machine_skin.SetCurSel(sel);

	if (bBmpBkg)
	{
		CString str1(szBmpBkgFilename);
		int i = str1.ReverseFind('\\')+1;
		CString str2 = str1.Mid(i);
		m_machine_background_bitmap.SetWindowText(str2);
	}
	else
	{
		m_machine_background_bitmap.SetWindowText(SF::CResourceString(IDS_ERR_MSG0095));
	}

	for (i = 8; i <= 64; i++)
	{
		_stprintf(s,_T("%2i"),i);
		m_triangle_size.AddString(s);
	}
	m_triangle_size.SetCurSel(_triangle_size-8);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
