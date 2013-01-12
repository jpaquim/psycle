#include <psycle/host/detail/project.private.hpp>
#include "InstrumentFilDlg.hpp"


namespace psycle { namespace host {

CInstrumentFilDlg::CInstrumentFilDlg()
: CDialog(CInstrumentFilDlg::IDD)
{
}

CInstrumentFilDlg::~CInstrumentFilDlg()
{
}

void CInstrumentFilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FILTERTYPE, m_FilterType);
	DDX_Control(pDX, IDC_VOLCUTOFFPAN, m_SlVolCutoffPan);
	DDX_Control(pDX, IDC_SWING1, m_SlSwing1Glide);
	DDX_Control(pDX, IDC_FADEOUTRES, m_SlFadeoutRes);
	DDX_Control(pDX, IDC_SWING2, m_SlSwing2);

	DDX_Control(pDX, IDC_SLNOTEMODNOTE, m_SlNoteModNote);
	DDX_Control(pDX, IDC_NOTEMOD, m_SlNoteMod);
	
	DDX_Control(pDX, IDC_CUTOFFPAN, m_cutoffPan);
	DDX_Control(pDX, IDC_RESSONANCE, m_Ressonance);

}

BEGIN_MESSAGE_MAP(CInstrumentFilDlg, CDialog)
	ON_CBN_SELENDOK(IDC_FILTERTYPE, OnCbnSelendokFiltertype)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BOOL CInstrumentFilDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_FilterType.AddString(_T("LowPass"));
	m_FilterType.AddString(_T("HighPass"));
	m_FilterType.AddString(_T("BandPass"));
	m_FilterType.AddString(_T("NotchBand"));
	m_FilterType.AddString(_T("Off"));

	m_SlVolCutoffPan.SetRangeMax(127);
	m_SlVolCutoffPan.SetRangeMin(0);
	m_SlSwing1Glide.SetRangeMax(100);
	m_SlFadeoutRes.SetRangeMax(127);
	m_SlSwing2.SetRangeMax(100);

	m_SlNoteModNote.SetRangeMin(0);
	m_SlNoteModNote.SetRangeMax(119);

	m_SlNoteMod.SetRangeMax(32);
	m_SlNoteMod.SetRangeMin(-32);
	//Hack to fix "0 placed on leftmost on start".
	m_SlNoteMod.SetPos(1);
	m_EnvelopeEditorDlg.Create(CEnvelopeEditorDlg::IDD,this);
	CRect rect, rect2;
	((CStatic*)GetDlgItem(IDC_GROUP_ENV))->GetWindowRect(rect);
	this->GetWindowRect(rect2);
	rect.OffsetRect(-rect2.left,-rect2.top);
	m_EnvelopeEditorDlg.SetWindowPos(this,rect.left+12,rect.top+12,0,0,SWP_NOZORDER|SWP_NOREDRAW|SWP_NOSIZE);
	m_EnvelopeEditorDlg.ShowWindow(SW_SHOW);
	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE

}
void CInstrumentFilDlg::AssignFilterValues(XMInstrument& inst)
{
	m_instr = &inst;

	m_FilterType.SetCurSel((int)inst.FilterType());

	m_SlVolCutoffPan.SetPos(inst.FilterCutoff()&0x7F);
	if (inst.FilterCutoff()) m_cutoffPan.SetCheck(!(inst.FilterCutoff()&0x80));
	else m_cutoffPan.SetCheck(0);

	m_SlFadeoutRes.SetPos(inst.FilterResonance()&0x7F);
	if (inst.FilterResonance()) m_Ressonance.SetCheck(!(inst.FilterResonance()&0x80));
	else m_Ressonance.SetCheck(0);
	m_SlSwing1Glide.SetPos(inst.RandomCutoff()*100.0f);
	m_SlSwing2.SetPos(inst.RandomResonance()*100.0f);
//	m_SlNoteModNote.SetPos(inst.NoteModFilterCenter());
//	m_SlNoteMod.SetPos(inst.NoteModFilterSep());
	SliderCutoff(&m_SlVolCutoffPan);
	SliderRessonance(&m_SlFadeoutRes);
	SliderGlideCut(&m_SlSwing1Glide);
	SliderGlideRes(&m_SlSwing2);
//	SliderModNote(&m_SlNoteModNote);
//	SliderMod(&m_SlNoteMod);

	m_EnvelopeEditorDlg.m_EnvEnabled.SetCheck(inst.FilterEnvelope().IsEnabled());
	m_EnvelopeEditorDlg.m_EnvelopeEditor.Initialize(inst.FilterEnvelope());
}

