// WireDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "Machine.h"
#include "WireDlg.h"
#include "Helpers.h"

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
	m_volslider.SetRange(0,128);
	m_volslider.SetTicFreq(16);
	_dstWireIndex = _pSrcMachine->FindInputWire(_pDstMachine,isrcMac);

	float val;
	_pDstMachine->GetWireVolume(_dstWireIndex,val);
	m_volslider.SetPos(f2i(val*128));

	char buffer[64];
	sprintf(buffer,"[%d] %s -> %s", wireIndex, _pSrcMachine->_editName, _pDstMachine->_editName);
	SetWindowText(buffer);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWireDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char buffer[32];
	const float invol = m_volslider.GetPos()*0.0078125f;

	if (invol > 0.0f){	sprintf(buffer,"%.1f dB",20.0f * log10(invol)); }
	else {				sprintf(buffer,"-Inf. dB"); }

	m_volabel.SetWindowText(buffer);

	_pDstMachine->SetWireVolume(_dstWireIndex, invol );

	*pResult = 0;
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
