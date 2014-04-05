#include <psycle/host/detail/project.private.hpp>
#include "InstrNoteMap.hpp"
#include <psycle/host/Song.hpp>
#include "PsycleConfig.hpp"

namespace psycle { namespace host {

CInstrNoteMap::CInstrNoteMap()
: CDialog(CInstrNoteMap::IDD)
{
}

CInstrNoteMap::~CInstrNoteMap()
{
}

void CInstrNoteMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SAMPLESLIST, m_SampleList);
	DDX_Control(pDX, IDC_NOTELIST, m_NoteList);
	DDX_Control(pDX, IDC_NOTEMAP_LIST, m_NoteMapList);
	DDX_Control(pDX, IDC_RADIO_EDIT, m_radio_edit);
}

BEGIN_MESSAGE_MAP(CInstrNoteMap, CDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_LBN_SELCHANGE(IDC_SAMPLESLIST, OnSelchangeSampleList)
	ON_LBN_SELCHANGE(IDC_NOTELIST, OnSelchangeNoteList)
	ON_LBN_SELCHANGE(IDC_NOTEMAP_LIST, OnSelchangeNoteMapList)
	ON_BN_CLICKED(IDC_RADIO_EDIT, OnRadioEdit)
	ON_BN_CLICKED(IDC_RADIO_SHOW, OnRadioShow)

END_MESSAGE_MAP()

BOOL CInstrNoteMap::PreTranslateMessage(MSG* pMsg)
{
/*	CWnd *tabCtl = GetParent();
	CWnd *UIInst = tabCtl->GetParent();
	InstrumentEditorUI* parent = dynamic_cast<InstrumentEditorUI*>(UIInst->GetParent());
	BOOL res = parent->PreTranslateChildMessage(pMsg, GetFocus()->GetSafeHwnd());
	if (res == FALSE )*/ return CDialog::PreTranslateMessage(pMsg);
//	return res;
}

BOOL CInstrNoteMap::OnInitDialog() 
{
	CDialog::OnInitDialog();
	for (int i = 0; i<XMInstrument::NOTE_MAP_SIZE; i++ ) {
		const XMInstrument::NotePair& pair = m_instr->NoteToSample(i);
		m_AssignNoteToSample[i] = pair;
	}

	CFont* m_font = &PsycleGlobal::conf().fixedFont;
	m_NoteList.SetColumnWidth(35);
	m_NoteList.SetFont(m_font);
	m_NoteMapList.SetFont(m_font);


	RefreshSampleList(-1);
	RefreshNoteList();
	RefreshNoteMapList();

	m_radio_edit.SetCheck(TRUE);

	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CInstrNoteMap::OnOk()
{
	for (int i = 0; i<XMInstrument::NOTE_MAP_SIZE; i++ ) {
		m_instr->NoteToSample(i,m_AssignNoteToSample[i]);
	}
	OnOK();		
}


void CInstrNoteMap::RefreshSampleList(int sample){

	char line[48];
	SampleList& list = Global::song().samples;
	if (sample == -1) {
		int i = m_SampleList.GetCurSel();
		m_SampleList.ResetContent();
		for (int i=0;i<XMInstrument::MAX_INSTRUMENT;i++)
		{
			if (list.Exists(i)) {
				const XMInstrument::WaveData<>& wave = list[i];
				sprintf(line,"%02X%s: ",i,wave.WaveLength()>0?"*":" ");
				strcat(line,wave.WaveName().c_str());
			}
			else {
				sprintf(line,"%02X : ",i);
			}
			m_SampleList.AddString(line);
		}
		if (i !=  LB_ERR) {
			m_SampleList.SetCurSel(i);
		}
	}
}

void CInstrNoteMap::RefreshNoteList(){
	PsycleConfig::PatternView & patView = PsycleGlobal::conf().patView();
	char ** notechars = (patView.showA440) ? patView.notes_tab_a440 : patView.notes_tab_a220;

	m_NoteList.ResetContent();
	for (int i = 0; i<120; i++ ) {
		m_NoteList.AddString(notechars[i]);
	}
}

void CInstrNoteMap::RefreshNoteMapList(bool reset/*=true*/){
	PsycleConfig::PatternView & patView = PsycleGlobal::conf().patView();
	char ** notechars = (patView.showA440) ? patView.notes_tab_a440 : patView.notes_tab_a220;
	char line[48];
	if (reset) {
		m_NoteMapList.ResetContent();
		
		for (int i = 0; i<XMInstrument::NOTE_MAP_SIZE; i++ ) {
			const XMInstrument::NotePair& pair = m_AssignNoteToSample[i];
			if(pair.second==255) { sprintf(line,"%s: %s --",notechars[i],notechars[pair.first]);}
			else { sprintf(line,"%s: %s %02X",notechars[i],notechars[pair.first],pair.second);}
			m_NoteMapList.AddString(line);
		}
	}
	else { 
		for (int i = 0; i<XMInstrument::NOTE_MAP_SIZE; i++ ) {
			bool isSel = (m_NoteMapList.GetSel(i) > 0);
			m_NoteMapList.DeleteString(i);

			const XMInstrument::NotePair& pair = m_AssignNoteToSample[i];
			if(pair.second==255) { sprintf(line,"%s: %s --",notechars[i],notechars[pair.first]);}
			else { sprintf(line,"%s: %s %02X",notechars[i],notechars[pair.first],pair.second);}

			m_NoteMapList.InsertString(i,line);
			m_NoteMapList.SetSel(i,isSel);
		}
	}
}

void CInstrNoteMap::OnSelchangeSampleList()
{
	int cursel = m_SampleList.GetCurSel();
	if(m_radio_edit.GetCheck()> 0) {
		if (m_SampleList.GetSelCount()>1) {
			if (m_NoteMapList.GetSelCount() != m_SampleList.GetSelCount()) {
				MessageBox("It is necessary that the selection of the mapping and the samples contain the same number of elements");
				m_SampleList.SelItemRange(false,0,m_SampleList.GetCount()-1);
			}
			else {
				int maxitems=m_NoteMapList.GetCount();
				int smpidx=0;
				for (int i=0;i<maxitems;i++) {
					if ( m_NoteMapList.GetSel(i) != 0) {
						while( m_SampleList.GetSel(smpidx)==0) smpidx++;
						XMInstrument::NotePair & pair = m_AssignNoteToSample[i];
						pair.second = smpidx;
						smpidx++;
					}
				}
				RefreshNoteMapList(false);
			}
		}
		else {
			int maxitems=m_NoteMapList.GetCount();
			for (int i=0;i<maxitems;i++ ) {
				if ( m_NoteMapList.GetSel(i) != 0) {
					XMInstrument::NotePair & pair = m_AssignNoteToSample[i];
					pair.second = cursel;
				}
			}
			RefreshNoteMapList(false);
		}
	}
	else {
		m_NoteList.SelItemRange(false,0,m_NoteList.GetCount()-1);
		m_NoteMapList.SelItemRange(false,0,m_NoteMapList.GetCount()-1);
		int maxitems=m_NoteMapList.GetCount();
		for (int i=0;i<maxitems;i++ ) {
			XMInstrument::NotePair & pair = m_AssignNoteToSample[i];
			if (pair.second == cursel ) {
				m_NoteMapList.SetSel(i,true);
				m_NoteList.SetSel(pair.first,true);
			}
		}
	}
}
void CInstrNoteMap::OnSelchangeNoteList()
{
	int cursel = m_NoteList.GetCurSel();
	if(m_radio_edit.GetCheck()> 0) {
		if (m_NoteList.GetSelCount()>1) {
			if (m_NoteMapList.GetSelCount() != m_NoteList.GetSelCount()) {
				MessageBox("It is necessary that the selection of the mapping and the samples contain the same number of elements");
				m_NoteList.SelItemRange(false,0,m_NoteList.GetCount()-1);
			}
			else {
				int maxitems=m_NoteMapList.GetCount();
				int noteidx=0;
				for (int i=0;i<maxitems;i++) {
					if ( m_NoteMapList.GetSel(i) != 0) {
						while( m_NoteList.GetSel(noteidx)==0) noteidx++;
						XMInstrument::NotePair & pair = m_AssignNoteToSample[i];
						pair.first = noteidx;
						noteidx++;
					}
				}
				RefreshNoteMapList(false);
			}
		}
		else {
			int maxitems=m_NoteMapList.GetCount();
			for (int i=0;i<maxitems;i++ ) {
				if ( m_NoteMapList.GetSel(i) != 0) {
					XMInstrument::NotePair & pair = m_AssignNoteToSample[i];
					pair.first = cursel;
				}
			}
			RefreshNoteMapList(false);
		}
	}
	else {
		m_SampleList.SelItemRange(false,0,m_SampleList.GetCount()-1);
		m_NoteMapList.SelItemRange(false,0,m_NoteMapList.GetCount()-1);
		int maxitems=m_NoteMapList.GetCount();
		for (int i=0;i<maxitems;i++ ) {
			XMInstrument::NotePair & pair = m_AssignNoteToSample[i];
			if (pair.first == cursel ) {
				m_NoteMapList.SetSel(i,true);
				if (pair.second != 255) {
					m_SampleList.SetSel(pair.second,true);
				}
			}
		}
	}
}
void CInstrNoteMap::OnSelchangeNoteMapList()
{
	m_SampleList.SelItemRange(false,0,m_SampleList.GetCount()-1);
	m_NoteList.SelItemRange(false,0,m_NoteList.GetCount()-1);
	
	int maxitems=m_NoteMapList.GetCount();
	for (int i=0;i<maxitems;i++ ) {
		if ( m_NoteMapList.GetSel(i) != 0) {
			const XMInstrument::NotePair & pair = m_AssignNoteToSample[i];
			if (pair.second != 255) {
				m_SampleList.SetSel(pair.second,true);
			}
			m_NoteList.SetSel(pair.first,true);
		}
	}
}
void CInstrNoteMap::OnRadioEdit()
{
	OnSelchangeNoteMapList();
}
void CInstrNoteMap::OnRadioShow()
{
	OnSelchangeNoteMapList();
}


//void CInstrumentGenDlg
}}