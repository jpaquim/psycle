// ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "ConfigDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPsycleApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg

IMPLEMENT_DYNAMIC(CConfigDlg, CPropertySheet)

CConfigDlg::CConfigDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CConfigDlg::CConfigDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CConfigDlg::~CConfigDlg()
{
}


BEGIN_MESSAGE_MAP(CConfigDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CConfigDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CConfigDlg::Init(
	Configuration* pConfig,int dlgnum) 
{
	_pConfig = pConfig;
	_skinDlg._patternSeparatorColor = pConfig->pvc_separator;
	_skinDlg._patternSeparatorColor2 = pConfig->pvc_separator2;
	_skinDlg._patternViewColor = pConfig->pvc_background;
	_skinDlg._patternViewColor2 = pConfig->pvc_background2;
	_skinDlg._fontColor = pConfig->pvc_font;
	_skinDlg._fontColor2 = pConfig->pvc_font2;
	_skinDlg._fontColorPlay = pConfig->pvc_fontPlay;
	_skinDlg._fontColorPlay2 = pConfig->pvc_fontPlay2;
	_skinDlg._fontColorCur = pConfig->pvc_fontCur;
	_skinDlg._fontColorCur2 = pConfig->pvc_fontCur2;
	_skinDlg._fontColorSel = pConfig->pvc_fontSel;
	_skinDlg._fontColorSel2 = pConfig->pvc_fontSel2;
	_skinDlg._rowColor = pConfig->pvc_row;
	_skinDlg._rowColor2 = pConfig->pvc_row2;
	_skinDlg._beatColor = pConfig->pvc_rowbeat;
	_skinDlg._beatColor2 = pConfig->pvc_rowbeat2;
	_skinDlg._4beatColor = pConfig->pvc_row4beat;
	_skinDlg._4beatColor2 = pConfig->pvc_row4beat2;
	_skinDlg._selectionColor = pConfig->pvc_selection;
	_skinDlg._selectionColor2 = pConfig->pvc_selection2;
	_skinDlg._cursorColor = pConfig->pvc_cursor;
	_skinDlg._cursorColor2 = pConfig->pvc_cursor2;
	_skinDlg._playbarColor = pConfig->pvc_playbar;
	_skinDlg._playbarColor2 = pConfig->pvc_playbar2;
	
	_skinDlg._machineViewColor = pConfig->mv_colour;
	_skinDlg._machineViewWireColor = pConfig->mv_wirecolour;
	_skinDlg._machineViewPolyColor = pConfig->mv_polycolour;
	_skinDlg._machineViewGeneratorFontColor = pConfig->mv_generator_fontcolour;
	_skinDlg._machineViewEffectFontColor = pConfig->mv_effect_fontcolour;
	_skinDlg._wireaa = pConfig->mv_wireaa;
	_skinDlg._wirewidth = pConfig->mv_wirewidth;

	_skinDlg._vubColor = pConfig->vu1;
	_skinDlg._vugColor = pConfig->vu2;
	_skinDlg._vucColor = pConfig->vu3;
	_skinDlg._gfxbuffer = pConfig->useDoubleBuffer;
	_skinDlg._linenumbers = pConfig->_linenumbers;
	_skinDlg._linenumbersHex = pConfig->_linenumbersHex;
	_skinDlg._linenumbersCursor = pConfig->_linenumbersCursor;

	strcpy(_skinDlg._pattern_fontface, pConfig->pattern_fontface);
	_skinDlg._pattern_font_point = pConfig->pattern_font_point;
	_skinDlg._pattern_font_x = pConfig->pattern_font_x;
	_skinDlg._pattern_font_y = pConfig->pattern_font_y;
	_skinDlg._pattern_draw_empty_data = pConfig->pattern_draw_empty_data;
	_skinDlg._draw_mac_index = pConfig->draw_mac_index;
	strcpy(_skinDlg._pattern_header_skin, pConfig->pattern_header_skin);
	_skinDlg._pattern_font_flags = pConfig->pattern_font_flags;

	strcpy(_skinDlg._generator_fontface, pConfig->generator_fontface);
	_skinDlg._generator_font_point = pConfig->generator_font_point;
	_skinDlg._generator_font_flags = pConfig->generator_font_flags;

	strcpy(_skinDlg._effect_fontface, pConfig->effect_fontface);
	_skinDlg._effect_font_point = pConfig->effect_font_point;
	_skinDlg._effect_font_flags = pConfig->effect_font_flags;

	strcpy(_skinDlg._machine_skin, pConfig->machine_skin);

	strcpy(_skinDlg.szBmpBkgFilename, pConfig->szBmpBkgFilename);
	_skinDlg.bBmpBkg = pConfig->bBmpBkg;

	_outputDlg.m_driverIndex = pConfig->_outputDriverIndex;
	_outputDlg.m_midiDriverIndex = pConfig->_midiDriverIndex;	// MIDI IMPLEMENTATION
	_outputDlg.m_syncDriverIndex = pConfig->_syncDriverIndex;
	_outputDlg.m_midiHeadroom = pConfig->_midiHeadroom;
	_outputDlg._numDrivers = pConfig->_numOutputDrivers;
	_outputDlg.m_ppDrivers = pConfig->_ppOutputDrivers;


	char* ps = pConfig->GetInitialInstrumentDir();
	if (ps != NULL)
	{
		strcpy(_dirDlg._instPathBuf, ps);
	}
	ps = pConfig->GetInitialSongDir();
	if (ps != NULL)
	{
		strcpy(_dirDlg._songPathBuf, ps);
	}
	ps = pConfig->GetInitialPluginDir();
	if (ps != NULL)
	{
		strcpy(_dirDlg._pluginPathBuf, ps);
	}
	ps = pConfig->GetInitialVstDir();
	if (ps != NULL)
	{
		strcpy(_dirDlg._vstPathBuf, ps);
	}
	ps = pConfig->GetInitialSkinDir();
	if (ps != NULL)
	{
		strcpy(_dirDlg._skinPathBuf, ps);
		strcpy(_skinDlg._skinPathBuf, ps);
		strcpy(_keyDlg._skinPathBuf, ps);
	}
	else
	{
		_dirDlg._skinPathBuf[0] = 0;
		_skinDlg._skinPathBuf[0] = 0;
		_keyDlg._skinPathBuf[0] = 0;
	}
	
	if ( dlgnum == 1 || dlgnum == 0 ) AddPage(&_skinDlg);
	if ( dlgnum == 2 || dlgnum == 0 ) AddPage(&_keyDlg);
	if ( dlgnum == 3 || dlgnum == 0 ) AddPage(&_dirDlg);
	if ( dlgnum == 4 || dlgnum == 0 ) AddPage(&_outputDlg);
	if ( dlgnum == 5 || dlgnum == 0 ) AddPage(&_midiDlg);
}

int CConfigDlg::DoModal() 
{
	int retVal = CPropertySheet::DoModal();
	if (retVal == IDOK)
	{
		_pConfig->mv_colour = _skinDlg._machineViewColor;
		_pConfig->mv_wirecolour = _skinDlg._machineViewWireColor;
		_pConfig->mv_wireaacolour = ((((_pConfig->mv_wirecolour&0x00ff0000) + ((_pConfig->mv_colour&0x00ff0000)*4))/5)&0x00ff0000) +
									((((_pConfig->mv_wirecolour&0x00ff00) + ((_pConfig->mv_colour&0x00ff00)*4))/5)&0x00ff00) +
									((((_pConfig->mv_wirecolour&0x00ff) + ((_pConfig->mv_colour&0x00ff)*4))/5)&0x00ff);

		_pConfig->mv_wireaacolour2 = (((((_pConfig->mv_wirecolour&0x00ff0000)) + ((_pConfig->mv_colour&0x00ff0000)))/2)&0x00ff0000) +
									(((((_pConfig->mv_wirecolour&0x00ff00)) + ((_pConfig->mv_colour&0x00ff00)))/2)&0x00ff00) +
									(((((_pConfig->mv_wirecolour&0x00ff)) + ((_pConfig->mv_colour&0x00ff)))/2)&0x00ff);

		_pConfig->mv_polycolour = _skinDlg._machineViewPolyColor;

		_pConfig->mv_generator_fontcolour = _skinDlg._machineViewGeneratorFontColor;
		_pConfig->mv_effect_fontcolour = _skinDlg._machineViewEffectFontColor;

		_pConfig->pvc_separator = _skinDlg._patternSeparatorColor;
		_pConfig->pvc_separator2 = _skinDlg._patternSeparatorColor2;
		_pConfig->pvc_background = _skinDlg._patternViewColor;
		_pConfig->pvc_background2 = _skinDlg._patternViewColor2;
		_pConfig->pvc_font = _skinDlg._fontColor;
		_pConfig->pvc_font2 = _skinDlg._fontColor2;
		_pConfig->pvc_fontPlay = _skinDlg._fontColorPlay;
		_pConfig->pvc_fontPlay2 = _skinDlg._fontColorPlay2;
		_pConfig->pvc_fontCur = _skinDlg._fontColorCur;
		_pConfig->pvc_fontCur2 = _skinDlg._fontColorCur2;
		_pConfig->pvc_fontSel = _skinDlg._fontColorSel;
		_pConfig->pvc_fontSel2 = _skinDlg._fontColorSel2;
		_pConfig->pvc_row = _skinDlg._rowColor;
		_pConfig->pvc_row2 = _skinDlg._rowColor2;
		_pConfig->pvc_rowbeat = _skinDlg._beatColor;
		_pConfig->pvc_rowbeat2 = _skinDlg._beatColor2;
		_pConfig->pvc_row4beat = _skinDlg._4beatColor;
		_pConfig->pvc_row4beat2 = _skinDlg._4beatColor2;
		_pConfig->pvc_selection = _skinDlg._selectionColor;
		_pConfig->pvc_selection2 = _skinDlg._selectionColor2;
		_pConfig->pvc_playbar = _skinDlg._playbarColor;
		_pConfig->pvc_playbar2 = _skinDlg._playbarColor2;
		_pConfig->pvc_cursor = _skinDlg._cursorColor;
		_pConfig->pvc_cursor2 = _skinDlg._cursorColor2;

		_pConfig->vu1 = _skinDlg._vubColor;
		_pConfig->vu2 = _skinDlg._vugColor;
		_pConfig->vu3 = _skinDlg._vucColor;
		_pConfig->mv_wireaa = _skinDlg._wireaa;
		_pConfig->mv_wirewidth = _skinDlg._wirewidth;

		_pConfig->useDoubleBuffer = _skinDlg._gfxbuffer;
		_pConfig->_linenumbers = _skinDlg._linenumbers;
		_pConfig->_linenumbersHex = _skinDlg._linenumbersHex;
		_pConfig->_linenumbersCursor = _skinDlg._linenumbersCursor;

		_pConfig->pattern_font_x = _skinDlg._pattern_font_x;
		_pConfig->pattern_font_y = _skinDlg._pattern_font_y;
		_pConfig->pattern_draw_empty_data = _skinDlg._pattern_draw_empty_data;
		_pConfig->draw_mac_index = _skinDlg._draw_mac_index;

		if (strcmp(_pConfig->pattern_header_skin, _skinDlg._pattern_header_skin))
		{
			// LOAD HEADER SKIN
			strcpy(_pConfig->pattern_header_skin, _skinDlg._pattern_header_skin);
			if (_pConfig->Initialized() ) ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.LoadPatternHeaderSkin();
		}

		_pConfig->pattern_font_point = _skinDlg._pattern_font_point;
		_pConfig->pattern_font_flags = _skinDlg._pattern_font_flags;
		strcpy(_pConfig->pattern_fontface, _skinDlg._pattern_fontface);

		_pConfig->generator_font_point = _skinDlg._generator_font_point;
		_pConfig->generator_font_flags = _skinDlg._generator_font_flags;
		strcpy(_pConfig->generator_fontface, _skinDlg._generator_fontface);

		_pConfig->effect_font_point = _skinDlg._effect_font_point;
		_pConfig->effect_font_flags = _skinDlg._effect_font_flags;
		strcpy(_pConfig->effect_fontface, _skinDlg._effect_fontface);

		_pConfig->CreateFonts();

		if (strcmp(_pConfig->machine_skin, _skinDlg._machine_skin))
		{
			strcpy(_pConfig->machine_skin, _skinDlg._machine_skin);
			// LOAD HEADER SKIN
			if (_pConfig->Initialized() ) 
			{
				((CMainFrame *)theApp.m_pMainWnd)->m_wndView.LoadMachineSkin();
			}
		}

		_pConfig->bBmpBkg = _skinDlg.bBmpBkg;
		if (_pConfig->bBmpBkg)
		{
			strcpy(_pConfig->szBmpBkgFilename, _skinDlg.szBmpBkgFilename);
			// LOAD HEADER SKIN
			if (_pConfig->Initialized() ) 
			{
				((CMainFrame *)theApp.m_pMainWnd)->m_wndView.LoadMachineBackground();
			}
		}
		if (_pConfig->Initialized() ) 
		{
			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.RecalcMetrics();
		}



		_pConfig->_outputDriverIndex = _outputDlg.m_driverIndex;
		_pConfig->_midiDriverIndex = _outputDlg.m_midiDriverIndex;	// MIDI IMPLEMENTATION
		_pConfig->_syncDriverIndex = _outputDlg.m_syncDriverIndex;
		_pConfig->_midiHeadroom = _outputDlg.m_midiHeadroom;
		_pConfig->_pOutputDriver = _pConfig->_ppOutputDrivers[_pConfig->_outputDriverIndex];

		if (_dirDlg._instPathChanged)
		{
			_pConfig->SetInitialInstrumentDir(_dirDlg._instPathBuf);
			_pConfig->SetInstrumentDir(_dirDlg._instPathBuf);
		}
		if (_dirDlg._songPathChanged)
		{
			_pConfig->SetInitialSongDir(_dirDlg._songPathBuf);
			_pConfig->SetSongDir(_dirDlg._songPathBuf);
		}
		if (_dirDlg._pluginPathChanged)
		{
			_pConfig->SetInitialPluginDir(_dirDlg._pluginPathBuf);
			_pConfig->SetPluginDir(_dirDlg._pluginPathBuf);
		}
		if (_dirDlg._vstPathChanged)
		{
			_pConfig->SetInitialVstDir(_dirDlg._vstPathBuf);
			_pConfig->SetVstDir(_dirDlg._vstPathBuf);
		}
		if (_dirDlg._skinPathChanged)
		{
			_pConfig->SetInitialSkinDir(_dirDlg._skinPathBuf);
			_pConfig->SetSkinDir(_dirDlg._skinPathBuf);
		}

		if (_pConfig->Initialized() ) 
		{
			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.RecalculateColourGrid();
			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.Repaint();
		}
	}
	return retVal;
}
