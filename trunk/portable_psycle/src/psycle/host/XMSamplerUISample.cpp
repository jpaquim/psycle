#include "project.private.hpp"
#include "Psycle.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSampler.hpp"
#include ".\xmsampleruisample.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

// XMSamplerUISample

IMPLEMENT_DYNAMIC(XMSamplerUISample, CPropertyPage)
XMSamplerUISample::XMSamplerUISample()
: CPropertyPage(XMSamplerUISample::IDD)
{
}

XMSamplerUISample::~XMSamplerUISample()
{
}

void XMSamplerUISample::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLELIST, m_SampleList);
}



BEGIN_MESSAGE_MAP(XMSamplerUISample, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_SAMPLELIST, OnLbnSelchangeSamplelist)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_DEFVOLUME, OnNMCustomdrawDefvolume)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_GLOBVOLUME, OnNMCustomdrawGlobvolume)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_PAN, OnNMCustomdrawPan)
ON_CBN_SELENDOK(IDC_VIBRATOTYPE, OnCbnSelendokVibratotype)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIBRATORATE, OnNMCustomdrawVibratorate)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIBRATOSPEED, OnNMCustomdrawVibratospeed)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIBRATODEPTH, OnNMCustomdrawVibratodepth)
ON_CBN_SELENDOK(IDC_LOOP, OnCbnSelendokLoop)
ON_CBN_SELENDOK(IDC_SUSTAINLOOP, OnCbnSelendokSustainloop)
ON_EN_CHANGE(IDC_LOOPSTART, OnEnChangeLoopstart)
ON_EN_CHANGE(IDC_LOOPEND, OnEnChangeLoopend)
ON_EN_CHANGE(IDC_SUSTAINSTART, OnEnChangeSustainstart)
ON_EN_CHANGE(IDC_SUSTAINEND, OnEnChangeSustainend)
ON_EN_CHANGE(IDC_WAVENAME, OnEnChangeWavename)
ON_EN_CHANGE(IDC_SAMPLERATE, OnEnChangeSamplerate)
ON_NOTIFY(UDN_DELTAPOS, IDC_SPINSAMPLERATE, OnDeltaposSpinsamplerate)
END_MESSAGE_MAP()

// Controladores de mensajes de XMSamplerUISample
BOOL XMSamplerUISample::OnSetActive()
{
	((CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME))->SetRangeMax(128);
	((CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME))->SetRangeMax(128);
	((CSliderCtrl*)GetDlgItem(IDC_PAN))->SetRangeMax(128);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATORATE))->SetRangeMax(256);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED))->SetRangeMax(64);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH))->SetRangeMax(32);
	((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE))->AddString("Sinus");
	((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE))->AddString("Square");
	((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE))->AddString("RampUp");
	((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE))->AddString("RampDown");
	((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE))->AddString("Random");
	((CComboBox*)GetDlgItem(IDC_SUSTAINLOOP))->AddString("Disabled");
	((CComboBox*)GetDlgItem(IDC_SUSTAINLOOP))->AddString("Forward");
	((CComboBox*)GetDlgItem(IDC_SUSTAINLOOP))->AddString("Bidirection");
	((CComboBox*)GetDlgItem(IDC_LOOP))->AddString("Disabled");
	((CComboBox*)GetDlgItem(IDC_LOOP))->AddString("Forward");
	((CComboBox*)GetDlgItem(IDC_LOOP))->AddString("Bidirection");

	for (int i=0;i<XMSampler::MAX_INSTRUMENT;i++)
	{
		char line[48];
		XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
		sprintf(line,"%03d%s: ",i,wave.WaveLength()>0?"s":" ");
		strcat(line,wave.WaveName().c_str());
		m_SampleList.AddString(line);
	}
	m_SampleList.SetCurSel(0);
	OnLbnSelchangeSamplelist();

	return CPropertyPage::OnSetActive();
}

