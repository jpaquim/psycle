// WireDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "Song.h"
#include "WireDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWireDlg dialog


CWireDlg::CWireDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWireDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWireDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWireDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWireDlg)
	DDX_Control(pDX, IDC_STATIC1, m_volabel);
	DDX_Control(pDX, IDC_SLIDER1, m_volslider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWireDlg, CDialog)
	//{{AFX_MSG_MAP(CWireDlg)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWireDlg message handlers

BOOL CWireDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	Inval = false;
	m_volslider.SetRange(0,128,true);
	m_volslider.SetTicFreq(16);

/*
** The following code has to be updated with the new "SetWireVolume".
** (In fact, GetWireVolume, which is not done yet)
**
*/	
	// Get number of wire destination machine
	int idstMac = _pSrcMachine->_outputMachines[wireIndex];

	// Get reference to the destination machine
	_pDstMachine = Global::_pSong->_pMachines[idstMac];

	for (int c=0; c<MAX_CONNECTIONS; c++)
	{
		if (_pDstMachine->_inputCon[c])
		{
			if (_pDstMachine->_inputMachines[c] == isrcMac)
			{
				_dstWireIndex = c;
				break;
			}
		}
	}
	
	if ( _pSrcMachine->_type == MACH_VST || _pSrcMachine->_type == MACH_VSTFX )
	{
		if (_pDstMachine->_type == MACH_VST || _pDstMachine->_type == MACH_VSTFX )
		{
			m_volslider.SetPos(int(_pDstMachine->_inputConVol[_dstWireIndex]*128.0f));
		}
		else m_volslider.SetPos(int(_pDstMachine->_inputConVol[_dstWireIndex]*0.00390625f));
	}
	else if ( _pDstMachine->_type == MACH_VST || _pDstMachine->_type == MACH_VSTFX )
	{
		m_volslider.SetPos(int(_pDstMachine->_inputConVol[_dstWireIndex]*4194304.0f));
	}
	else m_volslider.SetPos(int(_pDstMachine->_inputConVol[_dstWireIndex]*128.0f));

	char buffer[64];
	sprintf(buffer,"[%d] %s -> %s", wireIndex, _pSrcMachine->_editName, destName);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWireDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{

	*pResult = 0;
	char buffer[32];
	const float invol = m_volslider.GetPos()*0.0078125f;

	if (invol > 0.0f){	sprintf(buffer,"%.1f dB",20.0f * log10(invol)); }
	else {				sprintf(buffer,"-Inf. dB"); }

	m_volabel.SetWindowText(buffer);

	if ( _pSrcMachine->_type == MACH_VST || _pSrcMachine->_type == MACH_VSTFX )
	{
		if (_pDstMachine->_type == MACH_VST || _pDstMachine->_type == MACH_VSTFX )
		{
			_pDstMachine->_inputConVol[_dstWireIndex] = invol;
		}
		else _pDstMachine->_inputConVol[_dstWireIndex] = invol*32768;
	}
	else if ( _pDstMachine->_type == MACH_VST || _pDstMachine->_type == MACH_VSTFX )
	{
		_pDstMachine->_inputConVol[_dstWireIndex] = invol*0.000030517578125f;
	}
	else _pDstMachine->_inputConVol[_dstWireIndex] = invol;

}

void CWireDlg::OnButton1() 
{
	Inval = true;
	_pSrcMachine->_connection[wireIndex] = false;
	_pSrcMachine->_numOutputs--;
	
	_pDstMachine->_inputCon[_dstWireIndex] = false;
	_pDstMachine->_numInputs--;

	OnOK();
}
