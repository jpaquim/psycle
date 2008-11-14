// XMSamplerMixerPage.cpp: archivo de implementación
//


#include "XMSamplerMixerPage.hpp"
#include "Psycle.hpp"
#include "Song.hpp"
#include "XMSampler.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)
// Cuadro de diálogo de XMSamplerMixerPage

const int XMSamplerMixerPage::dlgName[8] = {
	IDC_NAME1,IDC_NAME2,IDC_NAME3,IDC_NAME4,
	IDC_NAME5,IDC_NAME6,IDC_NAME7,IDC_NAME8
};
const int XMSamplerMixerPage::dlgVol[8] = {
	IDC_SL_VOL1,IDC_SL_VOL2,IDC_SL_VOL3,IDC_SL_VOL4,
	IDC_SL_VOL5,IDC_SL_VOL6,IDC_SL_VOL7,IDC_SL_VOL8
};
const int XMSamplerMixerPage::dlgSurr[8] = {
	IDC_CH_SURR1,IDC_CH_SURR2,IDC_CH_SURR3,IDC_CH_SURR4,
	IDC_CH_SURR5,IDC_CH_SURR6,IDC_CH_SURR7,IDC_CH_SURR8
};
const int XMSamplerMixerPage::dlgPan[8] = {
	IDC_SL_PAN1,IDC_SL_PAN2,IDC_SL_PAN3,IDC_SL_PAN4,
	IDC_SL_PAN5,IDC_SL_PAN6,IDC_SL_PAN7,IDC_SL_PAN8
};
const int XMSamplerMixerPage::dlgRes[8] = {
	IDC_SL_RES1,IDC_SL_RES2,IDC_SL_RES3,IDC_SL_RES4,
	IDC_SL_RES5,IDC_SL_RES6,IDC_SL_RES7,IDC_SL_RES8
};
const int XMSamplerMixerPage::dlgCut[8] = {
	IDC_SL_CUTOFF1,IDC_SL_CUTOFF2,IDC_SL_CUTOFF3,IDC_SL_CUTOFF4,
	IDC_SL_CUTOFF5,IDC_SL_CUTOFF6,IDC_SL_CUTOFF7,IDC_SL_CUTOFF8
};
const int XMSamplerMixerPage::dlgMute[8] = {
	IDC_CH_MUTE1,IDC_CH_MUTE2,IDC_CH_MUTE3,IDC_CH_MUTE4,
	IDC_CH_MUTE5,IDC_CH_MUTE6,IDC_CH_MUTE7,IDC_CH_MUTE8
};

int const XMSamplerMixerPage::volumeRange = 200;
int const XMSamplerMixerPage::panningRange = 200;
int const XMSamplerMixerPage::resRange = 127;
int const XMSamplerMixerPage::cutoffRange = 127;


IMPLEMENT_DYNAMIC(XMSamplerMixerPage, CPropertyPage)
XMSamplerMixerPage::XMSamplerMixerPage()
	: CPropertyPage(XMSamplerMixerPage::IDD)
	, m_ChannelOffset(0)
	, m_UpdatingGraphics(true)
{
}

XMSamplerMixerPage::~XMSamplerMixerPage()
{
}

void XMSamplerMixerPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(XMSamplerMixerPage, CPropertyPage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF1, OnNMCustomdrawSlCutoff1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF1, OnNMCustomdrawSlCutoff1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF2, OnNMCustomdrawSlCutoff2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF3, OnNMCustomdrawSlCutoff3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF4, OnNMCustomdrawSlCutoff4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF5, OnNMCustomdrawSlCutoff5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF6, OnNMCustomdrawSlCutoff6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF7, OnNMCustomdrawSlCutoff7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CUTOFF8, OnNMCustomdrawSlCutoff8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES1, OnNMCustomdrawSlRes1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES2, OnNMCustomdrawSlRes2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES3, OnNMCustomdrawSlRes3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES4, OnNMCustomdrawSlRes4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES5, OnNMCustomdrawSlRes5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES6, OnNMCustomdrawSlRes6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES7, OnNMCustomdrawSlRes7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_RES8, OnNMCustomdrawSlRes8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN1, OnNMCustomdrawSlPan1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN2, OnNMCustomdrawSlPan2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN3, OnNMCustomdrawSlPan3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN4, OnNMCustomdrawSlPan4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN5, OnNMCustomdrawSlPan5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN6, OnNMCustomdrawSlPan6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN7, OnNMCustomdrawSlPan7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_PAN8, OnNMCustomdrawSlPan8)
	ON_BN_CLICKED(IDC_CH_SURR1, OnBnClickedChSurr1)
	ON_BN_CLICKED(IDC_CH_SURR2, OnBnClickedChSurr2)
	ON_BN_CLICKED(IDC_CH_SURR3, OnBnClickedChSurr3)
	ON_BN_CLICKED(IDC_CH_SURR4, OnBnClickedChSurr4)
	ON_BN_CLICKED(IDC_CH_SURR5, OnBnClickedChSurr5)
	ON_BN_CLICKED(IDC_CH_SURR6, OnBnClickedChSurr6)
	ON_BN_CLICKED(IDC_CH_SURR7, OnBnClickedChSurr7)
	ON_BN_CLICKED(IDC_CH_SURR8, OnBnClickedChSurr8)
	ON_BN_CLICKED(IDC_CH_MUTE1, OnBnClickedChMute1)
	ON_BN_CLICKED(IDC_CH_MUTE2, OnBnClickedChMute2)
	ON_BN_CLICKED(IDC_CH_MUTE3, OnBnClickedChMute3)
	ON_BN_CLICKED(IDC_CH_MUTE4, OnBnClickedChMute4)
	ON_BN_CLICKED(IDC_CH_MUTE5, OnBnClickedChMute5)
	ON_BN_CLICKED(IDC_CH_MUTE6, OnBnClickedChMute6)
	ON_BN_CLICKED(IDC_CH_MUTE7, OnBnClickedChMute7)
	ON_BN_CLICKED(IDC_CH_MUTE8, OnBnClickedChMute8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL1, OnNMCustomdrawSlVol1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL2, OnNMCustomdrawSlVol2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL3, OnNMCustomdrawSlVol3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL4, OnNMCustomdrawSlVol4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL5, OnNMCustomdrawSlVol5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL6, OnNMCustomdrawSlVol6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL7, OnNMCustomdrawSlVol7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL8, OnNMCustomdrawSlVol8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_VOL_MASTER, OnNMCustomdrawSlVolMaster)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SL_CHANNELS, OnNMCustomdrawSlChannels)
END_MESSAGE_MAP()


// Controladores de mensajes de XMSamplerMixerPage
BOOL XMSamplerMixerPage::OnSetActive()
{
	m_UpdatingGraphics=true;
	((CSliderCtrl*)GetDlgItem(IDC_SL_CHANNELS))->SetRangeMax((Global::_pSong->SongTracks()>8)?(Global::_pSong->SongTracks()-8):0); // maxchans-visiblechans
	if ( ((CButton*)GetDlgItem(IDC_R_SHOWVOICE))->GetCheck() == 0 ) { ((CButton*)GetDlgItem(IDC_R_SHOWCHAN))->SetCheck(1); }
	for (int i=0;i<8;i++)
	{
		((CSliderCtrl*)GetDlgItem(dlgVol[i]))->SetRangeMax(volumeRange);
		((CSliderCtrl*)GetDlgItem(dlgPan[i]))->SetRangeMax(panningRange);
		((CSliderCtrl*)GetDlgItem(dlgRes[i]))->SetRangeMax(resRange);
		((CSliderCtrl*)GetDlgItem(dlgCut[i]))->SetRangeMax(cutoffRange);
		UpdateChannel(i);
	}
	((CSliderCtrl*)GetDlgItem(IDC_SL_VOL_MASTER))->SetRangeMax(128);
	((CProgressCtrl*)GetDlgItem(IDC_LEFT_VU))->SetRange(0,97);
	UpdateMaster();
	return CPropertyPage::OnSetActive();
}

