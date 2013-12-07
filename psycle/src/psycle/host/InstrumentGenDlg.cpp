#include <psycle/host/detail/project.private.hpp>
#include "InstrumentGenDlg.hpp"
#include "XMInstrument.hpp"
#include "InstrumentEditorUI.hpp"
#include "XMSamplerUIInst.hpp"

namespace psycle { namespace host {

CInstrumentGenDlg::CInstrumentGenDlg()
: CDialog(CInstrumentGenDlg::IDD)
, m_bInitialized(false)
{
}

CInstrumentGenDlg::~CInstrumentGenDlg()
{
}

void CInstrumentGenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INS_NAME, m_InstrumentName);
	DDX_Control(pDX, IDC_INS_NNACOMBO, m_NNA);
	DDX_Control(pDX, IDC_INS_DCTCOMBO, m_DCT);
	DDX_Control(pDX, IDC_INS_DCACOMBO, m_DCA);
	DDX_Control(pDX, IDC_SAMPLE_NUMBER, m_SampleNumber);
	DDX_Control(pDX, IDC_INS_SHIFTMOVE, m_ShiftMove);

	DDX_Control(pDX, IDC_INS_NOTEMAP, m_SampleAssign);
	DDX_Control(pDX, IDC_INS_NOTESCROLL, m_scBar);
}

BEGIN_MESSAGE_MAP(CInstrumentGenDlg, CDialog)
	ON_WM_VSCROLL()
	ON_EN_CHANGE(IDC_INS_NAME, OnEnChangeInsName)
	ON_CBN_SELENDOK(IDC_INS_NNACOMBO, OnCbnSelendokInsNnacombo)
	ON_CBN_SELENDOK(IDC_INS_DCTCOMBO, OnCbnSelendokInsDctcombo)
	ON_CBN_SELENDOK(IDC_INS_DCACOMBO, OnCbnSelendokInsDcacombo)
	ON_BN_CLICKED(IDC_SETDEFAULT,OnBtnSetDefaults)
	ON_BN_CLICKED(IDC_EDITMAPPING,OnBtnEditMapping)
	ON_BN_CLICKED(IDC_SET_ALL_SAMPLE,OnBtnSetSample)
	ON_BN_CLICKED(IDC_INCREASEOCT,OnBtnIncreaseOct)
	ON_BN_CLICKED(IDC_DECREASEOCT,OnBtnDecreaseOct)
	ON_BN_CLICKED(IDC_INCREASENOTE,OnBtnIncreaseNote)
	ON_BN_CLICKED(IDC_DECREASENOTE,OnBtnDecreaseNote)
END_MESSAGE_MAP()

BOOL CInstrumentGenDlg::PreTranslateMessage(MSG* pMsg)
{
	CWnd *tabCtl = GetParent();
	CWnd *UIInst = tabCtl->GetParent();
	InstrumentEditorUI* parent = dynamic_cast<InstrumentEditorUI*>(UIInst->GetParent());
	BOOL res = parent->PreTranslateChildMessage(pMsg, GetFocus()->GetSafeHwnd());
	if (res == FALSE ) return CDialog::PreTranslateMessage(pMsg);
	return res;
}

