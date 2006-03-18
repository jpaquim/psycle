// InstrumentEditor.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
//#include "psy.h"
//#include "InstrumentEditor.h"
#include "MainFrm.h"
#include "EnvDialog.h"
#include "io.h"
#include "direct.h"

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInstrumentEditor message handlers

//////////////////////////////////////////////////////////////////////
// Layer selector buttons

void CInstrumentEditor::OnLayer1() {songRef->waveSelected=0;WaveUpdate();}
void CInstrumentEditor::OnLayer2() {songRef->waveSelected=1;WaveUpdate();}
void CInstrumentEditor::OnLayer3() {songRef->waveSelected=2;WaveUpdate();}
void CInstrumentEditor::OnLayer4() {songRef->waveSelected=3;WaveUpdate();}
void CInstrumentEditor::OnLayer5() {songRef->waveSelected=4;WaveUpdate();}
void CInstrumentEditor::OnLayer6() {songRef->waveSelected=5;WaveUpdate();}
void CInstrumentEditor::OnLayer7() {songRef->waveSelected=6;WaveUpdate();}
void CInstrumentEditor::OnLayer8() {songRef->waveSelected=7;WaveUpdate();}
void CInstrumentEditor::OnLayer9() {songRef->waveSelected=8;WaveUpdate();}
void CInstrumentEditor::OnLayer10() {songRef->waveSelected=9;WaveUpdate();}
void CInstrumentEditor::OnLayer11() {songRef->waveSelected=10;WaveUpdate();}
void CInstrumentEditor::OnLayer12() {songRef->waveSelected=11;WaveUpdate();}
void CInstrumentEditor::OnLayer13() {songRef->waveSelected=12;WaveUpdate();}
void CInstrumentEditor::OnLayer14() {songRef->waveSelected=13;WaveUpdate();}
void CInstrumentEditor::OnLayer15() {songRef->waveSelected=14;WaveUpdate();}
void CInstrumentEditor::OnLayer16() {songRef->waveSelected=15;WaveUpdate();}

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

	songRef->waveSelected=0;

	WaveUpdate();

	cando=true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////
// Auxiliar members

void CInstrumentEditor::WaveUpdate()
{
	if(songRef->waveSelected<0)songRef->waveSelected=0;
	if(songRef->waveSelected>15)songRef->waveSelected=15;

	int sw=songRef->waveSelected;
	int si=songRef->instSelected;

	char buffer[64];
	bool const ils=songRef->instLoop[si];

	m_loopcheck.SetCheck(ils);
	sprintf(buffer,"%d",songRef->instLines[si]);
	m_loopedit.EnableWindow(ils);
	m_loopedit.SetWindowText(buffer);

	m_panslider.SetPos(songRef->instPAN[si]);
	m_rpan_check.SetCheck(songRef->instRPAN[si]);
	m_rcut_check.SetCheck(songRef->instRCUT[si]);
	m_rres_check.SetCheck(songRef->instRRES[si]);
	
	// Set instrument current selected label
	sprintf(buffer,"%d",si);
	m_instlabel.SetWindowText(buffer);

	// Set instrument current selected name
	m_instname.SetWindowText(songRef->instName[si]);

	// Set layer current selected label
	sprintf(buffer,"%d",sw+1);
	m_layer.SetWindowText(buffer);

	// Set layer name
	sprintf(buffer,"%s",songRef->waveName[si][sw]);
	m_wavename.SetWindowText(buffer);

	// Set mono/stereo
	if(songRef->waveLength[si][sw]>0)
	{
	if(songRef->waveStereo[si][sw])
	sprintf(buffer,"Stereo");
	else
	sprintf(buffer,"Mono");
	}
	else
	sprintf(buffer," ");
	
	m_chanlabel.SetWindowText(buffer);

	// Set looptype
	if(songRef->waveLoopType[si][sw])
	sprintf(buffer,"Forward");
	else
	sprintf(buffer,"Off");
	
	m_looptype.SetWindowText(buffer);

	// Display Loop Points & Wave Length
	
	sprintf(buffer,"%d",songRef->waveLoopStart[si][sw]);
	m_loopstart.SetWindowText(buffer);

	sprintf(buffer,"%d",songRef->waveLoopEnd[si][sw]);
	m_loopend.SetWindowText(buffer);

	sprintf(buffer,"%d",songRef->waveLength[si][sw]);
	m_wlen.SetWindowText(buffer);

	// Volume bar
	m_volumebar.SetPos(songRef->waveVolume[si][sw]);
	m_finetune.SetPos(songRef->waveFinetune[si][sw]);

	UpdateCombo();
}