// Refreshes the values of all the controls of the dialog, except IDC_SL_CHANNELS, IDC_LEFTVU
void XMSamplerMixerPage::UpdateAllChannels(void)
{
	m_UpdatingGraphics=true;
	for (int i=0;i<8;i++)
	{
		UpdateChannel(i);
	}
	UpdateMaster();
	m_UpdatingGraphics=false;
}
// Refreshes the values of the controls of a specific channel.
void XMSamplerMixerPage::UpdateChannel(int index)
{
	XMSampler::Channel &rChan = sampler->rChannel(index+m_ChannelOffset);
	XMSampler::Voice *voice = rChan.ForegroundVoice();
	char chname[32];
	CStatic* name = (CStatic*)GetDlgItem(dlgName[index]);
	if ( ((CButton*)GetDlgItem(IDC_R_SHOWCHAN))->GetCheck()) 
	{
		sprintf(chname,"%d",index+m_ChannelOffset);
		name->SetWindowText(chname);
		CSliderCtrl* sld = (CSliderCtrl*)GetDlgItem(dlgVol[index]);
		sld->SetPos(volumeRange-(rChan.DefaultVolumeFloat()*volumeRange));
		CButton* mute = (CButton*)GetDlgItem(dlgMute[index]);
		if ( rChan.DefaultIsMute() )
		{
			mute->SetCheck(true);
		}
		else
		{
			mute->SetCheck(false);
		}
		sld = (CSliderCtrl*)GetDlgItem(dlgPan[index]);
		int defpos = rChan.DefaultPanFactorFloat()*panningRange;
		sld->SetPos(defpos);
		CButton* surr = (CButton*)GetDlgItem(dlgSurr[index]);
		if ( rChan.DefaultIsSurround())
		{
			surr->SetCheck(true);
		} else {
			surr->SetCheck(false);
		}

		sld = (CSliderCtrl*)GetDlgItem(dlgRes[index]);
		sld->SetPos(rChan.DefaultRessonance());
		sld = (CSliderCtrl*)GetDlgItem(dlgCut[index]);
		sld->SetPos(rChan.DefaultCutoff());
	}
	else 
	{
		CSliderCtrl* sld = (CSliderCtrl*)GetDlgItem(dlgVol[index]);
		int defpos;
		if ( !voice )
		{
			sprintf(chname,"(%d)",index+m_ChannelOffset);
			name->SetWindowText(chname);
			sld->SetPos(volumeRange);
			defpos = int(rChan.PanFactor()*panningRange);
			sld = (CSliderCtrl*)GetDlgItem(dlgRes[index]);
			sld->SetPos(rChan.Ressonance());
			sld = (CSliderCtrl*)GetDlgItem(dlgCut[index]);
			sld->SetPos(rChan.Cutoff());
		} else {
			std::string tmpstr = voice->rInstrument().Name();
			sprintf(chname,"%02X:%s",voice->InstrumentNum(),tmpstr.c_str());
			name->SetWindowText(chname);
			sld->SetPos(volumeRange-(voice->RealVolume()*volumeRange));
			defpos = int(voice->PanFactor()*panningRange);
			sld = (CSliderCtrl*)GetDlgItem(dlgRes[index]);
			sld->SetPos(voice->Ressonance());
			sld = (CSliderCtrl*)GetDlgItem(dlgCut[index]);
			sld->SetPos(voice->CutOff()); 
		}

		CButton* surr = (CButton*)GetDlgItem(dlgSurr[index]);
		if ( rChan.IsSurround() )
		{
			surr->SetCheck(true);
		} else {
			sld = (CSliderCtrl*)GetDlgItem(dlgPan[index]);
			sld->SetPos(defpos);
			surr->SetCheck(false);
		}
	}
}

