// XMSamplerMixerPage.cpp: archivo de implementación
//

#include "project.private.hpp"
#include "Psycle.hpp"
#include "XMSamplerMixerPage.h"
#include "XMSampler.hpp"
#include ".\xmsamplermixerpage.h"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)
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


IMPLEMENT_DYNAMIC(XMSamplerMixerPage, CPropertyPage)
XMSamplerMixerPage::XMSamplerMixerPage()
	: CPropertyPage(XMSamplerMixerPage::IDD)
	, m_ChannelOffset(0)
	, m_ShowChan(FALSE)
{
}

XMSamplerMixerPage::~XMSamplerMixerPage()
{
}

void XMSamplerMixerPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_R_SHOWCHAN, m_ShowChan);
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
	ON_BN_CLICKED(IDC_CH_MASTERMONO, OnBnClickedChMastermono)
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
	ON_BN_CLICKED(IDC_CH_MUTE1, OnBnClickedChMute1)
	ON_BN_CLICKED(IDC_CH_MUTE2, OnBnClickedChMute2)
	ON_BN_CLICKED(IDC_CH_MUTE3, OnBnClickedChMute3)
	ON_BN_CLICKED(IDC_CH_MUTE4, OnBnClickedChMute4)
	ON_BN_CLICKED(IDC_CH_MUTE5, OnBnClickedChMute5)
	ON_BN_CLICKED(IDC_CH_MUTE6, OnBnClickedChMute6)
	ON_BN_CLICKED(IDC_CH_MUTE7, OnBnClickedChMute7)
	ON_BN_CLICKED(IDC_CH_MUTE8, OnBnClickedChMute8)
END_MESSAGE_MAP()


// Controladores de mensajes de XMSamplerMixerPage
BOOL XMSamplerMixerPage::OnSetActive()
{
	UpdateAllChannels();
	return CPropertyPage::OnSetActive();
}

// Refreshes the values of all the controls of the dialog, except IDC_SL_CHANNELS, IDC_LEFTVU and IDC_RIGHTVU
void XMSamplerMixerPage::UpdateAllChannels(void)
{
	for (int i=0;i<8;i++)
	{
		((CSliderCtrl*)GetDlgItem(dlgVol[i]))->SetRangeMax(64);
		((CSliderCtrl*)GetDlgItem(dlgPan[i]))->SetRangeMax(64);
		/* ((CSliderCtrl*)GetDlgItem(dlgRes[i]))->SetRangeMax(127);
		((CSliderCtrl*)GetDlgItem(dlgCut[i]))->SetRangeMax(127);
		*/
		UpdateChannel(i);
	}
	((CSliderCtrl*)GetDlgItem(IDC_SL_VOL_MASTER))->SetRangeMax(128);
	((CProgressCtrl*)GetDlgItem(IDC_LEFT_VU))->SetRange(0,97);
	UpdateMaster();
	((CSliderCtrl*)GetDlgItem(IDC_SL_CHANNELS))->SetRangeMax(56); // maxchans-visiblechans
}
// Refreshes the values of the controls of a specific channel.
void XMSamplerMixerPage::UpdateChannel(int index)
{
	char chname[8];
	if ( m_ShowChan ) 
	{
		CStatic* name = (CStatic*)GetDlgItem(dlgName[index]);
		sprintf(chname,"%d",index+m_ChannelOffset+1);
		name->SetWindowText(chname);
		CSliderCtrl* sld = (CSliderCtrl*)GetDlgItem(dlgVol[index]);

		sld->SetPos(64-int(sampler->rChannel(index+m_ChannelOffset).Volume()*64.0f));
		sld = (CSliderCtrl*)GetDlgItem(dlgPan[index]);
		int defpos = int(sampler->rChannel(index+m_ChannelOffset).PanFactor()*64.0f);
		CButton* surr = (CButton*)GetDlgItem(dlgSurr[index]);
		if ( sampler->rChannel(index+m_ChannelOffset).IsSurround() )
		{
			surr->SetCheck(true);
		} else {
			sld->SetPos(defpos);
			surr->SetCheck(false);
		}

		/*	sld = (CSliderCtrl*)GetDlgItem(dlgRes[index]);
		sld->SetPos(int(sampler->rChannel(index+m_ChannelOffset).DefaultRessonance()*64.0f));
		sld = (CSliderCtrl*)GetDlgItem(dlgCut[index]);
		sld->SetPos(int(sampler->rChannel(index+m_ChannelOffset).DefaultCuttof()*64.0f));
		*/
	}
	else
	{
		CStatic* name = (CStatic*)GetDlgItem(dlgName[index]);
		sprintf(chname,"%d",index+m_ChannelOffset+1);
		name->SetWindowText(chname);
		CSliderCtrl* sld = (CSliderCtrl*)GetDlgItem(dlgVol[index]);

		sld->SetPos(64-int(sampler->rChannel(index+m_ChannelOffset).Volume()*64.0f));
		sld = (CSliderCtrl*)GetDlgItem(dlgPan[index]);
		int defpos = int(sampler->rChannel(index+m_ChannelOffset).PanFactor()*64.0f);
		CButton* surr = (CButton*)GetDlgItem(dlgSurr[index]);
		if ( sampler->rChannel(index+m_ChannelOffset).IsSurround() )
		{
			surr->SetCheck(true);
		} else {
			sld->SetPos(defpos);
			surr->SetCheck(false);
		}

		/*	sld = (CSliderCtrl*)GetDlgItem(dlgRes[index]);
		sld->SetPos(int(sampler->rChannel(index+m_ChannelOffset).DefaultRessonance()*64.0f));
		sld = (CSliderCtrl*)GetDlgItem(dlgCut[index]);
		sld->SetPos(int(sampler->rChannel(index+m_ChannelOffset).DefaultCuttof()*64.0f));
		*/

	}
}

