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

	const char GENTAB_NAME[] = "(4)Pitch";
	const char AMPTAB_NAME[] = "(5)Amplitude";
	const char PANTAB_NAME[] = "(6)Pan";
	const char FILTAB_NAME[] = "(7)Filter";

IMPLEMENT_DYNAMIC(XMSamplerUIInst, CPropertyPage)
XMSamplerUIInst::XMSamplerUIInst()
: CPropertyPage(XMSamplerUIInst::IDD)
, m_bInitialized(false)
, m_iCurrentSelected(0)
{
	strcpy(genTabName,GENTAB_NAME);
	strcpy(ampTabName,AMPTAB_NAME);
	strcpy(panTabName,PANTAB_NAME);
	strcpy(filTabName,FILTAB_NAME);
}

XMSamplerUIInst::~XMSamplerUIInst()
{
}

void XMSamplerUIInst::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INSTRUMENTLIST, m_InstrumentList);
	DDX_Control(pDX, IDC_INSTLIST_SAMPLES, m_InstListSamples);
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
	m_tabMain.InsertItem(0,genTabName);
	m_tabMain.InsertItem(1,ampTabName);
	m_tabMain.InsertItem(2,panTabName);
	m_tabMain.InsertItem(3,filTabName);
	m_genTab.Create(IDD_INST_SAMPULSE_INSTPIT_NEW,&m_tabMain);
	m_ampTab.Create(IDD_INST_SAMPULSE_INSTAMP,&m_tabMain);
	m_panTab.Create(IDD_INST_SAMPULSE_INSTPAN,&m_tabMain);
	m_filTab.Create(IDD_INST_SAMPULSE_INSTFIL,&m_tabMain);

	m_tabMain.GetWindowRect(rect);
	rect2.CopyRect(rect);
	TabCtrl_AdjustRect(m_tabMain.m_hWnd, FALSE, &rect); 
	rect.OffsetRect(rect.left -rect2.left - rect.left, rect.top - rect2.top - rect.top); 
	m_genTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);
	m_ampTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);
	m_panTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);
	m_filTab.SetWindowPos(&m_tabMain,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOREDRAW);

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
	m_InstrumentList.SetCurSel(instno);

	UpdateTabNames();
	UpdateInstrSamples();
}


