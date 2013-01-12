#include <psycle/host/detail/project.private.hpp>
#include "EnvelopeEditorDlg.hpp"
#include <psycle/host/Player.hpp>

namespace psycle { namespace host {

CEnvelopeEditorDlg::CEnvelopeEditorDlg()
: CDialog(CEnvelopeEditorDlg::IDD)
{
}

CEnvelopeEditorDlg::~CEnvelopeEditorDlg()
{
}

void CEnvelopeEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INS_ENVELOPE, m_EnvelopeEditor);
	DDX_Control(pDX, IDC_ENVCHECK, m_EnvEnabled);
	DDX_Control(pDX, IDC_ADSRBASE, m_SlADSRBase);
	DDX_Control(pDX, IDC_ADSRMOD, m_SlADSRMod);
	DDX_Control(pDX, IDC_ADSRATT, m_SlADSRAttack);
	DDX_Control(pDX, IDC_ADSRDEC, m_SlADSRDecay);
	DDX_Control(pDX, IDC_ADSRSUS, m_SlADSRSustain);
	DDX_Control(pDX, IDC_ADSRREL, m_SlADSRRelease);
}
BEGIN_MESSAGE_MAP(CEnvelopeEditorDlg, CDialog)
	ON_BN_CLICKED(IDC_ENVCHECK, OnBnClickedEnvcheck)
	ON_BN_CLICKED(IDC_ENVADSR, OnBnClickedEnvadsr)
	ON_BN_CLICKED(IDC_ENVFREEFORM, OnBnClickedEnvfreeform)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BOOL CEnvelopeEditorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_SlADSRBase.SetRangeMax(100);
	m_SlADSRMod.SetRangeMax(100);
	m_SlADSRAttack.SetRangeMax(256);
	m_SlADSRDecay.SetRangeMax(256);
	m_SlADSRSustain.SetRangeMax(100);
	m_SlADSRRelease.SetRangeMax(512);
	((CButton*)GetDlgItem(IDC_ENVFREEFORM))->SetCheck(TRUE);
	OnBnClickedEnvfreeform();
	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEnvelopeEditorDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		if (uId == IDC_ADSRBASE) { SliderBase(the_slider); }
		if (uId == IDC_ADSRMOD) { SliderMod(the_slider); }
		if (uId == IDC_ADSRATT) { SliderAttack(the_slider); }
		if (uId == IDC_ADSRDEC) { SliderDecay(the_slider); }
		if (uId == IDC_ADSRSUS) { SliderSustain(the_slider); }
		if (uId == IDC_ADSRREL) { SliderRelease(the_slider); }
		break;
	case TB_ENDTRACK:
		break;
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEnvelopeEditorDlg::OnBnClickedEnvcheck()
{
	m_EnvelopeEditor.envelope().IsEnabled(m_EnvEnabled.GetCheck()!=0);
}

void CEnvelopeEditorDlg::OnBnClickedEnvadsr()
{
	m_SlADSRBase.EnableWindow(TRUE);
	m_SlADSRMod.EnableWindow(TRUE);
	m_SlADSRAttack.EnableWindow(TRUE);
	m_SlADSRDecay.EnableWindow(TRUE);
	m_SlADSRSustain.EnableWindow(TRUE);
	m_SlADSRRelease.EnableWindow(TRUE);
	XMInstrument::Envelope& env = m_EnvelopeEditor.envelope();
	while(env.NumOfPoints()< 4) {
		env.Insert(env.NumOfPoints(),0.0f);
	}
	while(env.NumOfPoints()>4) {
		env.Delete(env.NumOfPoints()-1);
	}
	float min=1.0f;
	float max=0.0f;
	for(int i=0;i<env.NumOfPoints();i++){
		min=std::min(min,env.GetValue(i));
		max=std::max(max,env.GetValue(i));
	}
	env.SetValue(0,min);
	env.SetValue(1,max);
	env.SetValue(3,min);
	m_SlADSRBase.SetPos(min*100);
	if(max!=min) {
		m_SlADSRMod.SetPos((max-min)*100);
	}
	else {m_SlADSRMod.SetPos(100);}
	m_SlADSRAttack.SetPos(env.GetTime(1)-env.GetTime(0));
	m_SlADSRDecay.SetPos(env.GetTime(2)-env.GetTime(1));
	m_SlADSRSustain.SetPos(env.GetValue(2)*100.f);
	m_SlADSRRelease.SetPos(env.GetTime(3)-env.GetTime(2));

	m_EnvelopeEditor.freeform(false);
}
		
