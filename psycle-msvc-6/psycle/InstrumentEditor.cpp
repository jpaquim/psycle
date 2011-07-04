// InstrumentEditor.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "MainFrm.h"
#include "InstrumentEditor.h"
#include "EnvDialog.h"
//#include "io.h"
//#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInstrumentEditor dialog
CMainFrame* pParentMainI;

CInstrumentEditor::CInstrumentEditor(CWnd* pParent /*=NULL*/)
	: CDialog(CInstrumentEditor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInstrumentEditor)
	//}}AFX_DATA_INIT
}


void CInstrumentEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInstrumentEditor)
	DDX_Control(pDX, IDC_VOLABEL2, m_finelabel);
	DDX_Control(pDX, IDC_SLIDER2, m_finetune);
	DDX_Control(pDX, IDC_LOOPEDIT, m_loopedit);
	DDX_Control(pDX, IDC_CHECK4, m_loopcheck);
	DDX_Control(pDX, IDC_RRES, m_rres_check);
	DDX_Control(pDX, IDC_PANSLIDER, m_panslider);
	DDX_Control(pDX, IDC_RPAN, m_rpan_check);
	DDX_Control(pDX, IDC_RCUT, m_rcut_check);
	DDX_Control(pDX, IDC_NNA_COMBO, m_nna_combo);
	DDX_Control(pDX, IDC_INSTNAME, m_instname);
	DDX_Control(pDX, IDC_VOLABEL, m_volabel);
	DDX_Control(pDX, IDC_SLIDER1, m_volumebar);
	DDX_Control(pDX, IDC_WAVELENGTH, m_wlen);
	DDX_Control(pDX, IDC_LOOPSTART, m_loopstart);
	DDX_Control(pDX, IDC_LOOPEND, m_loopend);
	DDX_Control(pDX, IDC_LOOPTYPE, m_looptype);
	DDX_Control(pDX, IDC_STEREO, m_chanlabel);
	DDX_Control(pDX, IDC_STATUS, m_status);
	DDX_Control(pDX, IDC_INSTNUMBER, m_instlabel);
	DDX_Control(pDX, IDC_WAVENAME, m_wavename);
	DDX_Control(pDX, IDC_LAYERN, m_layer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInstrumentEditor, CDialog)
	//{{AFX_MSG_MAP(CInstrumentEditor)
	ON_BN_CLICKED(IDC_LAYER1, OnLayer1)
	ON_BN_CLICKED(IDC_LAYER2, OnLayer2)
	ON_BN_CLICKED(IDC_LAYER3, OnLayer3)
	ON_BN_CLICKED(IDC_LAYER4, OnLayer4)
	ON_BN_CLICKED(IDC_LAYER5, OnLayer5)
	ON_BN_CLICKED(IDC_LAYER6, OnLayer6)
	ON_BN_CLICKED(IDC_LAYER7, OnLayer7)
	ON_BN_CLICKED(IDC_LAYER8, OnLayer8)
	ON_BN_CLICKED(IDC_LAYER9, OnLayer9)
	ON_BN_CLICKED(IDC_LAYER10, OnLayer10)
	ON_BN_CLICKED(IDC_LAYER11, OnLayer11)
	ON_BN_CLICKED(IDC_LAYER12, OnLayer12)
	ON_BN_CLICKED(IDC_LAYER13, OnLayer13)
	ON_BN_CLICKED(IDC_LAYER14, OnLayer14)
	ON_BN_CLICKED(IDC_LAYER15, OnLayer15)
	ON_BN_CLICKED(IDC_LAYER16, OnLayer16)
	ON_BN_CLICKED(IDC_LOOPOFF, OnLoopoff)
	ON_BN_CLICKED(IDC_LOOPFORWARD, OnLoopforward)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_EN_CHANGE(IDC_INSTNAME, OnChangeInstname)
	ON_EN_CHANGE(IDC_WAVENAME, OnChangeWavename)
	ON_CBN_SELCHANGE(IDC_NNA_COMBO, OnSelchangeNnaCombo)
	ON_BN_CLICKED(IDC_BUTTON12, OnPrevInstrument)
	ON_BN_CLICKED(IDC_BUTTON13, OnNextInstrument)
	ON_BN_CLICKED(IDC_BUTTON8, OnEnvButton)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PANSLIDER, OnCustomdrawPanslider)
	ON_BN_CLICKED(IDC_RPAN, OnRpan)
	ON_BN_CLICKED(IDC_RCUT, OnRcut)
	ON_BN_CLICKED(IDC_RRES, OnRres)
	ON_BN_CLICKED(IDC_CHECK4, OnLoopCheck)
	ON_EN_CHANGE(IDC_LOOPEDIT, OnChangeLoopedit)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_BN_CLICKED(IDC_BUTTON7, OnKillInstrument)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInstrumentEditor message handlers

//////////////////////////////////////////////////////////////////////
// Layer selector buttons

