// XMSamplerMixerPage.cpp: archivo de implementación

#include <psycle/host/detail/project.private.hpp>
#include "XMSamplerMixerPage.hpp"

#include "Song.hpp"
#include "XMSampler.hpp"
#include "XMSamplerUI.hpp"

namespace psycle { namespace host {

// Cuadro de diálogo de XMSamplerMixerPage

const int XMSamplerMixerPage::dlgCut[8] = {
	IDC_SL_CUTOFF1,IDC_SL_CUTOFF2,IDC_SL_CUTOFF3,IDC_SL_CUTOFF4,
	IDC_SL_CUTOFF5,IDC_SL_CUTOFF6,IDC_SL_CUTOFF7,IDC_SL_CUTOFF8
};
const int XMSamplerMixerPage::dlgRes[8] = {
	IDC_SL_RES1,IDC_SL_RES2,IDC_SL_RES3,IDC_SL_RES4,
	IDC_SL_RES5,IDC_SL_RES6,IDC_SL_RES7,IDC_SL_RES8
};
const int XMSamplerMixerPage::dlgPan[8] = {
	IDC_SL_PAN1,IDC_SL_PAN2,IDC_SL_PAN3,IDC_SL_PAN4,
	IDC_SL_PAN5,IDC_SL_PAN6,IDC_SL_PAN7,IDC_SL_PAN8
};
const int XMSamplerMixerPage::dlgSurr[8] = {
	IDC_CH_SURR1,IDC_CH_SURR2,IDC_CH_SURR3,IDC_CH_SURR4,
	IDC_CH_SURR5,IDC_CH_SURR6,IDC_CH_SURR7,IDC_CH_SURR8
};
const int XMSamplerMixerPage::dlgMute[8] = {
	IDC_CH_MUTE1,IDC_CH_MUTE2,IDC_CH_MUTE3,IDC_CH_MUTE4,
	IDC_CH_MUTE5,IDC_CH_MUTE6,IDC_CH_MUTE7,IDC_CH_MUTE8
};
const int XMSamplerMixerPage::dlgVol[8] = {
	IDC_SL_VOL1,IDC_SL_VOL2,IDC_SL_VOL3,IDC_SL_VOL4,
	IDC_SL_VOL5,IDC_SL_VOL6,IDC_SL_VOL7,IDC_SL_VOL8
};
const int XMSamplerMixerPage::dlgName[8] = {
	IDC_NAME1,IDC_NAME2,IDC_NAME3,IDC_NAME4,
	IDC_NAME5,IDC_NAME6,IDC_NAME7,IDC_NAME8
};

int const XMSamplerMixerPage::resRange = 127;
int const XMSamplerMixerPage::cutoffRange = 127;
int const XMSamplerMixerPage::panningRange = 200;
int const XMSamplerMixerPage::volumeRange = 200;


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
	DDX_Control(pDX, IDC_SL_CHANNELS, m_slChannels);
	DDX_Control(pDX, IDC_SL_VOL_MASTER, m_slMaster);
	DDX_Control(pDX, IDC_LEFT_VU, m_vu);
	DDX_Control(pDX, IDC_VOICESPLAYING, m_voicesPl);
	DDX_Control(pDX, IDC_R_SHOWCHAN, m_bShowChan);
	DDX_Control(pDX, IDC_R_SHOWVOICE, m_bShowPlay);
}


BEGIN_MESSAGE_MAP(XMSamplerMixerPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
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
END_MESSAGE_MAP()

BOOL XMSamplerMixerPage::PreTranslateMessage(MSG* pMsg) 
{
	XMSamplerUI* parent = dynamic_cast<XMSamplerUI*>(GetParent());
	BOOL res = parent->PreTranslateChildMessage(pMsg, GetFocus()->GetSafeHwnd());
	if (res == FALSE ) return CPropertyPage::PreTranslateMessage(pMsg);
	return res;
}

// Controladores de mensajes de XMSamplerMixerPage
BOOL XMSamplerMixerPage::OnSetActive()
{
	m_UpdatingGraphics=true;
	m_slChannels.SetRangeMax((Global::song().SongTracks()>8)?(Global::song().SongTracks()-8):0); // maxchans-visiblechans
	if ( m_bShowPlay.GetCheck() == 0 ) { m_bShowChan.SetCheck(1); }
	for (int i=0;i<8;i++)
	{
		((CSliderCtrl*)GetDlgItem(dlgVol[i]))->SetRangeMax(volumeRange);
		((CSliderCtrl*)GetDlgItem(dlgPan[i]))->SetRangeMax(panningRange);
		((CSliderCtrl*)GetDlgItem(dlgRes[i]))->SetRangeMax(resRange);
		((CSliderCtrl*)GetDlgItem(dlgCut[i]))->SetRangeMax(cutoffRange);
		UpdateChannel(i);
	}
	m_slMaster.SetRangeMax(128);
	m_vu.SetRange(0,97);
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
	if (m_bShowChan.GetCheck()) 
	{
		sprintf(chname,"%d",index+m_ChannelOffset);
		name->SetWindowText(chname);
		CSliderCtrl* sld = (CSliderCtrl*)GetDlgItem(dlgVol[index]);
		sld->SetPos(volumeRange-(rChan.DefaultVolumeFloat()*volumeRange));
		CButton* mute = (CButton*)GetDlgItem(dlgMute[index]);
		mute->SetCheck(rChan.DefaultIsMute());

		sld = (CSliderCtrl*)GetDlgItem(dlgPan[index]);
		int defpos = rChan.DefaultPanFactorFloat()*panningRange;
		sld->SetPos(defpos);
		CButton* surr = (CButton*)GetDlgItem(dlgSurr[index]);
		surr->SetCheck(rChan.DefaultIsSurround());

		sld = (CSliderCtrl*)GetDlgItem(dlgRes[index]);
		sld->SetPos(rChan.DefaultRessonance());
		sld = (CSliderCtrl*)GetDlgItem(dlgCut[index]);
		sld->SetPos(rChan.DefaultCutoff());
	}
	else 
	{
		CSliderCtrl* sld = (CSliderCtrl*)GetDlgItem(dlgVol[index]);
		int defpos;
		bool surround;
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
			surround = rChan.IsSurround();
		} else {
			std::string tmpstr = voice->rInstrument().Name();
			sprintf(chname,"%02X:%s",voice->InstrumentNum(),tmpstr.c_str());
			name->SetWindowText(chname);
			sld->SetPos(volumeRange-(voice->ActiveVolume()*volumeRange));
			defpos = int(voice->ActivePan()*panningRange);
			sld = (CSliderCtrl*)GetDlgItem(dlgRes[index]);
			sld->SetPos(voice->ActiveRessonance());
			sld = (CSliderCtrl*)GetDlgItem(dlgCut[index]);
			sld->SetPos(voice->ActiveCutoff()); 
			surround =  voice->IsSurround();
		}

		CButton* surr = (CButton*)GetDlgItem(dlgSurr[index]);
		if ( surround )	{
			surr->SetCheck(true);
			defpos=0.5f*panningRange;
		} else {
			surr->SetCheck(false);
		}
		sld = (CSliderCtrl*)GetDlgItem(dlgPan[index]);
		sld->SetPos(defpos);
	}
}

// Refreshes the values of the controls of the master channel.
void XMSamplerMixerPage::UpdateMaster(void)
{
	char chvoices[4];
	m_slMaster.SetPos(128-sampler->GlobalVolume());
	m_vu.SetPos(sampler->_volumeDisplay);
	sprintf(chvoices,"%d",sampler->GetPlayingVoices());
	m_voicesPl.SetWindowText(chvoices);
}

void XMSamplerMixerPage::SliderCutoff(CSliderCtrl* slid, int offset)
{
	XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
	if (m_bShowChan.GetCheck())
	{
		rChan.DefaultCutoff(slid->GetPos());
	}
	else rChan.Cutoff(slid->GetPos());
}
void XMSamplerMixerPage::SliderRessonance(CSliderCtrl* slid, int offset)
{
	XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
	if (m_bShowChan.GetCheck())
	{
		rChan.DefaultRessonance(slid->GetPos());
	}
	else rChan.Ressonance(slid->GetPos());
}
void XMSamplerMixerPage::SliderPanning(CSliderCtrl* slid, int offset)
{
	XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
	if (m_bShowChan.GetCheck())
	{
		rChan.DefaultPanFactorFloat(slid->GetPos()/(float)panningRange);
	}
	else {
		rChan.PanFactor(slid->GetPos()/(float)panningRange);
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
		if (m_bShowChan.GetCheck())
		{
			rChan.DefaultIsSurround(surr->GetCheck());
		}
		else {
			rChan.IsSurround(surr->GetCheck());
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
		rChan.DefaultIsMute(mute->GetCheck());
	}
}
void XMSamplerMixerPage::SliderVolume(CSliderCtrl* slid, int offset)
{
	XMSampler::Channel &rChan = sampler->rChannel(offset+m_ChannelOffset);
	if (m_bShowChan.GetCheck())
	{
		rChan.DefaultVolumeFloat((volumeRange-slid->GetPos())/(float)volumeRange);
	} else if(rChan.ForegroundVoice()) {
		rChan.Volume((volumeRange-slid->GetPos())/(float)volumeRange);
	}
}


void XMSamplerMixerPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	CSliderCtrl* the_slider = reinterpret_cast<CSliderCtrl*>(pScrollBar);

	switch(nSBCode){
	case TB_BOTTOM: //fallthrough
	case TB_LINEDOWN: //fallthrough
	case TB_PAGEDOWN: //fallthrough
	case TB_TOP: //fallthrough
	case TB_LINEUP: //fallthrough
	case TB_PAGEUP: //fallthrough
	case TB_THUMBPOSITION: //fallthrough
	case TB_THUMBTRACK:
		if ( the_slider->GetDlgCtrlID() == m_slChannels.GetDlgCtrlID() ) {
			m_ChannelOffset = m_slChannels.GetPos();
		}
		else {
			int uId = the_slider->GetDlgCtrlID();
			for (int i=0;i<8;i++)
			{
				if(uId == dlgCut[i]) { SliderCutoff(the_slider, i); break; }
				if(uId == dlgRes[i]) { SliderRessonance(the_slider, i); break; }
				if(uId == dlgPan[i]) { SliderPanning(the_slider, i); break; }
			}
		}
		break;
	case TB_ENDTRACK:
		break;
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
void XMSamplerMixerPage::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
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
		if ( uId == m_slMaster.GetDlgCtrlID()) {
			sampler->GlobalVolume(128-m_slMaster.GetPos());
		}
		else {
			for (int i=0;i<8;i++)
			{
				if(uId == dlgVol[i]) { SliderVolume(the_slider, i); break; }
			}
		}
		break;
	case TB_ENDTRACK:
		break;
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


}}
