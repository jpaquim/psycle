// ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "ConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	Configuration* pConfig) 
{
	_pConfig = pConfig;
	_skinDlg._patternViewColor = pConfig->pvc_background;
	_skinDlg._fontColor = pConfig->pvc_font;
	_skinDlg._rowColor = pConfig->pvc_row;
	_skinDlg._beatColor = pConfig->pvc_rowbeat;
	_skinDlg._4beatColor = pConfig->pvc_row4beat;
	
	_skinDlg._machineViewColor = pConfig->mv_colour;
	_skinDlg._vubColor = pConfig->vu1;
	_skinDlg._vugColor = pConfig->vu2;
	_skinDlg._vucColor = pConfig->vu3;
	_skinDlg._gfxbuffer = pConfig->useDoubleBuffer;

	_outputDlg.m_driverIndex = pConfig->_outputDriverIndex;
	_outputDlg.m_midiDriverIndex = pConfig->_midiDriverIndex;	// MIDI IMPLEMENTATION
	_outputDlg.m_syncDriverIndex = pConfig->_syncDriverIndex;
	_outputDlg.m_midiHeadroom = pConfig->_midiHeadroom;
	_outputDlg._numDrivers = pConfig->_numOutputDrivers;
	_outputDlg.m_ppDrivers = pConfig->_ppOutputDrivers;

	char* ps = pConfig->GetInitialInstrumentDir();
	if (ps != NULL)
	{
		sprintf(_dirDlg._instPathBuf, ps);
	}
	ps = pConfig->GetInitialSongDir();
	if (ps != NULL)
	{
		sprintf(_dirDlg._songPathBuf, ps);
	}
	ps = pConfig->GetInitialPluginDir();
	if (ps != NULL)
	{
		sprintf(_dirDlg._pluginPathBuf, ps);
	}
	ps = pConfig->GetInitialVstDir();
	if (ps != NULL)
	{
		sprintf(_dirDlg._vstPathBuf, ps);
	}

	AddPage(&_dirDlg);
	AddPage(&_skinDlg);
	AddPage(&_outputDlg);
	AddPage(&_midiDlg);
}

int CConfigDlg::DoModal() 
{
	int retVal = CPropertySheet::DoModal();
	if (retVal == IDOK)
	{
		_pConfig->mv_colour = _skinDlg._machineViewColor;
		_pConfig->pvc_background = _skinDlg._patternViewColor;
		_pConfig->pvc_font = _skinDlg._fontColor;
		_pConfig->pvc_row = _skinDlg._rowColor;
		_pConfig->pvc_rowbeat = _skinDlg._beatColor;
		_pConfig->pvc_row4beat = _skinDlg._4beatColor;
		_pConfig->vu1 = _skinDlg._vubColor;
		_pConfig->vu2 = _skinDlg._vugColor;
		_pConfig->vu3 = _skinDlg._vucColor;
		_pConfig->useDoubleBuffer = _skinDlg._gfxbuffer;

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
	}
	return retVal;
}
