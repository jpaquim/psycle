#include <psycle/host/detail/project.private.hpp>
#include "InstrumentGenDlg.hpp"
#include "InstrumentEditorUI.hpp"
#include "XMSamplerUIInst.hpp"
#include "MainFrm.hpp"
#include "InstrNoteMap.hpp"
#include "PsycleConfig.hpp"
#include <psycle/host/Song.hpp>
#include <psycle/host/XMInstrument.hpp>

namespace psycle { namespace host {

	extern CPsycleApp theApp;

CInstrumentGenDlg::CInstrumentGenDlg()
: CDialog(CInstrumentGenDlg::IDD)
, m_bInitialized(false)
, m_instIdx(0)
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
	ON_BN_CLICKED(IDC_SET_ALL_SAMPLE,OnBtnPopupSetSample)
	ON_COMMAND_RANGE(IDC_INSTR_SETSAMPLES_0, IDC_INSTR_SETSAMPLES_255, OnSetSample)
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

	((CButton*)GetDlgItem(IDC_DECREASENOTE))->SetIcon(PsycleGlobal::conf().iconless);
	((CButton*)GetDlgItem(IDC_INCREASENOTE))->SetIcon(PsycleGlobal::conf().iconmore);
	((CButton*)GetDlgItem(IDC_DECREASEOCT))->SetIcon(PsycleGlobal::conf().iconlessless);
	((CButton*)GetDlgItem(IDC_INCREASEOCT))->SetIcon(PsycleGlobal::conf().iconmoremore);

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