void CInstrumentEditor::OnLayer1() {_pSong->waveSelected=0;WaveUpdate();}
void CInstrumentEditor::OnLayer2() {_pSong->waveSelected=1;WaveUpdate();}
void CInstrumentEditor::OnLayer3() {_pSong->waveSelected=2;WaveUpdate();}
void CInstrumentEditor::OnLayer4() {_pSong->waveSelected=3;WaveUpdate();}
void CInstrumentEditor::OnLayer5() {_pSong->waveSelected=4;WaveUpdate();}
void CInstrumentEditor::OnLayer6() {_pSong->waveSelected=5;WaveUpdate();}
void CInstrumentEditor::OnLayer7() {_pSong->waveSelected=6;WaveUpdate();}
void CInstrumentEditor::OnLayer8() {_pSong->waveSelected=7;WaveUpdate();}
void CInstrumentEditor::OnLayer9() {_pSong->waveSelected=8;WaveUpdate();}
void CInstrumentEditor::OnLayer10() {_pSong->waveSelected=9;WaveUpdate();}
void CInstrumentEditor::OnLayer11() {_pSong->waveSelected=10;WaveUpdate();}
void CInstrumentEditor::OnLayer12() {_pSong->waveSelected=11;WaveUpdate();}
void CInstrumentEditor::OnLayer13() {_pSong->waveSelected=12;WaveUpdate();}
void CInstrumentEditor::OnLayer14() {_pSong->waveSelected=13;WaveUpdate();}
void CInstrumentEditor::OnLayer15() {_pSong->waveSelected=14;WaveUpdate();}
void CInstrumentEditor::OnLayer16() {_pSong->waveSelected=15;WaveUpdate();}

BOOL CInstrumentEditor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	cando=false;
	
	m_volumebar.SetRange(0,512,true);
	m_finetune.SetRange(-256,256,true);

	m_instname.SetLimitText(31);
	m_wavename.SetLimitText(31);
	
	m_panslider.SetRange(0,256);
	
	m_nna_combo.AddString("Note Cut");
	m_nna_combo.AddString("Note Release");
	m_nna_combo.AddString("None");

	_pSong->waveSelected=0;

	WaveUpdate();

	cando=true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////
// Auxiliar members

void CInstrumentEditor::WaveUpdate()
{
	if(_pSong->waveSelected<0)_pSong->waveSelected=0;
	if(_pSong->waveSelected>15)_pSong->waveSelected=15;

	int sw = _pSong->waveSelected;
	int si = _pSong->instSelected;

	char buffer[64];
	bool const ils = _pSong->_instruments[si]._loop;

	m_loopcheck.SetCheck(ils);
	sprintf(buffer,"%d",_pSong->_instruments[si]._lines);
	m_loopedit.EnableWindow(ils);
	m_loopedit.SetWindowText(buffer);

	m_panslider.SetPos(_pSong->_instruments[si]._pan);
	m_rpan_check.SetCheck(_pSong->_instruments[si]._RPAN);
	m_rcut_check.SetCheck(_pSong->_instruments[si]._RCUT);
	m_rres_check.SetCheck(_pSong->_instruments[si]._RRES);
	
	// Set instrument current selected label
	sprintf(buffer,"%d",si);
	m_instlabel.SetWindowText(buffer);

	// Set instrument current selected name
	m_instname.SetWindowText(_pSong->_instruments[si]._sName);

	// Set layer current selected label
	sprintf(buffer,"%d",sw+1);
	m_layer.SetWindowText(buffer);

	// Set layer name
	sprintf(buffer,"%s",_pSong->waveName[si][sw]);
	m_wavename.SetWindowText(buffer);

	// Set mono/stereo
	if(_pSong->waveLength[si][sw]>0)
	{
	if(_pSong->waveStereo[si][sw])
	sprintf(buffer,"Stereo");
	else
	sprintf(buffer,"Mono");
	}
	else
	sprintf(buffer," ");
	
	m_chanlabel.SetWindowText(buffer);

	// Set looptype
	if(_pSong->waveLoopType[si][sw])
	sprintf(buffer,"Forward");
	else
	sprintf(buffer,"Off");
	
	m_looptype.SetWindowText(buffer);

	// Display Loop Points & Wave Length
	
	sprintf(buffer,"%d",_pSong->waveLoopStart[si][sw]);
	m_loopstart.SetWindowText(buffer);

	sprintf(buffer,"%d",_pSong->waveLoopEnd[si][sw]);
	m_loopend.SetWindowText(buffer);

	sprintf(buffer,"%d",_pSong->waveLength[si][sw]);
	m_wlen.SetWindowText(buffer);

	// Volume bar
	m_volumebar.SetPos(_pSong->waveVolume[si][sw]);
	m_finetune.SetPos(_pSong->waveFinetune[si][sw]);

	UpdateCombo();
}

//////////////////////////////////////////////////////////////////////
// Loop setting GUI Handlers

void CInstrumentEditor::OnLoopoff() 
{
int si = _pSong->instSelected;
int sw = _pSong->waveSelected;

	if(_pSong->waveLoopType[si][sw])
	{
	_pSong->waveLoopType[si][sw]=0;
	WaveUpdate();
	}
}

