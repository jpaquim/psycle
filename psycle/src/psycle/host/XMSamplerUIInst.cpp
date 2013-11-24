// XMSamplerUIInst.cpp : XMSamplerUIInst
/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#include <psycle/host/detail/project.private.hpp>
#include "XMSamplerUIInst.hpp"

#include "PsycleConfig.hpp"

#include "XMSongLoader.hpp"
#include "ITModule2.h"

#include "Player.hpp"
#include "Song.hpp"
#include "XMInstrument.hpp"
#include "InstrumentEditorUI.hpp"
#include "MainFrm.hpp"

namespace psycle { namespace host {

IMPLEMENT_DYNAMIC(XMSamplerUIInst, CPropertyPage)
XMSamplerUIInst::XMSamplerUIInst()
: CPropertyPage(XMSamplerUIInst::IDD)
, m_bInitialized(false)
, m_iCurrentSelected(0)
{
}

XMSamplerUIInst::~XMSamplerUIInst()
{
}

void XMSamplerUIInst::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INSTRUMENTLIST, m_InstrumentList);
	DDX_Control(pDX, IDC_XMINST_TAB, m_tabMain);
}

BEGIN_MESSAGE_MAP(XMSamplerUIInst, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_INSTRUMENTLIST, OnLbnSelchangeInstrumentlist)
	ON_NOTIFY(TCN_SELCHANGE, IDC_XMINST_TAB, &OnTcnSelchangeTab1)
	ON_NOTIFY(TCN_SELCHANGING, IDC_XMINST_TAB, &OnTcnSelchangingTab1)
	ON_BN_CLICKED(IDC_LOADINS, OnBnClickedLoadins)
	ON_BN_CLICKED(IDC_SAVEINS, OnBnClickedSaveins)
	ON_BN_CLICKED(IDC_DUPEINS, OnBnClickedDupeins)
	ON_BN_CLICKED(IDC_DELETEINS, OnBnClickedDeleteins)
END_MESSAGE_MAP()

BOOL XMSamplerUIInst::PreTranslateMessage(MSG* pMsg) 
{
	InstrumentEditorUI* parent = dynamic_cast<InstrumentEditorUI*>(GetParent());
	BOOL res = parent->PreTranslateChildMessage(pMsg, GetFocus()->GetSafeHwnd());
	if (res == FALSE ) return CPropertyPage::PreTranslateMessage(pMsg);
	return res;
}

BOOL XMSamplerUIInst::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	CRect rect, rect2;
	m_tabMain.InsertItem(0,_T("(4)General"));
	m_tabMain.InsertItem(1,_T("(5)Amplitude"));
	m_tabMain.InsertItem(2,_T("(6)Pan"));
	m_tabMain.InsertItem(3,_T("(7)Filter"));
	m_tabMain.InsertItem(4,_T("(8)Pitch"));
	m_genTab.Create(IDD_INST_SAMPULSE_INSTGEN,&m_tabMain);
	m_ampTab.Create(IDD_INST_SAMPULSE_INSTAMP,&m_tabMain);
	m_panTab.Create(IDD_INST_SAMPULSE_INSTPAN,&m_tabMain);
	m_filTab.Create(IDD_INST_SAMPULSE_INSTFIL,&m_tabMain);
	m_pitTab.Create(IDD_INST_SAMPULSE_INSTPIT,&m_tabMain);

	m_tabMain.GetWindowRect(rect);
	rect2.CopyRect(rect);
	TabCtrl_AdjustRect(m_tabMain.m_hWnd, FALSE, &rect); 
	rect.OffsetRect(rect.left -rect2.left - rect.left, rect.top - rect2.top - rect.top); 
	m_genTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);
	m_ampTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);
	m_panTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);
	m_filTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);
	m_pitTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);

	m_tabMain.SetCurSel(0);
	m_genTab.ShowWindow(SW_SHOW);
	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void XMSamplerUIInst::SetInstrumentData(const int instno)
{
	if (Global::song().xminstruments.Exists(instno) == false) {
		XMInstrument inst;
		inst.Init();
		Global::song().xminstruments.SetInst(inst,instno);
	}

	XMInstrument& inst = Global::song().xminstruments.get(instno);
	m_iCurrentSelected=instno;

	m_genTab.AssignGeneralValues(inst);
	m_ampTab.AssignAmplitudeValues(inst);
	m_panTab.AssignPanningValues(inst);
	m_filTab.AssignFilterValues(inst);
	m_pitTab.AssignPitchValues(inst);
}


BOOL XMSamplerUIInst::OnSetActive()
{
	TRACE("in setActive\n");
	if ( m_bInitialized == false )
	{
		FillInstrumentList();
		SetInstrumentData(0);
		m_bInitialized = true;
	}

	return CPropertyPage::OnSetActive();
}
void XMSamplerUIInst::FillInstrumentList() {
	int i = m_InstrumentList.GetCurSel();
	m_InstrumentList.ResetContent();
	InstrumentList& list = Global::song().xminstruments;
	for (int i=0;i<XMInstrument::MAX_INSTRUMENT;i++)
	{
		char line[48];
		if (list.Exists(i)) {
			const XMInstrument& inst = list[i];
			sprintf(line,"%02X%s: ",i,inst.IsEnabled()?"*":" ");
			strcat(line,inst.Name().c_str());
		}
		else {
			sprintf(line,"%02X : ",i);
		}
		m_InstrumentList.AddString(line);
	}
	if (i !=  LB_ERR) {
		m_InstrumentList.SetCurSel(i);
	}
	else {
		m_InstrumentList.SetCurSel(0);
	}
}
	
