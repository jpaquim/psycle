#include <psycle/host/detail/project.private.hpp>
#include "XMSamplerUISample.hpp"
#include <psycle/host/Song.hpp>
#include <psycle/host/player.hpp>
#include <psycle/host/PsycleConfig.hpp>
#include "InstrumentEditorUI.hpp"
#include "MainFrm.hpp"

namespace psycle { namespace host {


// XMSamplerUISample

IMPLEMENT_DYNAMIC(XMSamplerUISample, CPropertyPage)

XMSamplerUISample::XMSamplerUISample()
: CPropertyPage(XMSamplerUISample::IDD)
, m_Init(false)
, m_pWave(NULL)
{
}

XMSamplerUISample::~XMSamplerUISample()
{
}

void XMSamplerUISample::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLELIST, m_SampleList);
	DDX_Control(pDX, IDC_WAVESCOPE, m_WaveScope);
}



BEGIN_MESSAGE_MAP(XMSamplerUISample, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_SAMPLELIST, OnLbnSelchangeSamplelist)
	ON_CBN_SELENDOK(IDC_VIBRATOTYPE, OnCbnSelendokVibratotype)
	ON_CBN_SELENDOK(IDC_LOOP, OnCbnSelendokLoop)
	ON_CBN_SELENDOK(IDC_SUSTAINLOOP, OnCbnSelendokSustainloop)
	ON_EN_CHANGE(IDC_LOOPSTART, OnEnChangeLoopstart)
	ON_EN_CHANGE(IDC_LOOPEND, OnEnChangeLoopend)
	ON_EN_CHANGE(IDC_SUSTAINSTART, OnEnChangeSustainstart)
	ON_EN_CHANGE(IDC_SUSTAINEND, OnEnChangeSustainend)
	ON_EN_CHANGE(IDC_WAVENAME, OnEnChangeWavename)
	ON_EN_CHANGE(IDC_SAMPLERATE, OnEnChangeSamplerate)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINSAMPLERATE, OnDeltaposSpinsamplerate)
	ON_NOTIFY_RANGE(NM_CUSTOMDRAW, IDC_DEFVOLUME, IDC_VIBRATODEPTH, OnCustomdrawSliderm)
	ON_BN_CLICKED(IDC_OPENWAVEEDITOR, OnBnClickedOpenwaveeditor)
	ON_BN_CLICKED(IDC_LOAD, OnBnClickedLoad)
	ON_BN_CLICKED(IDC_SAVE, OnBnClickedSave)
	ON_BN_CLICKED(IDC_DUPE, OnBnClickedDupe)
	ON_BN_CLICKED(IDC_DELETE, OnBnClickedDelete)
	ON_BN_CLICKED(IDC_PANENABLED, OnBnClickedPanenabled)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BOOL XMSamplerUISample::PreTranslateMessage(MSG* pMsg) 
{
	InstrumentEditorUI* parent = dynamic_cast<InstrumentEditorUI*>(GetParent());
	BOOL res = parent->PreTranslateChildMessage(pMsg, GetFocus()->GetSafeHwnd());
	if (res == FALSE ) return CPropertyPage::PreTranslateMessage(pMsg);
	return res;
}
// Controladores de mensajes de XMSamplerUISample
BOOL XMSamplerUISample::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	((CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME))->SetRangeMax(128);
	((CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME))->SetRangeMax(128);
	((CSliderCtrl*)GetDlgItem(IDC_PAN))->SetRangeMax(128);
	((CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE))->SetRange(-60, 59);
	//Hack to fix "0 placed on leftmost on start".
	((CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE))->SetPos(-60);
	((CSliderCtrl*)GetDlgItem(IDC_FINETUNE))->SetRange(-100, 100);
	//Hack to fix "0 placed on leftmost on start".
	((CSliderCtrl*)GetDlgItem(IDC_FINETUNE))->SetPos(-100);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOATTACK))->SetRangeMax(255);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED))->SetRangeMax(64);
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH))->SetRangeMax(32);
	CComboBox* vibratoType = ((CComboBox*)GetDlgItem(IDC_VIBRATOTYPE));
	vibratoType->AddString("Sinus");
	vibratoType->AddString("Square");
	vibratoType->AddString("RampUp");
	vibratoType->AddString("RampDown");
	vibratoType->AddString("Random");
	CComboBox* sustainLoop = ((CComboBox*)GetDlgItem(IDC_SUSTAINLOOP));
	sustainLoop->AddString("Disabled");
	sustainLoop->AddString("Forward");
	sustainLoop->AddString("Bidirection");
	CComboBox* loop =  ((CComboBox*)GetDlgItem(IDC_LOOP));
	loop->AddString("Disabled");
	loop->AddString("Forward");
	loop->AddString("Bidirection");
	RefreshSampleList();
	m_SampleList.SetCurSel(0);
	return TRUE;
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void XMSamplerUISample::RefreshSampleList()
{
	int i = m_SampleList.GetCurSel();
	m_SampleList.ResetContent();
	SampleList& list = Global::song().samples;
	for (int i=0;i<XMInstrument::MAX_INSTRUMENT;i++)
	{
		char line[48];
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
BOOL XMSamplerUISample::OnSetActive()
{
	if ( m_SampleList.GetCurSel() == LB_ERR ) {
		m_SampleList.SetCurSel(0);
	}
	OnLbnSelchangeSamplelist();
	m_Init=true;

	return CPropertyPage::OnSetActive();
}

void XMSamplerUISample::OnLbnSelchangeSamplelist()
{
	m_Init=false;
	int i= m_SampleList.GetCurSel();
	if (Global::song().samples.Exists(i) == false) {
		XMInstrument::WaveData<> wave;
		wave.Init();
		Global::song().samples.SetSample(wave,i);
	}
	CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	win->ChangeIns(i);

	XMInstrument::WaveData<>& wave = Global::song().samples.get(i);
	pWave(&wave);
	RefreshSampleData();
	m_Init=true;
}

void XMSamplerUISample::RefreshSampleData()
{
	char tmp[40];
	XMInstrument::WaveData<>& wave = rWave();
	strcpy(tmp,wave.WaveName().c_str());
	((CEdit*)GetDlgItem(IDC_WAVENAME))->SetWindowText(tmp);
	sprintf(tmp,"%.0d",wave.WaveSampleRate());
	((CEdit*)GetDlgItem(IDC_SAMPLERATE))->SetWindowText(tmp);
	((CStatic*)GetDlgItem(IDC_WAVESTEREO))->SetWindowText(wave.IsWaveStereo()?"Stereo":"Mono");
	sprintf(tmp,"%d",wave.WaveLength());
	((CStatic*)GetDlgItem(IDC_WAVELENGTH))->SetWindowText(tmp);

	((CSliderCtrl*)GetDlgItem(IDC_GLOBVOLUME))->SetPos(int(wave.WaveGlobVolume()*128.0f));
	((CSliderCtrl*)GetDlgItem(IDC_DEFVOLUME))->SetPos(wave.WaveVolume());

	const int panpos=wave.PanFactor()*128.0f;
	((CButton*)GetDlgItem(IDC_PANENABLED))->SetCheck(wave.PanEnabled()?1:0);
	((CSliderCtrl*)GetDlgItem(IDC_PAN))->SetPos(panpos);
	FillPanDescription(panpos);

	((CSliderCtrl*)GetDlgItem(IDC_SAMPLENOTE))->SetPos(-1*wave.WaveTune());
	((CSliderCtrl*)GetDlgItem(IDC_FINETUNE))->SetPos(wave.WaveFineTune());

	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOATTACK))->SetPos(wave.VibratoAttack());
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATOSPEED))->SetPos(wave.VibratoSpeed());
	((CSliderCtrl*)GetDlgItem(IDC_VIBRATODEPTH))->SetPos(wave.VibratoDepth());
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

    //
	m_WaveScope.SetWave(&wave);
	DrawScope();
}
void XMSamplerUISample::OnBnClickedLoad()
{
	CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	win->ChangeIns(m_SampleList.GetCurSel());
	win->SendMessage(WM_COMMAND,IDC_LOADWAVE);
	XMInstrument::WaveData<>& wave = Global::song().samples.get(m_SampleList.GetCurSel());
	pWave(&wave);
	RefreshSampleList();
	RefreshSampleData();
}