void XMSamplerUISample::OnLbnSelchangeSamplelist()
{
	char tmp[40];
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	strcpy(tmp,wave.WaveName().c_str());
	((CEdit*)GetDlgItem(IDC_WAVENAME))->SetWindowText(tmp);
//	((CEdit*)GetDlgItem(IDC_SAMPLERATE))->SetWindowText();
	((CStatic*)GetDlgItem(IDC_WAVESTEREO))->SetWindowText(wave.IsWaveStereo()?"Stereo":"Mono");
	sprintf(tmp,"%d",wave.WaveLength());
	((CStatic*)GetDlgItem(IDC_WAVELENGTH))->SetWindowText(tmp);
	((CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME))->SetPos(int(wave.WaveGlobVolume()*128.0f));
	((CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME))->SetPos(wave.WaveVolume());
	sprintf(tmp,"%.0f",wave.WaveGlobVolume()*128.0f);
	((CSliderCtrl*)GetDlgItem(IDC_LGLOBVOL))->SetWindowText(tmp);
	sprintf(tmp,"%d",wave.WaveVolume());
	((CSliderCtrl*)GetDlgItem(IDC_LDEFVOL))->SetWindowText(tmp);
	((CSliderCtrl*)GetDlgItem(IDC_PAN))->SetPos(int(wave.PanFactor()*128.0f));
	sprintf(tmp,"%.0f",wave.PanFactor()*128.0f);
	((CSliderCtrl*)GetDlgItem(IDC_LPAN))->SetWindowText(tmp);
	((CButton*)GetDlgItem(IDC_PANENABLED))->SetCheck(wave.PanEnabled()?1:0);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATORATE))->SetPos(wave.VibratoRate());
	if ( wave.VibratoRate()>0 ) sprintf(tmp,"%d",wave.VibratoRate());
	else strcpy(tmp,"Disabled");
	((CSliderCtrl*)GetDlgItem(IDC_LVIBRATORATE))->SetWindowText(tmp);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED))->SetPos(wave.VibratoSweep());
	sprintf(tmp,"%d",wave.VibratoSweep());
	((CSliderCtrl*)GetDlgItem(IDC_LVIBRATOSWEEP))->SetWindowText(tmp);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH))->SetPos(wave.VibratoDepth());
	sprintf(tmp,"%d",wave.VibratoDepth());
	((CSliderCtrl*)GetDlgItem(IDC_LVIBRATODEPTH))->SetWindowText(tmp);
	((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE))->SetCurSel((int)wave.VibratoType());
	((CComboBox*)GetDlgItem(IDC_SUSTAINLOOP))->SetCurSel((int)wave.WaveSusLoopType());
	sprintf(tmp,"%d",wave.WaveSusLoopStart());
	((CEdit*)GetDlgItem(IDC_SUSTAINSTART))->SetWindowText(tmp);
	sprintf(tmp,"%d",wave.WaveSusLoopEnd());
	((CEdit*)GetDlgItem(IDC_SUSTAINEND))->SetWindowText(tmp);
	((CComboBox*)GetDlgItem(IDC_LOOP))->SetCurSel((int)wave.WaveLoopType());
	sprintf(tmp,"%d",wave.WaveLoopStart());
	((CEdit*)GetDlgItem(IDC_LOOPSTART))->SetWindowText(tmp);
	sprintf(tmp,"%d",wave.WaveLoopEnd());
	((CEdit*)GetDlgItem(IDC_LOOPEND))->SetWindowText(tmp);

}

void XMSamplerUISample::OnNMCustomdrawDefvolume(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.WaveVolume(slid->GetPos());
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CSliderCtrl*)GetDlgItem(IDC_LDEFVOL))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawGlobvolume(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.WaveGlobVolume(slid->GetPos()/128.0f);
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CSliderCtrl*)GetDlgItem(IDC_LGLOBVOL))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawPan(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_PAN);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.PanFactor(slid->GetPos()/128.0f);
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CSliderCtrl*)GetDlgItem(IDC_LPAN))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnCbnSelendokVibratotype()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_VIBRATOTYPE);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.VibratoType(XMInstrument::WaveData::WaveForms(cbox->GetCurSel()));
}

void XMSamplerUISample::OnNMCustomdrawVibratorate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_VIBRATORATE);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.VibratoRate(slid->GetPos());
	char tmp[40];
	if ( wave.VibratoRate()>0 ) sprintf(tmp,"%d",wave.VibratoRate());
	else strcpy(tmp,"Disabled");
	((CSliderCtrl*)GetDlgItem(IDC_LVIBRATORATE))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawVibratospeed(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.VibratoSweep(slid->GetPos());
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CSliderCtrl*)GetDlgItem(IDC_LVIBRATOSWEEP))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnNMCustomdrawVibratodepth(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.VibratoDepth(slid->GetPos());
	char tmp[40];
	sprintf(tmp,"%d",slid->GetPos());
	((CSliderCtrl*)GetDlgItem(IDC_LVIBRATODEPTH))->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnCbnSelendokLoop()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_LOOP);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.WaveLoopType(XMInstrument::WaveData::LoopType(cbox->GetCurSel()));
}

void XMSamplerUISample::OnCbnSelendokSustainloop()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_SUSTAINLOOP);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	wave.WaveSusLoopType(XMInstrument::WaveData::LoopType(cbox->GetCurSel()));
}

void XMSamplerUISample::OnEnChangeLoopstart()
{
	char tmp[40];
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_LOOPSTART);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	cedit->GetWindowText(tmp,40);
	wave.WaveLoopStart(atoi(tmp));
}

void XMSamplerUISample::OnEnChangeLoopend()
{
	char tmp[40];
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_LOOPEND);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	cedit->GetWindowText(tmp,40);
	wave.WaveLoopEnd(atoi(tmp));
}

void XMSamplerUISample::OnEnChangeSustainstart()
{
	char tmp[40];
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_SUSTAINSTART);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	cedit->GetWindowText(tmp,40);
	wave.WaveSusLoopStart(atoi(tmp));
}

void XMSamplerUISample::OnEnChangeSustainend()
{
	char tmp[40];
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_SUSTAINEND);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	cedit->GetWindowText(tmp,40);
	wave.WaveSusLoopEnd(atoi(tmp));
}

void XMSamplerUISample::OnEnChangeWavename()
{
	char tmp[40];
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_WAVENAME);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	cedit->GetWindowText(tmp,40);
	wave.WaveName(tmp);
}

void XMSamplerUISample::OnEnChangeSamplerate()
{
/*	char tmp[40];
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_SAMPLERATE);
	int i= m_SampleList.GetCurSel();
	XMInstrument::WaveData& wave = m_pMachine->SampleData(i);
	cedit->GetWindowText(tmp,40);
	wave.SampleRate(atoi(str));
*/
}

void XMSamplerUISample::OnDeltaposSpinsamplerate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta > 0 )
	{
		//\todo : increase one semitone/octave
	} else {
		//\todo : decrease one semitone/octave
	}
	*pResult = 0;
}

NAMESPACE__END
NAMESPACE__END