void XMSamplerUIInst::OnLbnSelchangeInstrumentlist()
{
	if(m_bInitialized)
	{
		m_bInitialized = false;
		SetInstrumentData(m_InstrumentList.GetCurSel());
		CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
		win->ChangeIns(m_InstrumentList.GetCurSel());
		m_bInitialized = true;
	}
}
void XMSamplerUIInst::SetActivePage(int index) {
	m_tabMain.SetCurSel(index);
	m_genTab.ShowWindow(index==0?SW_SHOW:SW_HIDE);
	m_ampTab.ShowWindow(index==1?SW_SHOW:SW_HIDE);
	m_panTab.ShowWindow(index==2?SW_SHOW:SW_HIDE);
	m_filTab.ShowWindow(index==3?SW_SHOW:SW_HIDE);
	m_pitTab.ShowWindow(index==4?SW_SHOW:SW_HIDE);
	switch(index) 
	{
		case 0: m_genTab.SetFocus(); break;
		case 1: m_ampTab.SetFocus(); break;
		case 2: m_panTab.SetFocus(); break;
		case 3: m_filTab.SetFocus(); break;
		case 4: m_pitTab.SetFocus(); break;
		default: break;
	}
}
void XMSamplerUIInst::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = m_tabMain.GetCurSel();

	m_genTab.ShowWindow(iSel==0?SW_SHOW:SW_HIDE);
	m_ampTab.ShowWindow(iSel==1?SW_SHOW:SW_HIDE);
	m_panTab.ShowWindow(iSel==2?SW_SHOW:SW_HIDE);
	m_filTab.ShowWindow(iSel==3?SW_SHOW:SW_HIDE);
	m_pitTab.ShowWindow(iSel==4?SW_SHOW:SW_HIDE);

	*pResult = 0;
}

void XMSamplerUIInst::OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

			
void XMSamplerUIInst::OnBnClickedLoadins()
{
	OPENFILENAME ofn; // common dialog box structure
	char szFile[_MAX_PATH]; // buffer for file name
	szFile[0]='\0';
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter =
		"All Instrument types (*.xi *.iti" "\0" "*.xi;*.iti;" "\0"
		"FastTracker II Instruments (*.xi)"              "\0" "*.xi"                  "\0"
		"Impulse Tracker Instruments (*.iti)"             "\0" "*.iti"                  "\0"
		"All (*)"                                  "\0" "*"                     "\0"
		;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	std::string tmpstr = PsycleGlobal::conf().GetCurrentInstrumentDir();
	ofn.lpstrInitialDir = tmpstr.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn)==TRUE)
	{
		CString str = ofn.lpstrFile;
		int index = str.ReverseFind('.');
		if (index != -1)
		{
			CString ext = str.Mid(index+1);
			if (ext.CompareNoCase("XI") == 0)
			{
				XMSongLoader xmsong;
				xmsong.Open(ofn.lpstrFile);
				xmsong.LoadInstrumentFromFile(Global::song(),m_iCurrentSelected);
				xmsong.Close();
			}
			else if (ext.CompareNoCase("ITI") == 0)
			{
				ITModule2 itsong;
				itsong.Open(ofn.lpstrFile);
				itsong.LoadInstrumentFromFile(Global::song(),m_iCurrentSelected);
				itsong.Close();
			}
			SetInstrumentData(m_iCurrentSelected);
			char line[48];
			const XMInstrument& inst = Global::song().xminstruments[m_iCurrentSelected];
			sprintf(line,"%02X%s: ",m_iCurrentSelected,inst.IsEnabled()?"*":" ");
			strcat(line,inst.Name().c_str());
			m_InstrumentList.DeleteString(m_iCurrentSelected);
			m_InstrumentList.InsertString(m_iCurrentSelected,line);
		}
	}
}

void XMSamplerUIInst::OnBnClickedSaveins()
{
	///\todo Agregue aquí su código de controlador de notificación de control
}

void XMSamplerUIInst::OnBnClickedDupeins()
{
	for (int j=0;j<XMInstrument::MAX_INSTRUMENT;j++)
	{
		if (Global::song().xminstruments.Exists(j) == false ) 
		{
			const XMInstrument& inst = Global::song().xminstruments[m_iCurrentSelected];
			Global::song().xminstruments.SetInst(inst,j);
			FillInstrumentList();
			return;
		}
	}
	MessageBox("Couldn't find an appropiate instrument slot to copy to.","Error While Duplicating!");
}

void XMSamplerUIInst::OnBnClickedDeleteins()
{
	XMInstrument& inst = Global::song().xminstruments.get(m_iCurrentSelected);
	inst.Init();
	FillInstrumentList();
	SetInstrumentData(m_iCurrentSelected);
}
			
}}