//////////////////////////////////////////////////////////////////////
// Loop setting GUI Handlers

void CInstrumentEditor::OnLoopoff() 
{
int si=songRef->instSelected;
int sw=songRef->waveSelected;

	if(songRef->waveLoopType[si][sw])
	{
	songRef->waveLoopType[si][sw]=0;
	WaveUpdate();
	}
}

void CInstrumentEditor::OnLoopforward() 
{
int sw=songRef->waveSelected;
int si=songRef->instSelected;

	if(!songRef->waveLoopType[si][sw])
	{
	songRef->waveLoopType[si][sw]=1;
	WaveUpdate();
	}
}

//////////////////////////////////////////////////////////////////////
// Volume bar handler

void CInstrumentEditor::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
int si=songRef->instSelected;
int sw=songRef->waveSelected;
char buffer[8];

	songRef->waveVolume[si][sw]=m_volumebar.GetPos();
	
	sprintf(buffer,"%d%%",songRef->waveVolume[si][sw]);
	m_volabel.SetWindowText(buffer);

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////
// Change instrument & layer name

void CInstrumentEditor::OnChangeInstname() 
{
	int si=songRef->instSelected;
	m_instname.GetWindowText(songRef->instName[si],32);
}

void CInstrumentEditor::OnChangeWavename() 
{
	int si=songRef->instSelected;
	int sw=songRef->waveSelected;

	m_wavename.GetWindowText(songRef->waveName[si][sw],32);

}

void CInstrumentEditor::OnSelchangeNnaCombo() 
{
	songRef->instNNA[songRef->instSelected]=m_nna_combo.GetCurSel();	
}

void CInstrumentEditor::UpdateCombo() 
{
	switch(songRef->instNNA[songRef->instSelected])
	{
	case 0:m_nna_combo.SelectString(0,"Note Cut");break;
	case 1:m_nna_combo.SelectString(0,"Note Release");break;
	case 2:m_nna_combo.SelectString(0,"None");break;
	}
}

void CInstrumentEditor::OnPrevInstrument() 
{
	int si=songRef->instSelected;

	if(si>0)
	{
		songRef->instSelected--;
		WaveUpdate();
		pParentMainI->UpdateComboIns();
	}

}

void CInstrumentEditor::OnNextInstrument() 
{
	int si=songRef->instSelected;

	if(si<255)
	{
		songRef->instSelected++;
		WaveUpdate();
		pParentMainI->UpdateComboIns();
	}
}

void CInstrumentEditor::OnEnvButton() 
{
	CEnvDialog dlg;
	dlg.songRef=songRef;
	dlg.DoModal();
}

void CInstrumentEditor::OnCustomdrawPanslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;
	songRef->instPAN[si]=m_panslider.GetPos();
	*pResult = 0;
}

void CInstrumentEditor::OnRpan() 
{
	int si=songRef->instSelected;
	
	if(m_rpan_check.GetCheck())
	songRef->instRPAN[si]=true;
	else
	songRef->instRPAN[si]=false;
}

void CInstrumentEditor::OnRcut() 
{
	int si=songRef->instSelected;
	
	if(m_rcut_check.GetCheck())
	songRef->instRCUT[si]=true;
	else
	songRef->instRCUT[si]=false;
}

void CInstrumentEditor::OnRres() 
{
	int si=songRef->instSelected;
	
	if(m_rres_check.GetCheck())
	songRef->instRRES[si]=true;
	else
	songRef->instRRES[si]=false;
}

CInstrumentEditor::Validate()
{
pParentMainI=(CMainFrame*)GetParentFrame();
}

void CInstrumentEditor::OnLoopCheck() 
{
	int si=songRef->instSelected;
	
	if(m_loopcheck.GetCheck())
	{
		songRef->instLoop[si]=true;
		m_loopedit.EnableWindow(true);
	}
	else
	{
		songRef->instLoop[si]=false;
		m_loopedit.EnableWindow(false);
	}
}

void CInstrumentEditor::OnChangeLoopedit() 
{
	int si=songRef->instSelected;
	CString buffer;
	m_loopedit.GetWindowText(buffer);
	songRef->instLines[si]=atoi(buffer);

	if(songRef->instLines[si]<1)songRef->instLines[si]=1;
	if(songRef->instLines[si]>128)songRef->instLines[si]=128;
}

void CInstrumentEditor::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;
	int sw=songRef->waveSelected;
	char buffer[8];

	if(cando)
	songRef->waveFinetune[si][sw]=m_finetune.GetPos();
	
	sprintf(buffer,"%d",songRef->waveFinetune[si][sw]);
	m_finelabel.SetWindowText(buffer);

	*pResult = 0;
}