// Refreshes the values of the controls of the master channel.
void XMSamplerMixerPage::UpdateMaster(void)
{
	char chvoices[4];
//	CSliderCtrl*sld = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL_MASTER);
//	sld->SetPos(sampler->rChannel(index+m_ChannelOffset).DefaultVolume());
	CProgressCtrl* pctrl= (CProgressCtrl*)GetDlgItem(IDC_LEFT_VU);
	pctrl->SetPos(sampler->_volumeDisplay);
	CStatic* voices = (CStatic*)GetDlgItem(IDC_VOICESPLAYING);
	sprintf(chvoices,"%d",sampler->GetPlayingVoices());
	voices->SetWindowText(chvoices);
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff4(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff5(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff6(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff7(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlCutoff8(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes4(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes5(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes6(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes7(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlRes8(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan1(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN1);
	sampler->rChannel(m_ChannelOffset).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan2(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN2);
	sampler->rChannel(m_ChannelOffset+1).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan3(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN3);
	sampler->rChannel(m_ChannelOffset+2).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan4(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN4);
	sampler->rChannel(m_ChannelOffset+3).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan5(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN5);
	sampler->rChannel(m_ChannelOffset+4).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan6(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN6);
	sampler->rChannel(m_ChannelOffset+5).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan7(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN7);
	sampler->rChannel(m_ChannelOffset+6).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlPan8(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_PAN8);
	sampler->rChannel(m_ChannelOffset+7).DefaultPanFactor(slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnBnClickedChSurr1()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR1);
	sampler->rChannel(m_ChannelOffset).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChSurr2()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR2);
	sampler->rChannel(m_ChannelOffset+1).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChSurr3()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR3);
	sampler->rChannel(m_ChannelOffset+2).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChSurr4()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR4);
	sampler->rChannel(m_ChannelOffset+3).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChSurr5()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR5);
	sampler->rChannel(m_ChannelOffset+4).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChSurr6()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR6);
	sampler->rChannel(m_ChannelOffset+5).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChSurr7()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR7);
	sampler->rChannel(m_ChannelOffset+6).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChSurr8()
{
	CButton* surr = (CButton*)GetDlgItem(IDC_CH_SURR8);
	sampler->rChannel(m_ChannelOffset+7).IsSurround(surr->GetCheck()?true:false);
}

void XMSamplerMixerPage::OnBnClickedChMute1()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMute2()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMute3()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMute4()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMute5()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMute6()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMute7()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMute8()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnBnClickedChMastermono()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol1(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL1);
	sampler->rChannel(m_ChannelOffset).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol2(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL2);
	sampler->rChannel(m_ChannelOffset+1).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol3(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL3);
	sampler->rChannel(m_ChannelOffset+2).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol4(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL4);
	sampler->rChannel(m_ChannelOffset+3).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol5(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL5);
	sampler->rChannel(m_ChannelOffset+4).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol6(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL6);
	sampler->rChannel(m_ChannelOffset+5).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol7(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL7);
	sampler->rChannel(m_ChannelOffset+6).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVol8(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_VOL8);
	sampler->rChannel(m_ChannelOffset+7).DefaultVolume(64-slid->GetPos());
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlVolMaster(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Agregue aquí su código de controlador de notificación de control
	*pResult = 0;
}

void XMSamplerMixerPage::OnNMCustomdrawSlChannels(NMHDR *pNMHDR, LRESULT *pResult)
{
	CSliderCtrl* slid = (CSliderCtrl*)GetDlgItem(IDC_SL_CHANNELS);
	m_ChannelOffset = slid->GetPos();
	*pResult = 0;
}
NAMESPACE__END
NAMESPACE__END