BOOL XMSamplerUIInst::OnSetActive()
{
	TRACE("in setActive\n");
	if ( m_bInitialized == false )
	{
		FillInstrumentList();
		SetInstrumentData(Global::song().instSelected);
		m_bInitialized = true;
	}
	else {
		SetInstrumentData(Global::song().instSelected);
	}

	return CPropertyPage::OnSetActive();
}
void XMSamplerUIInst::FillInstrumentList(int instNo/*=-1*/) {
	char line[48];
	InstrumentList& list = Global::song().xminstruments;
	if (instNo == -1) {
		int i = m_InstrumentList.GetCurSel();
		m_InstrumentList.ResetContent();
		for (int i=0;i<XMInstrument::MAX_INSTRUMENT;i++)
		{
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
	else {
		int i=instNo;
		if (instNo == -2) {
			i = m_InstrumentList.GetCurSel();
		}
		if (list.Exists(i)) {
			const XMInstrument& inst = list[i];
			sprintf(line,"%02X%s: ",i,inst.IsEnabled()?"*":" ");
			strcat(line,inst.Name().c_str());
		}
		else {
			sprintf(line,"%02X : ",i);
		}
		m_InstrumentList.DeleteString(i);
		m_InstrumentList.InsertString(i, line);
		m_InstrumentList.SetCurSel(i);
	}
}
void XMSamplerUIInst::UpdateTabNames()
{
	XMInstrument& inst = Global::song().xminstruments.get(m_iCurrentSelected);

	strcpy(genTabName,GENTAB_NAME); 
	strcpy(ampTabName,AMPTAB_NAME);
	strcpy(panTabName,PANTAB_NAME);
	strcpy(filTabName,FILTAB_NAME);
	if (inst.PitchEnvelope().IsEnabled()) { strcat(genTabName," (e)"); }
	if (inst.AmpEnvelope().IsEnabled()) { strcat(ampTabName," (e)"); }
	if (inst.PanEnvelope().IsEnabled() && inst.NoteModPanSep() != 0) { strcat(panTabName," (e,n)"); }
	else if (inst.PanEnvelope().IsEnabled() ) { strcat(panTabName," (e)"); }
	else if (inst.NoteModPanSep() != 0 ) { strcat(panTabName," (n)"); }
	if (inst.FilterEnvelope().IsEnabled() && inst.FilterType() != dsp::F_NONE) { strcat(filTabName," (e,f)"); }
	else if (inst.FilterEnvelope().IsEnabled()) { strcat(filTabName," (e)"); }
	else if (inst.FilterType() != dsp::F_NONE) { strcat(filTabName," (f)"); }

	TCITEM tab;
	//TODO: Find the correct mask name
	tab.mask = 0x00000001;
	tab.pszText = genTabName;
	m_tabMain.SetItem(0,&tab);
	tab.pszText = ampTabName;
	m_tabMain.SetItem(1,&tab);
	tab.pszText = panTabName;
	m_tabMain.SetItem(2,&tab);
	tab.pszText = filTabName;
	m_tabMain.SetItem(3,&tab);
}
void XMSamplerUIInst::UpdateInstrSamples()
{
	char line[48];
	const SampleList &samplist =  Global::song().samples;
	const XMInstrument & inst = Global::song().xminstruments[m_InstrumentList.GetCurSel()];
	m_InstListSamples.ResetContent();
	if (inst.IsEnabled()) {
		std::set<int> sampNums =  inst.GetWavesUsed();
		for (std::set<int>::iterator it = sampNums.begin(); it != sampNums.end(); ++it) {
			if ( samplist.Exists(*it) ) {
				const XMInstrument::WaveData<> &wave = samplist[*it];
				sprintf(line,"%02X%s: %s",*it,samplist.IsEnabled(*it)?"*":" ", wave.WaveName().c_str());
			}
			else {
				sprintf(line,"%02X : ",*it);
			}
			m_InstListSamples.AddString(line);
		}
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
	switch(index) 
	{
		case 0: m_genTab.SetFocus(); break;
		case 1: m_ampTab.SetFocus(); break;
		case 2: m_panTab.SetFocus(); break;
		case 3: m_filTab.SetFocus(); break;
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
			//TODO: Needs to do XMSamplerUISample.WaveUpdate();
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
			FillInstrumentList(j);
			return;
		}
	}
	MessageBox(_T("Couldn't find an appropiate instrument slot to copy to."),_T("Error While Duplicating!"));
}

void XMSamplerUIInst::OnBnClickedDeleteins()
{
	XMInstrument & inst = Global::song().xminstruments.get(m_InstrumentList.GetCurSel());
	m_InstListSamples.ResetContent();
	if (inst.IsEnabled()) {
		std::set<int> sampNums =  inst.GetWavesUsed();
		if (sampNums.size() > 0) {
			std::ostringstream os;
			os << "This instrument uses the following samples: " << std::hex;
			for (std::set<int>::iterator it = sampNums.begin(); it != sampNums.end();++it) {
				os << *it << ',';
			}
			os << std::endl <<"Do you want to ALSO delete the samples?";
			int result = MessageBox(os.str().c_str(),"Deleting Instrument",MB_YESNOCANCEL | MB_ICONQUESTION);
			if (result == IDYES) {
				for (std::set<int>::iterator it = sampNums.begin(); it != sampNums.end();++it) {
					Global::song().samples.RemoveAt(*it);
				}
				//TODO: Needs to do XMSamplerUISample.WaveUpdate();
			}
			else if (result == IDCANCEL) {
				return;
			}
		}
		inst.Init();
		FillInstrumentList(-2);
		SetInstrumentData(m_iCurrentSelected);
	}
}

}}
