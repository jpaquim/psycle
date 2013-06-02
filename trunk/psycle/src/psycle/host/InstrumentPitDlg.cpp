#include <psycle/host/detail/project.private.hpp>
#include "InstrumentPitDlg.hpp"
#include "InstrumentEditorUI.hpp"


namespace psycle { namespace host {

CInstrumentPitDlg::CInstrumentPitDlg()
: CDialog(CInstrumentPitDlg::IDD)
{
}

CInstrumentPitDlg::~CInstrumentPitDlg()
{
}

void CInstrumentPitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VOLCUTOFFPAN, m_SlVolCutoffPan);
	DDX_Control(pDX, IDC_SWING1, m_SlSwing1Glide);

}
BEGIN_MESSAGE_MAP(CInstrumentPitDlg, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BOOL CInstrumentPitDlg::PreTranslateMessage(MSG* pMsg)
{
	InstrumentEditorUI* parent = dynamic_cast<InstrumentEditorUI*>(GetParent()->GetParent());
	BOOL res = parent->PreTranslateChildMessage(pMsg, GetFocus()->GetSafeHwnd());
	if (res == FALSE ) return CDialog::PreTranslateMessage(pMsg);
	return res;
}

BOOL CInstrumentPitDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_SlVolCutoffPan.SetRange(-48, 48);
	//Hack to fix "0 placed on leftmost on start".
	m_SlVolCutoffPan.SetPos(-48);
	m_SlSwing1Glide.SetRangeMax(256);

//	m_SlNoteModNote.SetRange(0, 119);

//	m_SlNoteMod.SetRange(-32, 32);
	//Hack to fix "0 placed on leftmost on start".
//	m_SlNoteMod.SetPos(-32);
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

void CInstrumentPitDlg::AssignPitchValues(XMInstrument& inst)
{
	m_instr = &inst;
//	m_SlVolCutoffPan.SetPos(inst.Tune());
//	m_SlSwing1Glide.SetPos(inst.Glide());
//	m_SlNoteMod.SetPos(inst.Lines());
	SliderTune(&m_SlVolCutoffPan);
	SliderGlide(&m_SlSwing1Glide);

	m_EnvelopeEditorDlg.m_EnvEnabled.SetCheck(inst.PitchEnvelope().IsEnabled());
	m_EnvelopeEditorDlg.m_EnvelopeEditor.Initialize(inst.PitchEnvelope());
}

void CInstrumentPitDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		if (uId == IDC_VOLCUTOFFPAN) { SliderTune(the_slider); }
		if (uId == IDC_SWING1) { SliderGlide(the_slider); }
		break;
	case TB_ENDTRACK:
		break;
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CInstrumentPitDlg::SliderTune(CSliderCtrl* slid)
{
	char tmp[64];
	sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
	//m_inst->Tune(m_SlVolCutoffPan.GetPos());
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->SetWindowText(tmp);
}
void CInstrumentPitDlg::SliderGlide(CSliderCtrl* slid)
{
	char tmp[64];
	sprintf(tmp,"%d",m_SlSwing1Glide.GetPos());
	//m_inst->Glide(m_SlVolCutoffPan.GetPos());
	((CStatic*)GetDlgItem(IDC_LSWING))->SetWindowText(tmp);
}

/*
void CInstrumentPitDlg::SliderLines(CSliderCtrl* slid)
{
	char tmp[40];
	sprintf(tmp,"%.02f%%",(slid->GetPos()/2.56f));
	m_inst->NoteModPanSep(slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->SetWindowText(tmp);
}
*/
}}