BOOL CInstrumentGenDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_InstrumentName.SetLimitText(31);
	m_SampleNumber.SetLimitText(2);

	m_NNA.AddString(_T("Note Cut"));
	m_NNA.AddString(_T("Note Continue"));
	m_NNA.AddString(_T("Note Off"));
	m_NNA.AddString(_T("Note Fadeout"));

	m_DCT.AddString(_T("Disabled"));
	m_DCT.AddString(_T("Note"));
	m_DCT.AddString(_T("Sample"));
	m_DCT.AddString(_T("Instrument"));

	m_DCA.AddString(_T("Note Cut"));
	m_DCA.AddString(_T("Note Continue"));
	m_DCA.AddString(_T("Note Off"));
	m_DCA.AddString(_T("Note Fadeout"));

	m_ShiftMove.AddString(_T("Tune"));
	m_ShiftMove.AddString(_T("Move"));
	m_ShiftMove.SetCurSel(0);

	SCROLLINFO info;
	m_scBar.GetScrollInfo(&info, SIF_PAGE|SIF_RANGE);
	info.fMask = SIF_RANGE|SIF_POS;
	info.nMin = 0;
	info.nMax  = 8;
	info.nPos = m_SampleAssign.Octave();
	m_scBar.SetScrollInfo(&info, false);

	m_EnvelopeEditorDlg.Create(CEnvelopeEditorDlg::IDD,this);
	CRect rect, rect2;
	((CStatic*)GetDlgItem(IDC_GROUP_ENV))->GetWindowRect(rect);
	this->GetWindowRect(rect2);
	rect.OffsetRect(-rect2.left,-rect2.top);
	m_EnvelopeEditorDlg.m_EnvelopeEditor.negative(true);
	m_EnvelopeEditorDlg.SetWindowPos(this,rect.left+12,rect.top+12,0,0,SWP_NOZORDER|SWP_NOREDRAW|SWP_NOSIZE);
	m_EnvelopeEditorDlg.ShowWindow(SW_SHOW);

	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CInstrumentGenDlg::AssignGeneralValues(XMInstrument& inst)
{
	m_bInitialized=false;
	m_instr = &inst;
	m_InstrumentName.SetWindowText(inst.Name().c_str());
	SetNewNoteAction(inst.NNA(),inst.DCT(),inst.DCA());
	m_SampleAssign.Initialize(inst);
	m_EnvelopeEditorDlg.m_EnvEnabled.SetCheck(inst.PitchEnvelope().IsEnabled());
	m_EnvelopeEditorDlg.m_CarryEnabled.SetCheck(inst.PitchEnvelope().IsCarry());
	m_EnvelopeEditorDlg.m_EnvelopeEditor.Initialize(inst.PitchEnvelope());
	m_bInitialized=true;
}

void CInstrumentGenDlg::SetNewNoteAction(const int nna,const int dct,const int dca)
{
	m_NNA.SetCurSel(nna);
	m_DCT.SetCurSel(dct);
	m_DCA.EnableWindow(m_DCT.GetCurSel() != 0);
	m_DCA.SetCurSel(dca);
}

void CInstrumentGenDlg::OnEnChangeInsName()
{
	if(m_bInitialized)
	{
		TCHAR _buf[256];
		m_InstrumentName.GetWindowText(_buf,sizeof(_buf));
		m_instr->Name(_buf);
		m_instr->IsEnabled(true);
		XMSamplerUIInst* win = dynamic_cast<XMSamplerUIInst*>(GetParent()->GetParent());
		win->FillInstrumentList();
	}
}

void CInstrumentGenDlg::OnCbnSelendokInsNnacombo()
{
	m_instr->NNA((XMInstrument::NewNoteAction::Type)m_NNA.GetCurSel());
	m_instr->IsEnabled(true);
}
	
void CInstrumentGenDlg::OnCbnSelendokInsDctcombo()
{
	m_instr->DCT((XMInstrument::DupeCheck::Type)m_DCT.GetCurSel());
	m_DCA.EnableWindow(m_DCT.GetCurSel() != 0);
	m_instr->IsEnabled(true);
}

void CInstrumentGenDlg::OnCbnSelendokInsDcacombo()
{
	m_instr->DCA((XMInstrument::NewNoteAction::Type)m_DCA.GetCurSel());
	m_instr->IsEnabled(true);
}

void CInstrumentGenDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int m_Octave = m_SampleAssign.Octave();
	switch(nSBCode)
	{
		case SB_TOP:
			m_Octave=0;
			break;
		case SB_BOTTOM:
			m_Octave=8;
			break;
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if ( m_Octave < 8) { m_Octave++; }
			break;
		case SB_LINELEFT:
		case SB_PAGELEFT:
			if ( m_Octave>0 ) { m_Octave--; }
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_Octave=(int)std::max(0,std::min((int)nPos,8));
			break;
		default: 
			break;
	}
	if (m_Octave != m_SampleAssign.Octave()) {
		m_scBar.SetScrollPos(m_Octave);
		m_SampleAssign.Octave(m_Octave);
		m_SampleAssign.Invalidate();
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CInstrumentGenDlg::OnBtnSetDefaults()
{
	m_instr->SetDefaultNoteMap();
	m_SampleAssign.Invalidate();
	m_instr->IsEnabled(true);
}
void CInstrumentGenDlg::OnBtnSetSample()
{
	TCHAR sample_number[5];
	m_SampleNumber.GetWindowText(sample_number,4);
	int sample = hexstring_to_integer(sample_number);
	for(int i=0;i< XMInstrument::NOTE_MAP_SIZE; i++) {
		XMInstrument::NotePair pair = m_instr->NoteToSample(i);
		pair.second = sample;
		m_instr->NoteToSample(i, pair);
	}
	m_SampleAssign.Invalidate();
	m_instr->IsEnabled(true);
	XMSamplerUIInst* win = dynamic_cast<XMSamplerUIInst*>(GetParent()->GetParent());
	win->FillInstrumentList();
}
void CInstrumentGenDlg::OnBtnEditMapping()
{
	MessageBox("Global setup of notes and instruments not ready yet.\nYou can set a sample globally, or setup each note and sample individually by clicking the corresponding note.");
}


void CInstrumentGenDlg::OnBtnIncreaseOct()
{
	if (m_ShiftMove.GetCurSel()==0) {
		TuneNotes(12);
	}
	else {
		MoveMapping(12);
	}
	m_SampleAssign.Invalidate();
	m_instr->IsEnabled(true);
}
void CInstrumentGenDlg::OnBtnDecreaseOct()
{
	if (m_ShiftMove.GetCurSel()==0) {
		TuneNotes(-12);
	}
	else {
		MoveMapping(-12);
	}
	m_SampleAssign.Invalidate();
	m_instr->IsEnabled(true);
}
void CInstrumentGenDlg::OnBtnIncreaseNote()
{
	if (m_ShiftMove.GetCurSel()==0) {
		TuneNotes(1);
	}
	else {
		MoveMapping(1);
	}
	m_SampleAssign.Invalidate();
	m_instr->IsEnabled(true);
}
void CInstrumentGenDlg::OnBtnDecreaseNote()
{
	if (m_ShiftMove.GetCurSel()==0) {
		TuneNotes(-1);
	}
	else {
		MoveMapping(-1);
	}
	m_SampleAssign.Invalidate();
	m_instr->IsEnabled(true);
}

void CInstrumentGenDlg::MoveMapping(int amount)
{
	if (amount < 0 ) {
		for(int i=0;i< XMInstrument::NOTE_MAP_SIZE; i++) {
			XMInstrument::NotePair pair = m_instr->NoteToSample(std::max((int)notecommands::c0,std::min(i-amount,(int)notecommands::b9)));
			m_instr->NoteToSample(i, pair);
		}
	}
	else {
		for(int i=XMInstrument::NOTE_MAP_SIZE-1; i>=0; i--) {
			XMInstrument::NotePair pair = m_instr->NoteToSample(std::max((int)notecommands::c0,std::min(i-amount,(int)notecommands::b9)));
			m_instr->NoteToSample(i, pair);
		}
	}
}

void CInstrumentGenDlg::TuneNotes(int amount)
{
	for(int i=0;i< XMInstrument::NOTE_MAP_SIZE; i++) {
		XMInstrument::NotePair pair = m_instr->NoteToSample(i);
		pair.first = std::max((int)notecommands::c0,std::min((int)pair.first+amount,(int)notecommands::b9));
		m_instr->NoteToSample(i, pair);
	}
}
}}