void CInstrumentEditor::OnLoopforward() 
{
int sw=_pSong->waveSelected;
int si=_pSong->instSelected;

	if(!_pSong->waveLoopType[si][sw])
	{
	_pSong->waveLoopType[si][sw]=1;
	WaveUpdate();
	}
}

//////////////////////////////////////////////////////////////////////
// Volume bar handler

void CInstrumentEditor::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
int si=_pSong->instSelected;
int sw=_pSong->waveSelected;
char buffer[8];

	_pSong->waveVolume[si][sw]=m_volumebar.GetPos();
	
	sprintf(buffer,"%d%%",_pSong->waveVolume[si][sw]);
	m_volabel.SetWindowText(buffer);

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////
// Change instrument & layer name

void CInstrumentEditor::OnChangeInstname() 
{
	int si = _pSong->instSelected;
	m_instname.GetWindowText(_pSong->_instruments[si]._sName, 32);
}

void CInstrumentEditor::OnChangeWavename() 
{
	int si=_pSong->instSelected;
	int sw=_pSong->waveSelected;

	m_wavename.GetWindowText(_pSong->waveName[si][sw],32);

}

void CInstrumentEditor::OnSelchangeNnaCombo() 
{
	_pSong->_instruments[_pSong->instSelected]._NNA = m_nna_combo.GetCurSel();	
}

void CInstrumentEditor::UpdateCombo() 
{
	switch(_pSong->_instruments[_pSong->instSelected]._NNA)
	{
	case 0:m_nna_combo.SelectString(0,"Note Cut");break;
	case 1:m_nna_combo.SelectString(0,"Note Release");break;
	case 2:m_nna_combo.SelectString(0,"None");break;
	}
}

void CInstrumentEditor::OnPrevInstrument() 
{
	int si=_pSong->instSelected;

	if(si>0)
	{
		_pSong->instSelected--;
		WaveUpdate();
		pParentMainI->UpdateComboIns(false);
	}

}

void CInstrumentEditor::OnNextInstrument() 
{
	int si=_pSong->instSelected;

	if(si<255)
	{
		_pSong->instSelected++;
		WaveUpdate();
		pParentMainI->UpdateComboIns(false);
	}
}

void CInstrumentEditor::OnEnvButton() 
{
	CEnvDialog dlg;
	dlg._pSong=_pSong;
	dlg.DoModal();
}

void CInstrumentEditor::OnCustomdrawPanslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=_pSong->instSelected;
	_pSong->_instruments[si]._pan = m_panslider.GetPos();
	*pResult = 0;
}

void CInstrumentEditor::OnRpan() 
{
	int si = _pSong->instSelected;
	
	if (m_rpan_check.GetCheck())
	{
		_pSong->_instruments[si]._RPAN = true;
	}
	else
	{
		_pSong->_instruments[si]._RPAN = false;
	}
}

void CInstrumentEditor::OnRcut() 
{
	int si=_pSong->instSelected;
	
	if (m_rcut_check.GetCheck())
	{
		_pSong->_instruments[si]._RCUT = true;
	}
	else
	{
		_pSong->_instruments[si]._RCUT = false;
	}
}

void CInstrumentEditor::OnRres() 
{
	int si=_pSong->instSelected;
	
	if(m_rres_check.GetCheck())
	{
	_pSong->_instruments[si]._RRES = true;
	}
	else
	{
	_pSong->_instruments[si]._RRES = false;
	}
}

void CInstrumentEditor::Validate()
{
	pParentMainI=(CMainFrame*)GetParentFrame();
}

void CInstrumentEditor::OnLoopCheck() 
{
	int si=_pSong->instSelected;
	
	if(m_loopcheck.GetCheck())
	{
		_pSong->_instruments[si]._loop = true;
		m_loopedit.EnableWindow(true);
	}
	else
	{
		_pSong->_instruments[si]._loop = false;
		m_loopedit.EnableWindow(false);
	}
}

void CInstrumentEditor::OnChangeLoopedit() 
{
	int si = _pSong->instSelected;
	CString buffer;
	m_loopedit.GetWindowText(buffer);
	_pSong->_instruments[si]._lines = atoi(buffer);

	if (_pSong->_instruments[si]._lines < 1)
	{
		_pSong->_instruments[si]._lines = 1;
	}
	if (_pSong->_instruments[si]._lines > 128)
	{
		_pSong->_instruments[si]._lines = 128;
	}
}

void CInstrumentEditor::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=_pSong->instSelected;
	int sw=_pSong->waveSelected;
	char buffer[8];

	if(cando)
	_pSong->waveFinetune[si][sw]=m_finetune.GetPos();
	
	sprintf(buffer,"%d",_pSong->waveFinetune[si][sw]);
	m_finelabel.SetWindowText(buffer);

	*pResult = 0;
}

void CInstrumentEditor::OnKillInstrument() 
{
	_pSong->DeleteInstrument(_pSong->instSelected);
	WaveUpdate();
	pParentMainI->UpdateComboIns();
}