	m_ShiftMove.AddString(_T("Change tune"));
	m_ShiftMove.AddString(_T("Move all"));
	m_ShiftMove.AddString(_T("Move notes"));
	m_ShiftMove.AddString(_T("Move samples"));
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
void CInstrumentGenDlg::AssignGeneralValues(XMInstrument& inst, int instno)
{
	m_bInitialized=false;
	m_instr = &inst;
	m_instIdx = instno;
	m_InstrumentName.SetWindowText(inst.Name().c_str());
	SetNewNoteAction(inst.NNA(),inst.DCT(),inst.DCA());
	m_SampleAssign.Initialize(inst);
	m_EnvelopeEditorDlg.ChangeEnvelope(inst.PitchEnvelope());
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
		ValidateEnabled();
		XMSamplerUIInst* win = dynamic_cast<XMSamplerUIInst*>(GetParent()->GetParent());
		win->FillInstrumentList(-2);
	}
}

void CInstrumentGenDlg::OnCbnSelendokInsNnacombo()
{
	m_instr->NNA((XMInstrument::NewNoteAction::Type)m_NNA.GetCurSel());
	ValidateEnabled();
}
	
void CInstrumentGenDlg::OnCbnSelendokInsDctcombo()
{
	m_instr->DCT((XMInstrument::DupeCheck::Type)m_DCT.GetCurSel());
	m_DCA.EnableWindow(m_DCT.GetCurSel() != 0);
	ValidateEnabled();
}

void CInstrumentGenDlg::OnCbnSelendokInsDcacombo()
{
	m_instr->DCA((XMInstrument::NewNoteAction::Type)m_DCA.GetCurSel());
	ValidateEnabled();
}

void CInstrumentGenDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int m_Octave = 8-m_SampleAssign.Octave();
	switch(nSBCode)
	{
		case SB_TOP:
			m_Octave=0;
			break;
		case SB_BOTTOM:
			m_Octave=8;
			break;
		case SB_LINEDOWN:
		case SB_PAGEDOWN:
			if ( m_Octave < 8) { m_Octave++; }
			break;
		case SB_LINEUP:
		case SB_PAGEUP:
			if ( m_Octave>0 ) { m_Octave--; }
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_Octave=std::max(0,std::min((int)nPos,8));
			break;
		default: 
			break;
	}
	if (m_Octave != m_SampleAssign.Octave()) {
		m_scBar.SetScrollPos(m_Octave);
		m_SampleAssign.Octave(8-m_Octave);
		m_SampleAssign.Invalidate();
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}



void CInstrumentGenDlg::OnBtnSetDefaults()
{
	m_instr->SetDefaultNoteMap(m_instIdx);
	m_SampleAssign.Invalidate();
	ValidateEnabled();
}
void CInstrumentGenDlg::OnBtnPopupSetSample()
{
	RECT tmp;
	((CButton*)GetDlgItem(IDC_SET_ALL_SAMPLE))->GetWindowRect(&tmp);
//	ClientToScreen(&tmp);

	CMenu menu;
	menu.CreatePopupMenu();

	SampleList & list = Global::song().samples;
	for (int i = 0; i <list.size() && i < 256 ; i += 16)
	{
		CMenu popup;
		popup.CreatePopupMenu();
		for(int j = i; (j < i + 16) && j < list.size(); j++)
		{
			char s1[64]={'\0'};
			std::sprintf(s1,"%02X%s: %s",j,list.IsEnabled(j)?"*":" "
				,list.Exists(j)?list[j].WaveName().c_str():"");
			popup.AppendMenu(MF_STRING, IDC_INSTR_SETSAMPLES_0 + j, s1);
		}
		char szSub[256] = "";;
		std::sprintf(szSub,"Samples %d-%d",i,i+15);
		menu.AppendMenu(MF_POPUP | MF_STRING,
			(UINT_PTR)popup.Detach(),
			szSub);
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, tmp.left, tmp.top, this);
	menu.DestroyMenu();

}

void CInstrumentGenDlg::OnSetSample(UINT nID)
{
	int sample = nID - IDC_INSTR_SETSAMPLES_0;
	for(int i=0;i< XMInstrument::NOTE_MAP_SIZE; i++) {
		XMInstrument::NotePair pair = m_instr->NoteToSample(i);
		pair.second = sample;
		m_instr->NoteToSample(i, pair);
	}
	m_SampleAssign.Invalidate();
	ValidateEnabled();
}


void CInstrumentGenDlg::OnBtnEditMapping()
{
	CInstrNoteMap map;
	map.m_instr = m_instr;
	map.m_instIdx = m_instIdx;
	if (map.DoModal() == IDOK) {
		XMSamplerUIInst* win = dynamic_cast<XMSamplerUIInst*>(GetParent()->GetParent());
		win->FillInstrumentList(-2);
		m_SampleAssign.Invalidate();
		ValidateEnabled();
	}
}


void CInstrumentGenDlg::OnBtnIncreaseOct()
{
	if (m_ShiftMove.GetCurSel()==0) { m_instr->TuneNotes(12); }
	else if (m_ShiftMove.GetCurSel()==1) { m_instr->MoveMapping(12); }
	else if (m_ShiftMove.GetCurSel()==2) { m_instr->MoveOnlyNotes(12); }
	else { m_instr->MoveOnlySamples(12); }
	m_SampleAssign.Invalidate();
	ValidateEnabled();
}
void CInstrumentGenDlg::OnBtnDecreaseOct()
{
	if (m_ShiftMove.GetCurSel()==0) { m_instr->TuneNotes(-12); }
	else if (m_ShiftMove.GetCurSel()==1) { m_instr->MoveMapping(-12); }
	else if (m_ShiftMove.GetCurSel()==2) { m_instr->MoveOnlyNotes(-12); }
	else { m_instr->MoveOnlySamples(-12); }
	m_SampleAssign.Invalidate();
	ValidateEnabled();
}
void CInstrumentGenDlg::OnBtnIncreaseNote()
{
	if (m_ShiftMove.GetCurSel()==0) { m_instr->TuneNotes(1); }
	else if (m_ShiftMove.GetCurSel()==1) { m_instr->MoveMapping(1); }
	else if (m_ShiftMove.GetCurSel()==2) { m_instr->MoveOnlyNotes(1); }
	else { m_instr->MoveOnlySamples(1); }
	m_SampleAssign.Invalidate();
	ValidateEnabled();
}
void CInstrumentGenDlg::OnBtnDecreaseNote()
{
	if (m_ShiftMove.GetCurSel()==0) { m_instr->TuneNotes(-1); }
	else if (m_ShiftMove.GetCurSel()==1) { m_instr->MoveMapping(-1); }
	else if (m_ShiftMove.GetCurSel()==2) { m_instr->MoveOnlyNotes(-1); }
	else { m_instr->MoveOnlySamples(-1); }
	m_SampleAssign.Invalidate();
	ValidateEnabled();
}


void CInstrumentGenDlg::ValidateEnabled() {
	if (m_instr->IsEnabled()) return;

	m_instr->IsEnabled(true);
	XMSamplerUIInst* win = dynamic_cast<XMSamplerUIInst*>(GetParent()->GetParent());
	win->FillInstrumentList(-2);
	CMainFrame* winMain = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	winMain->UpdateComboIns(true);
}
//void CInstrumentGenDlg
}}