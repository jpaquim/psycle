// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief implementation file for psycle::host::CSkinDlg.
#include <psycle/project.private.hpp>
#include "SkinDlg.hpp"
#include "psycle.hpp"
#include "Helpers.hpp"
#include "Configuration.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		#define MAX_FONTS 256

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
			DDX_Control(pDX, IDC_MACHINE_BITMAP, m_machine_background_bitmap);
			DDX_Control(pDX, IDC_MACHINEGUI_BITMAP, m_machine_GUI_bitmap);
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
			ON_BN_CLICKED(IDC_MACHINEGUI_TOPFONTC, OnBnClickedMachineguiFontc)
			ON_BN_CLICKED(IDC_MACHINEGUI_TOPC, OnBnClickedMachineguiTopc)
			ON_BN_CLICKED(IDC_MACHINEGUI_BOTTOMC, OnBnClickedMachineguiBottomc)
			ON_BN_CLICKED(IDC_MACHINEGUI_BOTTOMFONTC, OnBnClickedMachineguiBottomfontc)
			ON_BN_CLICKED(IDC_MACHINEGUI_TITLEC, OnBnClickedMachineguiTitlec)
			ON_BN_CLICKED(IDC_MACHINEGUI_TITLEFONTC, OnBnClickedMachineguiTitlefontc2)
			ON_BN_CLICKED(IDC_MACHINEGUI_BITMAP, OnBnClickedMachineguiBitmap)
			ON_BN_CLICKED(IDC_MACHINEGUI_TOPC2, OnBnClickedMachineguiTopc2)
			ON_BN_CLICKED(IDC_MACHINEGUI_TOPFONTC2, OnBnClickedMachineguiTopfontc2)
			ON_BN_CLICKED(IDC_MACHINEGUI_BOTTOMC2, OnBnClickedMachineguiBottomc2)
			ON_BN_CLICKED(IDC_MACHINEGUI_BOTTOMFONTC2, OnBnClickedMachineguiBottomfontc2)
		END_MESSAGE_MAP()

		void CSkinDlg::OnOK()
		{
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
			m_draw_vus.SetCheck(_draw_vus);
			m_linenumbers.SetCheck(_linenumbers);
			m_linenumbersHex.SetCheck(_linenumbersHex);
			m_linenumbersCursor.SetCheck(_linenumbersCursor);
			m_pattern_draw_empty_data.SetCheck(_pattern_draw_empty_data);
			m_draw_mac_index.SetCheck(_draw_mac_index);
			SetTimer(2345,50,0);

			char s[4];
			m_wireaa.AddString("off");
			for (int i = 1; i <= 16; i++)
			{
				sprintf(s,"%2i",i);
				m_wirewidth.AddString(s);
				m_wireaa.AddString(s);
			}
			m_wirewidth.SetCurSel(_wirewidth-1);
			m_wireaa.SetCurSel(_wireaa);

			for(int i=4;i<=64;i++)
			{
				char s[4];
				sprintf(s,"%2i",i);
				m_pattern_font_x.AddString(s);
				m_pattern_font_y.AddString(s);
			}
			m_pattern_font_x.SetCurSel(_pattern_font_x-4);
			m_pattern_font_y.SetCurSel(_pattern_font_y-4);

			for (int i = 50; i <= 320; i+=5)
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
				m_generator_font_point.AddString(s);
				m_effect_font_point.AddString(s);
			}

			SetFontNames();

			m_pattern_header_skin.AddString(PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN);
			m_machine_skin.AddString(PSYCLE__PATH__DEFAULT_MACHINE_SKIN);
			
			// ok now browse our folder for skins
			FindSkinsInDir(Global::pConfig->GetSkinDir().c_str());

			int sel = m_pattern_header_skin.FindStringExact(0,_pattern_header_skin.c_str());
			if (sel==CB_ERR)
			{
				sel = m_pattern_header_skin.FindStringExact(0,PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN);
			}
			m_pattern_header_skin.SetCurSel(sel);

			sel = m_machine_skin.FindStringExact(0,_machine_skin.c_str());
			if (sel==CB_ERR)
			{
				sel = m_machine_skin.FindStringExact(0,PSYCLE__PATH__DEFAULT_MACHINE_SKIN);
			}
			m_machine_skin.SetCurSel(sel);

			if (bBmpBkg)
			{
				CString str1(szBmpBkgFilename.c_str());
				int i = str1.ReverseFind('\\')+1;
				CString str2 = str1.Mid(i);
				m_machine_background_bitmap.SetWindowText(str2);
			}
			else
			{
				m_machine_background_bitmap.SetWindowText("No Background Bitmap");
			}

			if (bBmpDial)
			{
				CString str1(szBmpDialFilename.c_str());
				int i = str1.ReverseFind('\\')+1;
				CString str2 = str1.Mid(i);
				m_machine_GUI_bitmap.SetWindowText(str2);
			}
			else
			{
				m_machine_GUI_bitmap.SetWindowText("Default Dial Bitmap");
			}

			for (int i = 8; i <= 64; i++)
			{
				sprintf(s,"%2i",i);
				m_triangle_size.AddString(s);
			}
			m_triangle_size.SetCurSel(_triangle_size-8);

			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CSkinDlg::FindSkinsInDir(CString findDir)
		{
			CFileFind finder;

			int loop = finder.FindFile(findDir + "\\*.");	// check for subfolders.
			while (loop) 
			{								// Note: Subfolders with dots won't work.
				loop = finder.FindNextFile();
				if(finder.IsDirectory() && !finder.IsDots())
				{
					FindSkinsInDir(finder.GetFilePath());
				}
			}
			finder.Close();

			loop = finder.FindFile(findDir + "\\*.psh"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if(!finder.IsDirectory())
				{
					CString sName, tmpPath;
					sName = finder.GetFileName();
					// ok so we have a .psh, does it have a valid matching .bmp?
					char szBmpName[MAX_PATH];
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					sprintf(szBmpName,"%s\\%s.bmp",findDir,sName);
					HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szBmpName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
					if (hBitmap)
					{
						m_pattern_header_skin.AddString(sName);
					}
					DeleteObject(hBitmap);
				}
			}

			loop = finder.FindFile(findDir + "\\*.psm"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if(!finder.IsDirectory())
				{
					CString sName, tmpPath;
					sName = finder.GetFileName();
					// ok so we have a .psh, does it have a valid matching .bmp?
					char szBmpName[MAX_PATH];
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					sprintf(szBmpName,"%s\\%s.bmp",findDir,sName);
					HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szBmpName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
					if (hBitmap)
					{
						m_machine_skin.AddString(sName);
					}
					DeleteObject(hBitmap);
				}
			}
			finder.Close();
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

		void CSkinDlg::OnCheckVus() 
		{
			_draw_vus = m_draw_vus.GetCheck() >0?true:false;
		}

		void CSkinDlg::OnLineNumbers() 
		{
			_linenumbers = m_linenumbers.GetCheck() >0?true:false;
		}

		void CSkinDlg::OnLineNumbersHex() 
		{
			_linenumbersHex = m_linenumbersHex.GetCheck() >0?true:false;
		}

		void CSkinDlg::OnLineNumbersCursor() 
		{
			_linenumbersCursor = m_linenumbersCursor.GetCheck() >0?true:false;
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
			UpdateCanvasColour(IDC_MACHINETOP_CAN,_machineGUITopColor);
			UpdateCanvasColour(IDC_MACHINEFONTTOP_CAN,_machineGUIFontTopColor);
			UpdateCanvasColour(IDC_MACHINEBOTTOM_CAN,_machineGUIBottomColor);
			UpdateCanvasColour(IDC_MACHINEFONTBOTTOM_CAN,_machineGUIFontBottomColor);
			UpdateCanvasColour(IDC_MACHINETOP_CAN2,_machineGUIHTopColor);
			UpdateCanvasColour(IDC_MACHINEFONTTOP_CAN2,_machineGUIHFontTopColor);
			UpdateCanvasColour(IDC_MACHINEBOTTOM_CAN2,_machineGUIHBottomColor);
			UpdateCanvasColour(IDC_MACHINEFONTBOTTOM_CAN2,_machineGUIHFontBottomColor);
			UpdateCanvasColour(IDC_MACHINETITLE_CAN,_machineGUITitleColor);
			UpdateCanvasColour(IDC_MACHINEFONTTITLE_CAN,_machineGUITitleFontColor);		
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
			OPENFILENAME ofn = OPENFILENAME(); // common dialog box structure
			char szFile[MAX_PATH]; // buffer for file name
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof szFile;
			ofn.lpstrFilter = "Psycle Display Presets\0*.psv\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = 0;
			ofn.nMaxFileTitle = 0;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			ofn.lpstrInitialDir = _skinPathBuf.c_str();
			// Display the Open dialog box. 
			if(::GetOpenFileName(&ofn))
			{
				std::FILE * hfile;
				if(!(hfile=std::fopen(szFile,"r")))
				{
					::MessageBox(0, "Couldn't open File for Reading. Operation Aborted", "File Open Error", MB_ICONERROR | MB_OK);
					return;
				}
				_pattern_font_flags = 0;
				_generator_font_flags = 0;
				_effect_font_flags = 0;

				char buf[1 << 10];
				while(std::fgets(buf, sizeof buf, hfile))
				{
					using helpers::hexstring_to_integer;
					#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION <= 1300
						// msvc 7.1 crashes because of the number of 'else if' statements
						// so, we can't use 'else'
						#define else
					#endif
					// this is a horror of repetitive code :-(
					if (std::strstr(buf,"\"pattern_fontface\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = std::strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								_pattern_fontface = q;
							}
						}
					}
					else if (std::strstr(buf,"\"pattern_font_point\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _pattern_font_point);
						}
					}
					else if (std::strstr(buf,"\"pattern_font_flags\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _pattern_font_flags);
						}
					}
					else if (std::strstr(buf,"\"pattern_font_x\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _pattern_font_x);
						}
					}
					else if (std::strstr(buf,"\"pattern_font_y\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _pattern_font_y);
						}
					}
					else if (std::strstr(buf,"\"pattern_header_skin\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								_pattern_header_skin = q;
							}
						}
					}
					else if (std::strstr(buf,"\"generator_fontface\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								_generator_fontface = q;
							}
						}
					}
					else if (std::strstr(buf,"\"generator_font_point\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _generator_font_point);
						}
					}
					else if (std::strstr(buf,"\"generator_font_flags\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _generator_font_flags);
						}
					}
					else if (std::strstr(buf,"\"effect_fontface\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								_effect_fontface = q;
							}
						}
					}
					else if (std::strstr(buf,"\"effect_font_point\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _effect_font_point);
						}
					}
					else if (std::strstr(buf,"\"effect_font_flags\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _effect_font_flags);
						}
					}
					else if (std::strstr(buf,"\"machine_skin\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								_machine_skin = q;
							}
						}
					}
					else if (std::strstr(buf,"\"machine_background\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								szBmpBkgFilename = q;
								// check for no \ in which case search for it?
								bBmpBkg = TRUE;
							}
						}
					}
					else if (std::strstr(buf,"\"machine_GUI_bitmap\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								szBmpDialFilename = q;
								// check for no \ in which case search for it?
								bBmpDial = TRUE;
							}
						}
					}
					/*
					else if (std::strstr(buf,"\"DisplayLineNumbers\"=hex:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _linenumbers) ? 1 : 0;
						}
					}
					else if (std::strstr(buf,"\"DisplayLineNumbersHex\"=hex:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _linenumbersHex) ? 1 : 0;
						}
					}
					*/
					else if (std::strstr(buf,"\"pvc_separator\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _patternSeparatorColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_separator2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _patternSeparatorColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_background\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _patternViewColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_background2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _patternViewColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_font\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_font2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_fontCur\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColorCur);
						}
					}
					else if (std::strstr(buf,"\"pvc_fontCur2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColorCur2);
						}
					}
					else if (std::strstr(buf,"\"pvc_fontSel\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColorSel);
						}
					}
					else if (std::strstr(buf,"\"pvc_fontSel2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColorSel2);
						}
					}
					else if (std::strstr(buf,"\"pvc_fontPlay\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColorPlay);
						}
					}
					else if (std::strstr(buf,"\"pvc_fontPlay2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _fontColorPlay2);
						}
					}
					else if (std::strstr(buf,"\"pvc_row\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _rowColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_row2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _rowColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_rowbeat\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _beatColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_rowbeat2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _beatColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_row4beat\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _4beatColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_row4beat2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _4beatColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_selection\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _selectionColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_selection2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _selectionColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_playbar\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _playbarColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_playbar2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _playbarColor2);
						}
					}
					else if (std::strstr(buf,"\"pvc_cursor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _cursorColor);
						}
					}
					else if (std::strstr(buf,"\"pvc_cursor2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _cursorColor2);
						}
					}
					else if (std::strstr(buf,"\"vu1\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _vubColor);
						}
					}
					else if (std::strstr(buf,"\"vu2\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _vugColor);
						}
					}
					else if (std::strstr(buf,"\"vu3\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _vucColor);
						}
					}
					else if (std::strstr(buf,"\"mv_colour\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineViewColor);
						}
					}
					else if (std::strstr(buf,"\"mv_wirecolour\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineViewWireColor);
						}
					}
					else if (std::strstr(buf,"\"mv_polycolour\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineViewPolyColor);
						}
					}
					else if (std::strstr(buf,"\"mv_wirewidth\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _wirewidth);
						}
					}
					else if (std::strstr(buf,"\"mv_wireaa\"=hex:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _wireaa);
						}
					}
					else if (std::strstr(buf,"\"mv_generator_fontcolour\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineViewGeneratorFontColor);
						}
					}
					else if (std::strstr(buf,"\"mv_effect_fontcolour\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineViewEffectFontColor);
						}
					}
					else if (std::strstr(buf,"\"mv_triangle_size\"=hex:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _triangle_size);
						}
					}
					else if (std::strstr(buf,"\"mv_effect_fontcolour\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineViewEffectFontColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUITopColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUITopColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUIFontTopColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUIFontTopColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUIBottomColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUIBottomColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUIFontBottomColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUIFontBottomColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUIHTopColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUIHTopColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUIHFontTopColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUIHFontTopColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUIHBottomColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUIHBottomColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUIHFontBottomColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUIHFontBottomColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUITitleColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUITitleColor);
						}
					}
					else if (std::strstr(buf,"\"machineGUITitleFontColor\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineGUITitleFontColor);
						}
					}
					//
					//
					//
					// legacy...
					//
					//
					//
					else if (std::strstr(buf,"\"mv_fontcolour\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _machineViewGeneratorFontColor);
							hexstring_to_integer(q+1, _machineViewEffectFontColor);
						}
					}
					else if (std::strstr(buf,"\"machine_fontface\"=\""))
					{
						char *q = std::strchr(buf,61); // =
						if (q)
						{
							q+=2;
							char *p = strrchr(q,34); // "
							if (p)
							{
								p[0]=0;
								_generator_fontface = q;
								_effect_fontface = q;
							}
						}
					}
					else if (std::strstr(buf,"\"machine_font_point\"=dword:"))
					{
						char *q = std::strchr(buf,58); // :
						if (q)
						{
							hexstring_to_integer(q+1, _generator_font_point);
							hexstring_to_integer(q+1, _effect_font_point);
						}
					}
					#if defined else // this is the case for DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION <= 1300
						// msvc 7.1 crashes because of the number of 'else if' statements
						// so, we can't use 'else'
						#undef else
					#endif
				}
				std::fclose(hfile);
				//m_linenumbers.SetCheck(_linenumbers);
				//m_linenumbersHex.SetCheck(_linenumbersHex);
				//::snprintf(buf,4,"%2i",_wirewidth);
				//m_wirewidth.SelectString(0,buf);
				m_wirewidth.SetCurSel(_wirewidth-1);
				m_wireaa.SetCurSel(_wireaa);
				RepaintAllCanvas();
				SetFontNames();
				m_pattern_font_x.SetCurSel(_pattern_font_x-4);
				m_pattern_font_y.SetCurSel(_pattern_font_y-4);
				int sel = m_pattern_header_skin.FindStringExact(0,_pattern_header_skin.c_str());
				if (sel==CB_ERR)
				{
					sel = m_pattern_header_skin.FindStringExact(0,PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN);
				}
				m_pattern_header_skin.SetCurSel(sel);
				sel = m_machine_skin.FindStringExact(0,_machine_skin.c_str());
				if (sel==CB_ERR)
				{
					sel = m_machine_skin.FindStringExact(0,PSYCLE__PATH__DEFAULT_MACHINE_SKIN);
				}
				m_machine_skin.SetCurSel(sel);
				if (bBmpBkg)
				{
					CString str1(szBmpBkgFilename.c_str());
					int i = str1.ReverseFind('\\')+1;
					CString str2 = str1.Mid(i);
					m_machine_background_bitmap.SetWindowText(str2);
				}
				else
				{
					m_machine_background_bitmap.SetWindowText("No Background Bitmap");
				}
				if (bBmpDial)
				{
					CString str1(szBmpDialFilename.c_str());
					int i = str1.ReverseFind('\\')+1;
					CString str2 = str1.Mid(i);
					m_machine_GUI_bitmap.SetWindowText(str2);
				}
				else
				{
					m_machine_GUI_bitmap.SetWindowText("Default Dial Bitmap");
				}
				m_triangle_size.SetCurSel(_triangle_size-8);
			}
		}

		void CSkinDlg::OnExportReg() 
		{
			OPENFILENAME ofn = OPENFILENAME(); // common dialog box structure
			char szFile[MAX_PATH]; // buffer for file name
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof szFile;
			ofn.lpstrFilter = "Psycle Display Presets\0*.psv\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = 0;
			ofn.nMaxFileTitle = 0;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;	
			ofn.lpstrInitialDir = _skinPathBuf.c_str();
			if(::GetSaveFileName(&ofn))
			{
				std::FILE * hfile;
				::CString str = szFile;
				::CString str2 = str.Right(4);
				if(str2.CompareNoCase(".psv")) str.Insert(str.GetLength(),".psv");
				std::sprintf(szFile,str);
				if(!(hfile=std::fopen(szFile,"wb")))
				{
					::MessageBox(0, "Couldn't open File for Writing. Operation Aborted", "File Save Error", MB_ICONERROR | MB_OK);
					return;
				}
				std::fprintf(hfile,"[Psycle Display Presets v1.0]\n\n");
				std::fprintf(hfile,"\"pattern_fontface\"=\"%s\"\n",_pattern_fontface.c_str());
				std::fprintf(hfile,"\"pattern_font_point\"=dword:%.8X\n",_pattern_font_point);
				std::fprintf(hfile,"\"pattern_font_flags\"=dword:%.8X\n",_pattern_font_flags);
				std::fprintf(hfile,"\"pattern_font_x\"=dword:%.8X\n",_pattern_font_x);
				std::fprintf(hfile,"\"pattern_font_y\"=dword:%.8X\n",_pattern_font_y);
				std::fprintf(hfile,"\"pattern_header_skin\"=\"%s\"\n",_pattern_header_skin.c_str());
				//std::fprintf(hfile,"\"DisplayLineNumbers\"=hex:%.2X\n",_linenumbers?1:0);
				//std::fprintf(hfile,"\"DisplayLineNumbersHex\"=hex:%.2X\n",_linenumbersHex?1:0);
				std::fprintf(hfile,"\"pvc_separator\"=dword:%.8X\n",_patternSeparatorColor);
				std::fprintf(hfile,"\"pvc_separator2\"=dword:%.8X\n",_patternSeparatorColor2);
				std::fprintf(hfile,"\"pvc_background\"=dword:%.8X\n",_patternViewColor);
				std::fprintf(hfile,"\"pvc_background2\"=dword:%.8X\n",_patternViewColor2);
				std::fprintf(hfile,"\"pvc_font\"=dword:%.8X\n",_fontColor);
				std::fprintf(hfile,"\"pvc_font2\"=dword:%.8X\n",_fontColor2);
				std::fprintf(hfile,"\"pvc_fontCur\"=dword:%.8X\n",_fontColorCur);
				std::fprintf(hfile,"\"pvc_fontCur2\"=dword:%.8X\n",_fontColorCur2);
				std::fprintf(hfile,"\"pvc_fontSel\"=dword:%.8X\n",_fontColorSel);
				std::fprintf(hfile,"\"pvc_fontSel2\"=dword:%.8X\n",_fontColorSel2);
				std::fprintf(hfile,"\"pvc_fontPlay\"=dword:%.8X\n",_fontColorPlay);
				std::fprintf(hfile,"\"pvc_fontPlay2\"=dword:%.8X\n",_fontColorPlay2);
				std::fprintf(hfile,"\"pvc_row\"=dword:%.8X\n",_rowColor);
				std::fprintf(hfile,"\"pvc_row2\"=dword:%.8X\n",_rowColor2);
				std::fprintf(hfile,"\"pvc_rowbeat\"=dword:%.8X\n",_beatColor);
				std::fprintf(hfile,"\"pvc_rowbeat2\"=dword:%.8X\n",_beatColor2);
				std::fprintf(hfile,"\"pvc_row4beat\"=dword:%.8X\n",_4beatColor);
				std::fprintf(hfile,"\"pvc_row4beat2\"=dword:%.8X\n",_4beatColor2);
				std::fprintf(hfile,"\"pvc_selection\"=dword:%.8X\n",_selectionColor);
				std::fprintf(hfile,"\"pvc_selection2\"=dword:%.8X\n",_selectionColor2);
				std::fprintf(hfile,"\"pvc_playbar\"=dword:%.8X\n",_playbarColor);
				std::fprintf(hfile,"\"pvc_playbar2\"=dword:%.8X\n",_playbarColor2);
				std::fprintf(hfile,"\"pvc_cursor\"=dword:%.8X\n",_cursorColor);
				std::fprintf(hfile,"\"pvc_cursor2\"=dword:%.8X\n",_cursorColor2);
				std::fprintf(hfile,"\"vu1\"=dword:%.8X\n",_vubColor);
				std::fprintf(hfile,"\"vu2\"=dword:%.8X\n",_vugColor);
				std::fprintf(hfile,"\"vu3\"=dword:%.8X\n",_vucColor);
				std::fprintf(hfile,"\"generator_fontface\"=\"%s\"\n",_generator_fontface.c_str());
				std::fprintf(hfile,"\"generator_font_point\"=dword:%.8X\n",_generator_font_point);
				std::fprintf(hfile,"\"generator_font_flags\"=dword:%.8X\n",_generator_font_flags);
				std::fprintf(hfile,"\"effect_fontface\"=\"%s\"\n",_effect_fontface.c_str());
				std::fprintf(hfile,"\"effect_font_point\"=dword:%.8X\n",_effect_font_point);
				std::fprintf(hfile,"\"effect_font_flags\"=dword:%.8X\n",_effect_font_flags);
				std::fprintf(hfile,"\"machine_skin\"=\"%s\"\n",_machine_skin.c_str());
				std::fprintf(hfile,"\"mv_colour\"=dword:%.8X\n",_machineViewColor);
				std::fprintf(hfile,"\"mv_wirecolour\"=dword:%.8X\n",_machineViewWireColor);
				std::fprintf(hfile,"\"mv_polycolour\"=dword:%.8X\n",_machineViewPolyColor);
				std::fprintf(hfile,"\"mv_generator_fontcolour\"=dword:%.8X\n",_machineViewGeneratorFontColor);
				std::fprintf(hfile,"\"mv_effect_fontcolour\"=dword:%.8X\n",_machineViewEffectFontColor);
				std::fprintf(hfile,"\"mv_wirewidth\"=dword:%.8X\n",_wirewidth);
				std::fprintf(hfile,"\"mv_wireaa\"=hex:%.2X\n",_wireaa);
				std::fprintf(hfile,"\"machine_background\"=\"%s\"\n",szBmpBkgFilename.c_str());
				std::fprintf(hfile,"\"machine_GUI_bitmap\"=\"%s\"\n",szBmpDialFilename.c_str());
				std::fprintf(hfile,"\"mv_triangle_size\"=hex:%.2X\n",_triangle_size);
				std::fprintf(hfile,"\"machineGUITopColor\"=dword:%.8X\n",_machineGUITopColor);
				std::fprintf(hfile,"\"machineGUIFontTopColor\"=dword:%.8X\n",_machineGUIFontTopColor);
				std::fprintf(hfile,"\"machineGUIBottomColor\"=dword:%.8X\n",_machineGUIBottomColor);
				std::fprintf(hfile,"\"machineGUIFontBottomColor\"=dword:%.8X\n",_machineGUIFontBottomColor);
				std::fprintf(hfile,"\"machineGUIHTopColor\"=dword:%.8X\n",_machineGUIHTopColor);
				std::fprintf(hfile,"\"machineGUIHFontTopColor\"=dword:%.8X\n",_machineGUIHFontTopColor);
				std::fprintf(hfile,"\"machineGUIHBottomColor\"=dword:%.8X\n",_machineGUIHBottomColor);
				std::fprintf(hfile,"\"machineGUIHFontBottomColor\"=dword:%.8X\n",_machineGUIHFontBottomColor);
				std::fprintf(hfile,"\"machineGUITitleColor\"=dword:%.8X\n",_machineGUITitleColor);
				std::fprintf(hfile,"\"machineGUITitleFontColor\"=dword:%.8X\n",_machineGUITitleFontColor);
				std::fclose(hfile);
			}
		}

		void CSkinDlg::OnSelchangePatternFontPoint() 
		{
			_pattern_font_point=(m_pattern_font_point.GetCurSel()*5)+50;
		}

		void CSkinDlg::OnSelchangePatternFontX() 
		{
			_pattern_font_x=m_pattern_font_x.GetCurSel()+4;
		}

		void CSkinDlg::OnSelchangePatternFontY() 
		{
			_pattern_font_y=m_pattern_font_y.GetCurSel()+4;
		}

		void CSkinDlg::OnPatternFontFace()
		{
			LOGFONT lf;
			std::memset(&lf, 0, sizeof lf);
			CClientDC dc(this);
			lf.lfHeight = -MulDiv(_pattern_font_point/10, dc.GetDeviceCaps(LOGPIXELSY), 72);
			strcpy(lf.lfFaceName, _pattern_fontface.c_str());
			if (_pattern_font_flags&1)
			{
				lf.lfWeight = FW_BOLD;
			}
			lf.lfItalic = (_pattern_font_flags&2)?true:false;
			
			CFontDialog dlg(&lf,CF_SCREENFONTS);
			if (dlg.DoModal() == IDOK)
			{
				_pattern_fontface = dlg.GetFaceName();
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
		}
		
		static std::string describeFont(std::string name, UINT flags)
		{
			std::string fontDesc = name;
			if (flags & 1)
				fontDesc += " Bold";
			if (flags & 2)
				fontDesc += " Italic";
			return fontDesc;
		}

		void CSkinDlg::SetFontNames()
		{
			m_pattern_fontface.SetWindowText(describeFont(_pattern_fontface,_pattern_font_flags).c_str());
			m_generator_fontface.SetWindowText(describeFont(_generator_fontface,_generator_font_flags).c_str());
			m_effect_fontface.SetWindowText(describeFont(_effect_fontface,_effect_font_flags).c_str());

			m_pattern_font_point.SetCurSel((_pattern_font_point-50)/5);
			m_generator_font_point.SetCurSel((_generator_font_point-50)/5);
			m_effect_font_point.SetCurSel((_effect_font_point-50)/5);
		}

		void CSkinDlg::OnSelchangePatternHeaderSkin()
		{
			CString temp;
			m_pattern_header_skin.GetLBText(m_pattern_header_skin.GetCurSel(),temp);
			_pattern_header_skin=temp;
		}

		void CSkinDlg::OnSelchangeWireWidth()
		{
			_wirewidth = m_wirewidth.GetCurSel()+1;
		}

		void CSkinDlg::OnSelchangeWireAA()
		{
			_wireaa = m_wireaa.GetCurSel();
		}


		void CSkinDlg::OnSelchangeGeneratorFontPoint() 
		{
			_generator_font_point=(m_generator_font_point.GetCurSel()*5)+50;
		}

		void CSkinDlg::OnGeneratorFontFace()
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));

			CClientDC dc(this);
			lf.lfHeight = -MulDiv(_generator_font_point/10, dc.GetDeviceCaps(LOGPIXELSY), 72);
			strcpy(lf.lfFaceName, _generator_fontface.c_str());
			if (_generator_font_flags&1)
			{
				lf.lfWeight = FW_BOLD;
			}
			lf.lfItalic = (_generator_font_flags&2)?true:false;
			
			CFontDialog dlg(&lf,CF_SCREENFONTS);
			if (dlg.DoModal() == IDOK)
			{
				_generator_fontface = dlg.GetFaceName();
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
		}


		void CSkinDlg::OnSelchangeEffectFontPoint() 
		{
			_effect_font_point=(m_effect_font_point.GetCurSel()*5)+50;
		}

		void CSkinDlg::OnEffectFontFace()
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));

			CClientDC dc(this);
			lf.lfHeight = -MulDiv(_effect_font_point/10, dc.GetDeviceCaps(LOGPIXELSY), 72);
			strcpy(lf.lfFaceName, _effect_fontface.c_str());
			if (_effect_font_flags&1)
			{
				lf.lfWeight = FW_BOLD;
			}
			lf.lfItalic = (_effect_font_flags&2)?true:false;
			
			CFontDialog dlg(&lf,CF_SCREENFONTS);
			if (dlg.DoModal() == IDOK)
			{
				_effect_fontface = dlg.GetFaceName();
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
		}


		void CSkinDlg::OnSelchangeMachineSkin()
		{
			CString temp;
			m_machine_skin.GetLBText(m_machine_skin.GetCurSel(),temp);
			_machine_skin = temp;
		}

		void CSkinDlg::OnMVGeneratorFontColour() 
		{
			CColorDialog dlg(_machineViewGeneratorFontColor);

			if(dlg.DoModal() == IDOK)
			{
				_machineViewGeneratorFontColor = dlg.GetColor();
				UpdateCanvasColour(IDC_MBG_MV_FONT,_machineViewGeneratorFontColor);
			}
		}

		void CSkinDlg::OnMVEffectFontColour() 
		{
			CColorDialog dlg(_machineViewEffectFontColor);

			if(dlg.DoModal() == IDOK)
			{
				_machineViewEffectFontColor = dlg.GetColor();
				UpdateCanvasColour(IDC_MBG_MV_FONT2,_machineViewEffectFontColor);
			}
		}

		void CSkinDlg::OnBnClickedMachineguiFontc()
		{
			CColorDialog dlg(_machineGUIFontTopColor);

			if(dlg.DoModal() == IDOK)
			{
				_machineGUIFontTopColor = dlg.GetColor();
				UpdateCanvasColour(IDC_MACHINEFONTTOP_CAN,_machineGUIFontTopColor);
			}	
		}

		void CSkinDlg::OnBnClickedMachineguiBottomfontc()
		{
				CColorDialog dlg(_machineGUIFontBottomColor);

				if(dlg.DoModal() == IDOK)
				{
					_machineGUIFontBottomColor = dlg.GetColor();
					UpdateCanvasColour(IDC_MACHINEFONTBOTTOM_CAN,_machineGUIFontBottomColor);
				}	
		}

		void CSkinDlg::OnBnClickedMachineguiTopc()
		{
			CColorDialog dlg(_machineGUITopColor);

			if(dlg.DoModal() == IDOK)
			{
				_machineGUITopColor = dlg.GetColor();
				UpdateCanvasColour(IDC_MACHINETOP_CAN,_machineGUITopColor);
			}	
		}

		void CSkinDlg::OnBnClickedMachineguiBottomc()
		{
			CColorDialog dlg(_machineGUIBottomColor);

			if(dlg.DoModal() == IDOK)
			{
				_machineGUIBottomColor = dlg.GetColor();
				UpdateCanvasColour(IDC_MACHINEBOTTOM_CAN,_machineGUIBottomColor);
			}	
		}

		void CSkinDlg::OnBnClickedMachineguiTitlec()
		{
				CColorDialog dlg(_machineGUITitleColor);

				if(dlg.DoModal() == IDOK)
				{
					_machineGUITitleColor = dlg.GetColor();
					UpdateCanvasColour(IDC_MACHINETITLE_CAN,_machineGUITitleColor);
				}
		}

		void CSkinDlg::OnBnClickedMachineguiTitlefontc2()
		{
				CColorDialog dlg(_machineGUITitleFontColor);

				if(dlg.DoModal() == IDOK)
				{
					_machineGUITitleFontColor = dlg.GetColor();
					UpdateCanvasColour(IDC_MACHINEFONTTITLE_CAN,_machineGUITitleFontColor);
				}
		}

		void CSkinDlg::OnBnClickedMachineguiTopc2()
		{
				CColorDialog dlg(_machineGUIHTopColor);

				if(dlg.DoModal() == IDOK)
				{
					_machineGUIHTopColor = dlg.GetColor();
					UpdateCanvasColour(IDC_MACHINETOP_CAN2,_machineGUIHTopColor);
				}
		}

		void CSkinDlg::OnBnClickedMachineguiTopfontc2()
		{
				CColorDialog dlg(_machineGUIHFontTopColor);

				if(dlg.DoModal() == IDOK)
				{
					_machineGUIHFontTopColor = dlg.GetColor();
					UpdateCanvasColour(IDC_MACHINEFONTTOP_CAN2,_machineGUIHFontTopColor);
				}
		}

		void CSkinDlg::OnBnClickedMachineguiBottomc2()
		{
				CColorDialog dlg(_machineGUIHBottomColor);

				if(dlg.DoModal() == IDOK)
				{
					_machineGUIHBottomColor = dlg.GetColor();
					UpdateCanvasColour(IDC_MACHINEBOTTOM_CAN2,_machineGUIHBottomColor);
				}
		}

		void CSkinDlg::OnBnClickedMachineguiBottomfontc2()
		{
				CColorDialog dlg(_machineGUIHFontBottomColor);

				if(dlg.DoModal() == IDOK)
				{
					_machineGUIHFontBottomColor = dlg.GetColor();
					UpdateCanvasColour(IDC_MACHINEFONTBOTTOM_CAN2,_machineGUIHFontBottomColor);
				}
		}


		void CSkinDlg::OnDrawEmptyData()
		{
			_pattern_draw_empty_data = m_pattern_draw_empty_data.GetCheck() >0?true:false;
		}

		void CSkinDlg::OnDrawMacIndex()
		{
			_draw_mac_index = m_draw_mac_index.GetCheck() >0?true:false;
		}

		void CSkinDlg::OnBnClickedMachineguiBitmap()
		{
				OPENFILENAME ofn; // common dialog box structure
				char szFile[_MAX_PATH]; // buffer for file name
				char szPath[_MAX_PATH]; // buffer for file name
				szFile[0]='\0';
				szPath[0]='\0';
				CString str1(szBmpDialFilename.c_str());
				int i = str1.ReverseFind('\\')+1;
				CString str2 = str1.Mid(i);
				std::strcpy(szFile,str2);
				std::strcpy(szPath,str1);
				szPath[i]=0;
				// Initialize OPENFILENAME
				::ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = GetParent()->m_hWnd;
				ofn.lpstrFile = szFile;
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = "Bitmaps (*.bmp)\0*.bmp\0";
				ofn.nFilterIndex = 0;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = szPath;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				// Display the Open dialog box. 
				if(GetOpenFileName(&ofn)==TRUE)
				{
					szBmpDialFilename = szFile;
					bBmpDial = TRUE;
					CString str1(szBmpDialFilename.c_str());
					int i = str1.ReverseFind('\\')+1;
					CString str2 = str1.Mid(i);
					m_machine_GUI_bitmap.SetWindowText(str2);
				}
				else
				{
					bBmpDial = FALSE;
					m_machine_GUI_bitmap.SetWindowText("Default Dial Bitmap");
				}
		}

		void CSkinDlg::OnMachineBitmap() 
		{
			OPENFILENAME ofn; // common dialog box structure
			char szFile[_MAX_PATH]; // buffer for file name
			char szPath[_MAX_PATH]; // buffer for file name
			szFile[0]='\0';
			szPath[0]='\0';
			CString str1(szBmpBkgFilename.c_str());
			int i = str1.ReverseFind('\\')+1;
			CString str2 = str1.Mid(i);
			std::strcpy(szFile,str2);
			std::strcpy(szPath,str1);
			szPath[i]=0;
			// Initialize OPENFILENAME
			::ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Bitmaps (*.bmp)\0*.bmp\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = szPath;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if(GetOpenFileName(&ofn)==TRUE)
			{
				szBmpBkgFilename = szFile;
				bBmpBkg = TRUE;
				CString str1(szBmpBkgFilename.c_str());
				int i = str1.ReverseFind('\\')+1;
				CString str2 = str1.Mid(i);
				m_machine_background_bitmap.SetWindowText(str2);
			}
			else
			{
				bBmpBkg = FALSE;
				m_machine_background_bitmap.SetWindowText("No Background Bitmap");
			}
		}

		void CSkinDlg::OnSelchangeTrianglesize() 
		{
			_triangle_size=m_triangle_size.GetCurSel()+8;
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