// Refreshes the values of the controls of the master channel.
void XMSamplerMixerPage::UpdateMaster(void)
{
	char chvoices[4];
	CSliderCtrl*sld = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL_MASTER);
	sld->SetPos(128-sampler->GlobalVolume());
	CProgressCtrl* pctrl= (CProgressCtrl*)GetDlgItem(IDC_LEFT_VU);
	pctrl->SetPos(sampler->_volumeDisplay);
	CStatic* voices = (CStatic*)GetDlgItem(IDC_VOICESPLAYING);
	sprintf(chvoices,"%d",sampler->GetPlayingVoices());
	voices->SetWindowText(chvoices);
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff1(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,0); }
void XMSamplerMixerPage::OnNMCustomdrawSlCutoff2(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,1); }
void XMSamplerMixerPage::OnNMCustomdrawSlCutoff3(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,2); }
void XMSamplerMixerPage::OnNMCustomdrawSlCutoff4(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,3); }
void XMSamplerMixerPage::OnNMCustomdrawSlCutoff5(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,4); }
void XMSamplerMixerPage::OnNMCustomdrawSlCutoff6(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,5); }
void XMSamplerMixerPage::OnNMCustomdrawSlCutoff7(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,6); }
void XMSamplerMixerPage::OnNMCustomdrawSlCutoff8(NMHDR *pNMHDR, LRESULT *pResult) { SliderCutoff(pNMHDR,pResult,7); }
void XMSamplerMixerPage::SliderCutoff(NMHDR *pNMHDR, LRESULT *pResult,int offset)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	if ( !m_UpdatingGraphics)
	{
		XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
		CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(dlgCut[offset]);
		if (((CButton*)GetDlgItem(IDC_R_SHOWCHAN))->GetCheck())
		{
			rChan.DefaultCutoff(slid->GetPos());
		}
		else rChan.Cutoff(slid->GetPos());
	}
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes1(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,0); }
void XMSamplerMixerPage::OnNMCustomdrawSlRes2(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,1); }
void XMSamplerMixerPage::OnNMCustomdrawSlRes3(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,2); }
void XMSamplerMixerPage::OnNMCustomdrawSlRes4(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,3); }
void XMSamplerMixerPage::OnNMCustomdrawSlRes5(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,4); }
void XMSamplerMixerPage::OnNMCustomdrawSlRes6(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,5); }
void XMSamplerMixerPage::OnNMCustomdrawSlRes7(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,6); }
void XMSamplerMixerPage::OnNMCustomdrawSlRes8(NMHDR *pNMHDR, LRESULT *pResult) { SliderRessonance(pNMHDR,pResult,7); }
void XMSamplerMixerPage::SliderRessonance(NMHDR *pNMHDR, LRESULT *pResult,int offset)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	if ( !m_UpdatingGraphics)
	{
		XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
		CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(dlgRes[offset]);
		if (((CButton*)GetDlgItem(IDC_R_SHOWCHAN))->GetCheck())
		{
			rChan.DefaultRessonance(slid->GetPos());
		}
		else rChan.Ressonance(slid->GetPos());
	}
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan1(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,0); }
void XMSamplerMixerPage::OnNMCustomdrawSlPan2(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,1); }
void XMSamplerMixerPage::OnNMCustomdrawSlPan3(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,2); }
void XMSamplerMixerPage::OnNMCustomdrawSlPan4(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,3); }
void XMSamplerMixerPage::OnNMCustomdrawSlPan5(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,4); }
void XMSamplerMixerPage::OnNMCustomdrawSlPan6(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,5); }
void XMSamplerMixerPage::OnNMCustomdrawSlPan7(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,6); }
void XMSamplerMixerPage::OnNMCustomdrawSlPan8(NMHDR *pNMHDR, LRESULT *pResult) { SliderPanning(pNMHDR,pResult,7); }
void XMSamplerMixerPage::SliderPanning(NMHDR *pNMHDR, LRESULT *pResult, int offset)
{
	if ( !m_UpdatingGraphics)
	{
		if (((CButton*)GetDlgItem(IDC_R_SHOWCHAN))->GetCheck())
		{
			CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(dlgPan[offset]);
			XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
			rChan.DefaultPanFactorFloat(slid->GetPos()/(float)panningRange);
			*pResult = 0;
		}
	}
}

