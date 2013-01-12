#include <psycle/host/detail/project.private.hpp>
#include "InstrumentAmpDlg.hpp"

#include <psycle/host/Player.hpp>

namespace psycle { namespace host {

CInstrumentAmpDlg::CInstrumentAmpDlg()
: CDialog(CInstrumentAmpDlg::IDD)
{
}

CInstrumentAmpDlg::~CInstrumentAmpDlg()
{
}

void CInstrumentAmpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VOLCUTOFFPAN, m_SlVolCutoffPan);
	DDX_Control(pDX, IDC_SWING1, m_SlSwing1Glide);
	DDX_Control(pDX, IDC_FADEOUTRES, m_SlFadeoutRes);

	DDX_Control(pDX, IDC_SLNOTEMODNOTE, m_SlNoteModNote);
	DDX_Control(pDX, IDC_NOTEMOD, m_SlNoteMod);
	

}
BEGIN_MESSAGE_MAP(CInstrumentAmpDlg, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BOOL CInstrumentAmpDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_SlVolCutoffPan.SetRangeMax(128);
	m_SlVolCutoffPan.SetRangeMin(0);
	m_SlSwing1Glide.SetRangeMax(100);
	m_SlFadeoutRes.SetRangeMax(256);

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


void CInstrumentAmpDlg::AssignAmplitudeValues(XMInstrument& inst)
{
	m_instr = &inst;

	m_SlVolCutoffPan.SetPos(inst.GlobVol()*128.0f);
	m_SlFadeoutRes.SetPos(inst.VolumeFadeSpeed()*1024.0f);
	m_SlSwing1Glide.SetPos(inst.RandomVolume()*100.0f);

	//TODO: noteMod
	//m_SlNoteModNote.SetPos(inst.NoteModAmpCenter());
	//m_SlNoteMod.SetPos(inst.NoteModAmpSep());
	SliderVolume(&m_SlVolCutoffPan);
	SliderFadeout(&m_SlFadeoutRes);
	SliderGlideVol(&m_SlSwing1Glide);
	//SliderModNote(&m_SlNoteModNote);
	//SliderMod(&m_SlNoteMod);

	m_EnvelopeEditorDlg.m_EnvEnabled.SetCheck(inst.AmpEnvelope().IsEnabled());
	m_EnvelopeEditorDlg.m_EnvelopeEditor.Initialize(inst.AmpEnvelope());
}

void CInstrumentAmpDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		if (uId == IDC_VOLCUTOFFPAN) { SliderVolume(the_slider); }
		if (uId == IDC_SWING1) { SliderGlideVol(the_slider); }
		if (uId == IDC_FADEOUTRES) { SliderFadeout(the_slider); }
		if (uId == IDC_SLNOTEMODNOTE) { SliderModNote(the_slider); }
		if (uId == IDC_NOTEMOD) { SliderMod(the_slider); }
		break;
	case TB_ENDTRACK:
		break;
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
void CInstrumentAmpDlg::SliderVolume(CSliderCtrl* slid)
{
	char tmp[64];
	sprintf(tmp,"%d",m_SlVolCutoffPan.GetPos());
	m_instr->GlobVol(m_SlVolCutoffPan.GetPos()/128.0f);
	((CStatic*)GetDlgItem(IDC_LVOLCUTOFFPAN))->SetWindowText(tmp);
}

void CInstrumentAmpDlg::SliderFadeout(CSliderCtrl* slid)
{
	char tmp[64];

//		1024 / getpos() = number of ticks that needs to decrease to 0.
//		(24.0f * Global::player().bpm/60.0f) = number of ticks in a second.
//		sprintf(tmp,"%.0fms",(float) (1024/m_SlFadeoutRes.GetPos()) / (24.0f * Global::player().bpm/60.0f));
		if (m_SlFadeoutRes.GetPos() == 0) strcpy(tmp,"off");
		else sprintf(tmp,"%.0fms",2560000.0f/ (Global::player().bpm *m_SlFadeoutRes.GetPos()) );

	m_instr->VolumeFadeSpeed(m_SlFadeoutRes.GetPos()/1024.0f);
	((CStatic*)GetDlgItem(IDC_LFADEOUTRES))->SetWindowText(tmp);
}

void CInstrumentAmpDlg::SliderGlideVol(CSliderCtrl* slid)
{
	char tmp[64];
	sprintf(tmp,"%d%",m_SlSwing1Glide.GetPos());
	m_instr->RandomVolume(m_SlSwing1Glide.GetPos()/100.0f);
	((CStatic*)GetDlgItem(IDC_LSWING))->SetWindowText(tmp);
}
void CInstrumentAmpDlg::SliderModNote(CSliderCtrl* slid)
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

void CInstrumentAmpDlg::SliderMod(CSliderCtrl* slid)
{
/*
	char tmp[40];
	sprintf(tmp,"%.02f%%",(slid->GetPos()/2.56f));
	m_instr->NoteModPanSep(slid->GetPos());
	((CStatic*)GetDlgItem(IDC_LNOTEMOD))->SetWindowText(tmp);
*/
}
}}