void XMSamplerUISample::OnBnClickedSave()
{
	CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	win->ChangeIns(m_SampleList.GetCurSel());
	win->SendMessage(WM_COMMAND,IDC_SAVEWAVE);
}

void XMSamplerUISample::OnBnClickedDupe()
{
	for (int j=0;j<XMInstrument::MAX_INSTRUMENT;j++)
	{
		if (Global::song().samples.Exists(j) == false ) 
		{
			Global::song().samples.SetSample(rWave(),j);
			RefreshSampleList();
			return;
		}
	}
	MessageBox("Couldn't find an appropiate sample slot to copy to.","Error While Duplicating!");
}

void XMSamplerUISample::OnBnClickedDelete()
{
	rWave().Init();
	RefreshSampleList();
	RefreshSampleData();
	//\todo: Do a search for instruments using this sample and remove it from them.
}

void XMSamplerUISample::OnEnChangeWavename()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_WAVENAME);
		cedit->GetWindowText(tmp,40);
		rWave().WaveName(tmp);
		RefreshSampleList();
	}
}

void XMSamplerUISample::OnEnChangeSamplerate()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_SAMPLERATE);
		cedit->GetWindowText(tmp,40);
		int i = atoi(tmp);
		if (i==0) i=44100;
		rWave().WaveSampleRate(i);
	}
}
void XMSamplerUISample::OnDeltaposSpinsamplerate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	int newval;
	if ( pNMUpDown->iDelta < 0 ) {
		newval = rWave().WaveSampleRate()*2;
	} else {
		newval = rWave().WaveSampleRate()*0.5f;
		if (newval==0) newval=8363;
	}
	rWave().WaveSampleRate(newval);
	char tmp[40];
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_SAMPLERATE);
	sprintf(tmp,"%d",newval);
	cedit->SetWindowText(tmp);
	*pResult = 0;
}