void CEnvelopeEditorDlg::OnBnClickedEnvfreeform()
{
	m_SlADSRBase.EnableWindow(FALSE);
	m_SlADSRMod.EnableWindow(FALSE);
	m_SlADSRAttack.EnableWindow(FALSE);
	m_SlADSRDecay.EnableWindow(FALSE);
	m_SlADSRSustain.EnableWindow(FALSE);
	m_SlADSRRelease.EnableWindow(FALSE);
	m_EnvelopeEditor.freeform(true);
}

void CEnvelopeEditorDlg::SliderBase(CSliderCtrl* slid)
{
	float diff = std::min(100-m_SlADSRBase.GetPos(),m_SlADSRMod.GetPos()) * 0.01f;
	float base = m_SlADSRBase.GetPos()*0.01f;
	XMInstrument::Envelope& env = m_EnvelopeEditor.envelope();
	env.SetValue(0, base);
	env.SetValue(1, base+diff);
	env.SetValue(2, base+((float)m_SlADSRSustain.GetPos()*0.01f*diff));
	env.SetValue(3, base);
	m_EnvelopeEditor.Invalidate();
	char tmp[64];
	sprintf(tmp,"%d%%",m_SlADSRBase.GetPos());
	((CStatic*)GetDlgItem(IDC_LADSRBASE))->SetWindowText(tmp);
}
void CEnvelopeEditorDlg::SliderMod(CSliderCtrl* slid)
{
	float diff = std::min(100-m_SlADSRBase.GetPos(),m_SlADSRMod.GetPos()) * 0.01f;
	float base = m_SlADSRBase.GetPos()*0.01f;
	XMInstrument::Envelope& env = m_EnvelopeEditor.envelope();
	env.SetValue(1, base+diff);
	env.SetValue(2, base+((float)m_SlADSRSustain.GetPos()*0.01f*diff));
	m_EnvelopeEditor.Invalidate();
	char tmp[64];
	sprintf(tmp,"%d%%",m_SlADSRMod.GetPos());
	((CStatic*)GetDlgItem(IDC_LADSRMOD))->SetWindowText(tmp);
}
void CEnvelopeEditorDlg::SliderAttack(CSliderCtrl* slid)
{
	m_EnvelopeEditor.envelope().SetTime(1,m_SlADSRAttack.GetPos());
	m_EnvelopeEditor.Invalidate();
	char tmp[64];
	sprintf(tmp,"%.0fms",m_SlADSRAttack.GetPos()*2500.f / Global::player().bpm );
	((CStatic*)GetDlgItem(IDC_LADSRATT))->SetWindowText(tmp);
}
void CEnvelopeEditorDlg::SliderDecay(CSliderCtrl* slid)
{
	m_EnvelopeEditor.envelope().SetTime(2,
			m_EnvelopeEditor.envelope().GetTime(1)+m_SlADSRDecay.GetPos());
	m_EnvelopeEditor.Invalidate();
	char tmp[64];
	sprintf(tmp,"%.0fms",m_SlADSRDecay.GetPos()*2500.f / Global::player().bpm );
	((CStatic*)GetDlgItem(IDC_LADSRDEC))->SetWindowText(tmp);
}
void CEnvelopeEditorDlg::SliderSustain(CSliderCtrl* slid)
{
	float diff = std::min(100-m_SlADSRBase.GetPos(),m_SlADSRMod.GetPos()) * 0.01f;
	float base = m_SlADSRBase.GetPos()*0.01f;
	m_EnvelopeEditor.envelope().SetValue(2, base+((float)m_SlADSRSustain.GetPos()*0.01f*diff));
	m_EnvelopeEditor.Invalidate();
	char tmp[64];
	sprintf(tmp,"%d%%",m_SlADSRSustain.GetPos());
	((CStatic*)GetDlgItem(IDC_LADSRSUS))->SetWindowText(tmp);
}
void CEnvelopeEditorDlg::SliderRelease(CSliderCtrl* slid)
{
	m_EnvelopeEditor.envelope().SetTime(3,
			m_EnvelopeEditor.envelope().GetTime(2)+m_SlADSRRelease.GetPos());
	m_EnvelopeEditor.Invalidate();
	char tmp[64];
	sprintf(tmp,"%.0fms",m_SlADSRRelease.GetPos()*2500.f / Global::player().bpm );
	((CStatic*)GetDlgItem(IDC_LADSRREL))->SetWindowText(tmp);
}

}}