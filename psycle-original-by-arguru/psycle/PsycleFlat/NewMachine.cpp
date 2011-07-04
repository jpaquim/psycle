// NewMachine.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "NewMachine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewMachine dialog


CNewMachine::CNewMachine(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMachine::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMachine)
	//}}AFX_DATA_INIT

	OutBus=false;
}


void CNewMachine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMachine)
	DDX_Control(pDX, IDC_VERLABEL, m_verslabel);
	DDX_Control(pDX, IDC_DESCLABEL, m_desclabel);
	DDX_Control(pDX, IDC_NAMELABEL, m_namelabel);
	DDX_Control(pDX, IDC_TREE1, m_mactree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
	//{{AFX_MSG_MAP(CNewMachine)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnDblclkTree1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewMachine message handlers

BOOL CNewMachine::OnInitDialog() 
{
	CDialog::OnInitDialog();

	hGen=m_mactree.InsertItem("Generators",NULL,NULL,TVI_ROOT,TVI_LAST);
	hSam=m_mactree.InsertItem("Sampler",NULL,NULL,hGen,TVI_LAST);
	hVstI=m_mactree.InsertItem("VST2 Instrument",NULL,NULL,hGen,TVI_LAST);
	
	hEfx[0]=m_mactree.InsertItem("Effects",NULL,NULL,TVI_ROOT,TVI_LAST);
	hEfx[1]=m_mactree.InsertItem("'Dist!' Distortion",NULL,NULL,hEfx[0],TVI_LAST);
	hEfx[2]=m_mactree.InsertItem("PsychOsc AM",NULL,NULL,hEfx[0],TVI_LAST);
	hEfx[3]=m_mactree.InsertItem("Dalay Delay",NULL,NULL,hEfx[0],TVI_LAST);
	hEfx[4]=m_mactree.InsertItem("2p Filter",NULL,NULL,hEfx[0],TVI_LAST);
	hEfx[5]=m_mactree.InsertItem("Gainer",NULL,NULL,hEfx[0],TVI_LAST);
	hEfx[6]=m_mactree.InsertItem("Flanger",NULL,NULL,hEfx[0],TVI_LAST);
	hEfx[7]=m_mactree.InsertItem("Dummy plug",NULL,NULL,hEfx[0],TVI_LAST);
	hEfx[8]=m_mactree.InsertItem("VST2 Effect",NULL,NULL,hEfx[0],TVI_LAST);
	
	hMis[0]=m_mactree.InsertItem("Plug-Ins",NULL,NULL,TVI_ROOT,TVI_LAST);
	
	for(int c=0;c<songRef->nBfxDll;c++)
	{
		GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[c],"GetInfo");
		
		if(!bfxGetInfo)
		hMis[c+1]=m_mactree.InsertItem("BAD PLUGIN DATA",NULL,NULL,hMis[0],TVI_LAST);
		else
		hMis[c+1]=m_mactree.InsertItem(bfxGetInfo()->Name,NULL,NULL,hMis[0],TVI_LAST);
	}

	m_mactree.Expand(hGen,TVE_EXPAND);
	m_mactree.Expand(hMis[0],TVE_EXPAND);
	m_mactree.Expand(hEfx[0],TVE_EXPAND);
	m_mactree.SelectItem(hGen);

	Outputmachine=-1;	
	
	return TRUE; 
}

//////////////////////////////////////////////////////////////////////
// New Machine selection function

void CNewMachine::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM tHand=m_mactree.GetSelectedItem();
	
	Outputmachine=-1;
	OutBus=false;

	char nameBuffer[64];
	char descBuffer[64];
	char versBuffer[64];
	
	sprintf(nameBuffer,"");
	sprintf(descBuffer,"");
	sprintf(versBuffer,"");
	
	// Generators
	if(tHand==hSam){
	sprintf(nameBuffer,"Sampler");
	sprintf(descBuffer,"Stereo Sampler Unit. Inserts new sampler.");
	sprintf(versBuffer,"V0.45b");
	Outputmachine=3;
	OutBus=true;
	}

	if(tHand==hVstI){
	sprintf(nameBuffer,"Vst2 Instrument");
	sprintf(descBuffer,"New VST2 Instrument");
	sprintf(versBuffer,"V0.1b");
	Outputmachine=9;
	OutBus=true;
	}

	// Effects
	if(tHand==hEfx[1]){
	sprintf(nameBuffer,"'Dist!' Distortion");
	sprintf(descBuffer,"Threshold/Clamp digital distortion/booster machine");
	sprintf(versBuffer,"V1.0");
	Outputmachine=2;
	}

	if(tHand==hEfx[2]){
	sprintf(nameBuffer,"PsychOsc Theremin/AM");
	sprintf(descBuffer,"Amplitude Modulator/Analog Theremin Emulator");
	sprintf(versBuffer,"V0.3b");
	Outputmachine=1;
	}

	if(tHand==hEfx[3]){
	sprintf(nameBuffer,"Dalay Delay");
	sprintf(descBuffer,"Digital Delay Unit");
	sprintf(versBuffer,"V1.1");
	Outputmachine=4;
	}

	if(tHand==hEfx[4]){
	sprintf(nameBuffer,"2p Filter");
	sprintf(descBuffer,"Digital 2-Pole Filter");
	sprintf(versBuffer,"V1.1");
	Outputmachine=5;
	}

	if(tHand==hEfx[5]){
	sprintf(nameBuffer,"Gainer");
	sprintf(descBuffer,"Volume Gainer");
	sprintf(versBuffer,"V1.0");
	Outputmachine=6;
	}

	if(tHand==hEfx[6]){
	sprintf(nameBuffer,"Flanger");
	sprintf(descBuffer,"Variable delay flanger unit");
	sprintf(versBuffer,"V1.0");
	Outputmachine=7;
	}

	if(tHand==hEfx[7]){
	sprintf(nameBuffer,"Dummy");
	sprintf(descBuffer,"Replaces inexistent plugins");
	sprintf(versBuffer,"V1.0");
	Outputmachine=255;
	}

	if(tHand==hEfx[8]){
	sprintf(nameBuffer,"VST2 Effect");
	sprintf(descBuffer,"Insert VST2 Effect");
	sprintf(versBuffer,"V0.1b");
	Outputmachine=10;
	}

	for(int c=0;c<songRef->nBfxDll;c++)
	{
		if(tHand==hMis[c+1])
		{
		GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[c],"GetInfo");
		
			if(bfxGetInfo)
			{
			sprintf(nameBuffer,bfxGetInfo()->ShortName);

			char b2[1024];
			sprintf(b2,"%s by %s",
			bfxGetInfo()->Name,
			bfxGetInfo()->Author);
			sprintf(descBuffer,b2);
			sprintf(versBuffer,"PsycleAPI 1.0");
			Outputmachine=8;
			Outputdll=c;
			
			if(bfxGetInfo()->Flags==3)
			OutBus=true;
			else
			OutBus=false;
			}
		}
		
	}

	m_namelabel.SetWindowText(nameBuffer);
	m_desclabel.SetWindowText(descBuffer);
	m_verslabel.SetWindowText(versBuffer);
	
	*pResult = 0;	
}

void CNewMachine::OnOK() 
{
	CDialog::OnOK();
}

void CNewMachine::OnDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{

	if(Outputmachine>-1)
	OnOK();
	
	*pResult = 0;
}