void XMSamplerUISample::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		if (uId == IDC_DEFVOLUME) { SliderDefvolume(the_slider); }
		else if (uId == IDC_GLOBVOLUME) { SliderGlobvolume(the_slider); }
		else if (uId == IDC_PAN) { SliderPan(the_slider); }
		else if (uId == IDC_SAMPLENOTE) { SliderSamplenote(the_slider); }
		else if (uId == IDC_FINETUNE) { SliderFinetune(the_slider); }
		else if (uId == IDC_VIBRATOATTACK) { SliderVibratoAttack(the_slider); }
		else if (uId == IDC_VIBRATOSPEED) { SliderVibratospeed(the_slider); }
		else if (uId == IDC_VIBRATODEPTH) { SliderVibratodepth(the_slider); }
		break;
	case TB_ENDTRACK:
		break;
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void XMSamplerUISample::SliderDefvolume(CSliderCtrl* slid)
{
	rWave().WaveVolume(slid->GetPos());
}
void XMSamplerUISample:: SliderGlobvolume(CSliderCtrl* slid)
{
	rWave().WaveGlobVolume(slid->GetPos()/128.0f);
}
void XMSamplerUISample::SliderPan(CSliderCtrl* slid)
{
	CButton* check = (CButton*)GetDlgItem(IDC_PANENABLED);
	if ( check->GetCheck() != 2 ) // 2 == SurrounD
	{
		rWave().PanFactor(slid->GetPos()/128.0f);
	}
}
void XMSamplerUISample::SliderVibratoAttack(CSliderCtrl* slid)
{
	rWave().VibratoAttack(slid->GetPos());
}
void XMSamplerUISample::SliderVibratospeed(CSliderCtrl* slid)
{
	rWave().VibratoSpeed(slid->GetPos());
}
void XMSamplerUISample::SliderVibratodepth(CSliderCtrl* slid)
{
	rWave().VibratoDepth(slid->GetPos());
}
void XMSamplerUISample::SliderSamplenote(CSliderCtrl* slid)
{
	rWave().WaveTune(-1*slid->GetPos());
}
void XMSamplerUISample::SliderFinetune(CSliderCtrl* slid)
{
	rWave().WaveFineTune(slid->GetPos());
}

void XMSamplerUISample::OnBnClickedPanenabled()
{
	CButton* check = (CButton*)GetDlgItem(IDC_PANENABLED);
	if ( m_Init )
	{
		rWave().PanEnabled(check->GetCheck() > 0);
		rWave().IsSurround(check->GetCheck() == 2);
	}

	FillPanDescription(static_cast<int>(rWave().PanFactor()*128));
}

void XMSamplerUISample::FillPanDescription(int nPos) {
	char buffer[40];
	CButton* check = (CButton*)GetDlgItem(IDC_PANENABLED);
	if ( check->GetCheck() == 2 ) {
		strcpy(buffer, "SurrounD");
	}
	else{
		if ( nPos == 0) sprintf(buffer,"||%02d  ",nPos);
		else if ( nPos < 32) sprintf(buffer,"<<%02d  ",nPos);
		else if ( nPos < 64) sprintf(buffer," <%02d< ",nPos);
		else if ( nPos == 64) sprintf(buffer," |%02d| ",nPos);
		else if ( nPos <= 96) sprintf(buffer," >%02d> ",nPos);
		else if (nPos < 128) sprintf(buffer,"  %02d>>",nPos);
		else sprintf(buffer,"  %02d||",nPos);
	}
	((CStatic*)GetDlgItem(IDC_LPAN))->SetWindowText(buffer);
}