void CInstrumentFilDlg::OnCbnSelendokFiltertype()
{

	m_instr->FilterType((dsp::FilterType)m_FilterType.GetCurSel());
}

void CInstrumentFilDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* the_slider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
	int uId = the_slider->GetDlgCtrlID();

	switch(nSBCode){
	case TB_BOTTOM: //fallthrough
	case TB_LINEDOWN: //fallthrough
	case TB_PAGEDOWN: //fallthrough
	case TB_TOP: //fallthrough
	case TB_LINEUP: //fallthrough
	case TB_PAGEUP: //fallthrough
	case TB_THUMBPOSITION: //fallthrough
	case TB_THUMBTRACK:
		if (uId == IDC_VOLCUTOFFPAN) { SliderCutoff(the_slider); }
		if (uId == IDC_SWING1) { SliderGlideCut(the_slider); }
		if (uId == IDC_FADEOUTRES) { SliderRessonance(the_slider); }
		if (uId == IDC_SWING2) { SliderGlideRes(the_slider); }
		if (uId == IDC_SLNOTEMODNOTE) { SliderModNote(the_slider); }
		if (uId == IDC_NOTEMOD) { SliderMod(the_slider); }
		break;
	case TB_ENDTRACK:
		break;
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
void CInstrumentFilDlg::OnBnEnableCutoff()
{
	if (m_cutoffPan.GetCheck()!=0) {
		m_instr->FilterCutoff(m_instr->FilterCutoff() & (0x80));
	}
	else {
		m_instr->FilterCutoff(m_instr->FilterCutoff() | (~0x80));
	}
}
void CInstrumentFilDlg::OnBnEnableRessonance()
{
	if (m_Ressonance.GetCheck()!=0) {
		m_instr->FilterResonance(m_instr->FilterResonance() & (0x80));
	}
	else {
		m_instr->FilterResonance(m_instr->FilterResonance() | (~0x80));
	}
}

void CInstrumentFilDlg::SliderCutoff(CSliderCtrl* slid)
{
	char tmp[64];
	sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
	m_instr->FilterCutoff(m_SlVolCutoffPan.GetPos());
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->SetWindowText(tmp);
}
void CInstrumentFilDlg::SliderRessonance(CSliderCtrl* slid)
{
	char tmp[64];
	sprintf(tmp,"%d",m_SlFadeoutRes.GetPos());
	m_instr->FilterResonance(m_SlFadeoutRes.GetPos());
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->SetWindowText(tmp);
}
void CInstrumentFilDlg::SliderGlideCut(CSliderCtrl* slid)
{
	char tmp[64];
	sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
	m_instr->RandomCutoff(m_SlSwing1Glide.GetPos()/100.0f);
	((CStatic*)GetDlgItem(IDC_LSWING))->SetWindowText(tmp);
}
void CInstrumentFilDlg::SliderGlideRes(CSliderCtrl* slid)
{
	char tmp[64];
	if (((CButton*)GetDlgItem(IDC_RESSONANCE))->GetCheck())
	{
		sprintf(tmp,"%d%",m_SlSwing2.GetPos());
		m_instr->RandomResonance(m_SlSwing2.GetPos()/100.0f);
	}
	else {
		strcpy(tmp,"off");
	}

	((CStatic*)GetDlgItem(IDC_LSWING2))->SetWindowText(tmp);
}
void CInstrumentFilDlg::SliderModNote(CSliderCtrl* slid)
{
/*
	char tmp[40], tmp2[40];
	char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
	sprintf(tmp,"%s",notes[slid->GetPos()%12]);
	sprintf(tmp2,"%s%d",tmp,(slid->GetPos()/12));
	m_instr->NoteModPanCenter(slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LNOTEMODNOTE))->SetWindowText(tmp2);
*/
}

void CInstrumentFilDlg::SliderMod(CSliderCtrl* slid)
{
/*
	char tmp[40];
	sprintf(tmp,"%.02f%%",(slid->GetPos()/2.56f));
	m_instr->NoteModPanSep(slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->SetWindowText(tmp);
*/
}
}}