void XMSamplerMixerPage::OnBnClickedChSurr1() { ClickSurround(0); }
void XMSamplerMixerPage::OnBnClickedChSurr2() { ClickSurround(1); }
void XMSamplerMixerPage::OnBnClickedChSurr3() { ClickSurround(2); }
void XMSamplerMixerPage::OnBnClickedChSurr4() { ClickSurround(3); }
void XMSamplerMixerPage::OnBnClickedChSurr5() { ClickSurround(4); }
void XMSamplerMixerPage::OnBnClickedChSurr6() { ClickSurround(5); }
void XMSamplerMixerPage::OnBnClickedChSurr7() { ClickSurround(6); }
void XMSamplerMixerPage::OnBnClickedChSurr8() { ClickSurround(7); }
void XMSamplerMixerPage::ClickSurround(int offset)
{
	if ( !m_UpdatingGraphics)
	{
		CButton* surr = (CButton*)GetDlgItem(dlgSurr[offset]);
		XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
		if ( surr->GetCheck() == 0 ) {
			rChan.DefaultIsSurround(false);
		}
		else {
			rChan.DefaultIsSurround(true);
		}
	}
}

void XMSamplerMixerPage::OnBnClickedChMute1() { ClickMute(0); }
void XMSamplerMixerPage::OnBnClickedChMute2() { ClickMute(1); }
void XMSamplerMixerPage::OnBnClickedChMute3() { ClickMute(2); }
void XMSamplerMixerPage::OnBnClickedChMute4() { ClickMute(3); }
void XMSamplerMixerPage::OnBnClickedChMute5() { ClickMute(4); }
void XMSamplerMixerPage::OnBnClickedChMute6() { ClickMute(5); }
void XMSamplerMixerPage::OnBnClickedChMute7() { ClickMute(6); }
void XMSamplerMixerPage::OnBnClickedChMute8() { ClickMute(7); }
void XMSamplerMixerPage::ClickMute(int offset)
{
	if ( !m_UpdatingGraphics)
	{
		CButton* mute = (CButton*)GetDlgItem(dlgMute[offset]);
		XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
		if ( mute->GetCheck() == 0 ) {
			rChan.DefaultIsMute(false);
		}
		else {
			rChan.DefaultIsMute(true);
		}
	}
}
void XMSamplerMixerPage::OnNMCustomdrawSlVol1(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,0); }
void XMSamplerMixerPage::OnNMCustomdrawSlVol2(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,1); }
void XMSamplerMixerPage::OnNMCustomdrawSlVol3(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,2); }
void XMSamplerMixerPage::OnNMCustomdrawSlVol4(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,3); }
void XMSamplerMixerPage::OnNMCustomdrawSlVol5(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,4); }
void XMSamplerMixerPage::OnNMCustomdrawSlVol6(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,5); }
void XMSamplerMixerPage::OnNMCustomdrawSlVol7(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,6); }
void XMSamplerMixerPage::OnNMCustomdrawSlVol8(NMHDR *pNMHDR, LRESULT *pResult) { SliderVolume(pNMHDR,pResult,7); }
void XMSamplerMixerPage::SliderVolume(NMHDR *pNMHDR, LRESULT *pResult, int offset)
{
	if ( !m_UpdatingGraphics)
	{
		if (((CButton*)GetDlgItem(IDC_R_SHOWCHAN))->GetCheck())
		{
			CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(dlgVol[offset]);
			XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
			rChan.DefaultVolumeFloat((volumeRange-slid->GetPos())/(float)volumeRange);
		}
	}
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVolMaster(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	if ( !m_UpdatingGraphics)
	{
		CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL_MASTER);
		sampler->GlobalVolume(128-slid->GetPos());
	}
	*pResult = 0;
}
void XMSamplerMixerPage::OnNMCustomdrawSlChannels(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_CHANNELS);
	m_ChannelOffset = slid->GetPos();
	*pResult = 0;
}
PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
