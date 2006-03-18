// GearVst.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "GearVst.h"
#include "VstRack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearVst dialog


CGearVst::CGearVst(CWnd* pParent /*=NULL*/)
	: CDialog(CGearVst::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearVst)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGearVst::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearVst)
	DDX_Control(pDX, IDC_COMBO2, m_combo2);
	DDX_Control(pDX, IDC_BUTTON1, m_but);
	DDX_Control(pDX, IDC_COMBO1, m_combo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearVst, CDialog)
	//{{AFX_MSG_MAP(CGearVst)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON1, OnEditor)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, OnEjectPlug)
	ON_BN_CLICKED(IDCANCEL3, OnQuitEjectExit)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearVst message handlers

BOOL CGearVst::OnInitDialog() 
{
	CDialog::OnInitDialog();
	editor=false;

	char buf[32];

	for(int nm=0;nm<16;nm++)
	{
	sprintf(buf,"%d",nm+1);
	m_combo2.AddString(buf);
	}

	for(int c=0;c<MAX_VST_INSTANCES;c++)
	iPlug[c];

	int n=0;
	nFound=0;

	for(c=0;c<MAX_VST_INSTANCES;c++)
	{
		if(songRef->m_Vst.Instanced[c])
		{
			m_combo.AddString(songRef->m_Vst.GetPlugName(c));
			iPlug[n]=c;
			++n;
			++nFound;
		}
	}

	SetInstCombo();
	UpdateButt();
	
	m_combo2.SetCurSel(machineRef->ovst.midichannel);
	
	return TRUE;
}

int CGearVst::SetInstCombo()
{
	if(machineRef->ovst.ANY)
	{
	for(int c=0;c<nFound;c++)
	{	
		if(iPlug[c]==machineRef->ovst.instance)
		{
		m_combo.SetCurSel(c);
		return c;
		}
	}
	}

	m_combo.SetWindowText("Nothing selected");
	return -1;
}

void CGearVst::OnSelchangeCombo1() 
{
	int const m=m_combo.GetCurSel();

	int const xm=iPlug[m];

	if(xm>-1 && xm<MAX_VST_INSTANCES)
	machineRef->ovst.SetVstInstance(xm);

	UpdateButt();
}

void CGearVst::OnEditor() 
{
	// TODO: Add your control notification handler code here
	int const m=m_combo.GetCurSel();

	int const se=iPlug[m];

	if(se>-1 && se<MAX_VST_INSTANCES && songRef->m_Vst.Instanced[se])
	{
		if(songRef->m_Vst.effect[se]->flags & effFlagsHasEditor)
		{
			if(!editor)
			{
			editor=true;
			vstgui=new CVstGui(0);
			sprintf(vstgui->cWndTitle,songRef->m_Vst.GetPlugName(se));
			vstgui->effect=songRef->m_Vst.effect[se];			
			vstgui->LoadFrame(IDR_VSTFRAME,
				WS_OVERLAPPED
				| WS_CAPTION
				| WS_SYSMENU
				| FWS_ADDTOTITLE);

			vstgui->DoTheInit();
			vstgui->ShowWindow(SW_SHOWNORMAL);
			vstgui->editorOpen=&editor;
			}
			else
			MessageBox("Close previous dialog","VST Error",MB_OK | MB_ICONERROR);
		}
		else
		{
		CVstRack dlg;
		dlg.m_pVst=&songRef->m_Vst;
		dlg.nPlug=se;
		dlg.DoModal();
		}
	}
	else
	MessageBox("Select a valid VST plugin first","VST Instance error",MB_OK | MB_ICONERROR);

}

void CGearVst::OnClose() 
{
	if(editor)
	MessageBox("Close VST editor first");
	else
	CDialog::OnOK();
}

CGearVst::UpdateButt()
{
m_but.EnableWindow(machineRef->ovst.ANY);
}

void CGearVst::OnEjectPlug() 
{
	machineRef->ovst.ANY=false;
	m_combo.SetCurSel(-1);

	UpdateButt();
}

void CGearVst::OnQuitEjectExit() 
{
	machineRef->ovst.ANY=false;
	m_combo.SetCurSel(-1);

	UpdateButt();
	OnClose();
}

void CGearVst::OnOK() 
{
	OnClose();
}

void CGearVst::OnSelchangeCombo2() 
{
	int const e=m_combo2.GetCurSel();
	machineRef->ovst.midichannel=e;
}

void CGearVst::OnButton2()
{
		int const m=m_combo.GetCurSel();
		int const se=iPlug[m];

		CVstRack dlg;
		dlg.m_pVst=&songRef->m_Vst;
		dlg.nPlug=se;
		dlg.DoModal();
}