void XMSamplerUISample::OnCustomdrawSliderm(UINT idx, NMHDR* pNMHDR, LRESULT* pResult) 
{
	static const char notes[12][3]={"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
	NMCUSTOMDRAW nmcd = *reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	if (nmcd.dwDrawStage == CDDS_POSTPAINT)
	{
		char tmp[64];
		int label=0;
		CSliderCtrl* slider = reinterpret_cast<CSliderCtrl*>(GetDlgItem(pNMHDR->idFrom));
		if (pNMHDR->idFrom == IDC_DEFVOLUME) {
			sprintf(tmp,"C%02X",slider->GetPos());
			label = IDC_LDEFVOL;
		}
		else if(pNMHDR->idFrom == IDC_GLOBVOLUME) {
			std::ostringstream temp;
			temp.setf(std::ios::fixed);
			if(slider->GetPos()==0)
				temp<<"-inf. dB";
			else
			{
				float vol = float(slider->GetPos())/1.28f;
				float db = 20 * log10(vol/100.0f);
				temp<<std::setprecision(1)<<db<<"dB";
			}
			strcpy(tmp,temp.str().c_str());
			label = IDC_LGLOBVOL;
		}
		else if(pNMHDR->idFrom == IDC_PAN) {
			FillPanDescription(slider->GetPos());
			*pResult = CDRF_DODEFAULT;
			return;
		}
		else if (pNMHDR->idFrom == IDC_SAMPLENOTE) {
			if (rWave().WaveLength() > 0) {
				int offset = (PsycleGlobal::conf().patView().showA440) ? -1 : 0;
				int pos = slider->GetPos()-slider->GetRangeMin();
				sprintf(tmp,"%s%d", notes[pos%12], offset+(pos/12));
			}
			else {
				sprintf(tmp,"%s%d",notes[0],5);
			}
			label = IDC_LSAMPLENOTE;
		}
		else if (pNMHDR->idFrom == IDC_FINETUNE) {
			sprintf(tmp,"%d ct.",slider->GetPos());
			label = IDC_LFINETUNE;
		}
		else if (pNMHDR->idFrom == IDC_VIBRATOATTACK) {
			if ( slider->GetPos() == 0 ) strcpy(tmp,"No Delay");
			else sprintf(tmp,"%.0fms", (4096000.0f*Global::player().SamplesPerTick())
				/(slider->GetPos()*Global::player().SampleRate()));
			label = IDC_LVIBRATOATTACK;
		}
		else if (pNMHDR->idFrom == IDC_VIBRATOSPEED) {
			if (slider->GetPos() == 0) strcpy(tmp,"off");
			else sprintf(tmp,"%.0fms", (256000.0f*Global::player().SamplesPerTick())
				/(slider->GetPos()*Global::player().SampleRate()));
			label = IDC_LVIBRATOSPEED;
		}
		else if (pNMHDR->idFrom == IDC_VIBRATODEPTH) {
			if (slider->GetPos() == 0) strcpy(tmp,"off");
			else sprintf(tmp,"%d",slider->GetPos());
			label = IDC_LVIBRATODEPTH;
		}
		if (label != 0) {
			((CStatic*)GetDlgItem(label))->SetWindowText(tmp);
		}
		*pResult = CDRF_DODEFAULT;
	}
	else if (nmcd.dwDrawStage == CDDS_PREPAINT ){
		*pResult = CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
	}
	else {
		*pResult = CDRF_DODEFAULT;
	}
}

void XMSamplerUISample::OnBnClickedOpenwaveeditor()
{
	CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	win->SendMessage(WM_COMMAND,IDC_WAVEBUT);
}
void XMSamplerUISample::OnCbnSelendokVibratotype()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_VIBRATOTYPE);
	rWave().VibratoType((XMInstrument::WaveData<>::WaveForms::Type)cbox->GetCurSel());
}

void XMSamplerUISample::OnCbnSelendokLoop()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_LOOP);
	rWave().WaveLoopType((XMInstrument::WaveData<>::LoopType::Type)cbox->GetCurSel());
	DrawScope();
}

void XMSamplerUISample::OnCbnSelendokSustainloop()
{
	CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_SUSTAINLOOP);
	rWave().WaveSusLoopType((XMInstrument::WaveData<>::LoopType::Type)cbox->GetCurSel());
	DrawScope();
}

void XMSamplerUISample::OnEnChangeLoopstart()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_LOOPSTART);
		cedit->GetWindowText(tmp,40);
		rWave().WaveLoopStart(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::OnEnChangeLoopend()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_LOOPEND);
		cedit->GetWindowText(tmp,40);
		rWave().WaveLoopEnd(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::OnEnChangeSustainstart()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_SUSTAINSTART);
		cedit->GetWindowText(tmp,40);
		rWave().WaveSusLoopStart(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::OnEnChangeSustainend()
{
	char tmp[40];
	if ( m_Init )
	{
		CEdit* cedit = (CEdit*)GetDlgItem(IDC_SUSTAINEND);
		cedit->GetWindowText(tmp,40);
		rWave().WaveSusLoopEnd(atoi(tmp));
		DrawScope();
	}
}

void XMSamplerUISample::DrawScope()
{
	m_WaveScope.Invalidate();
}

}}
