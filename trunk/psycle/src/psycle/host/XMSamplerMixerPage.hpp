#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

// Cuadro de diálogo de XMSamplerMixerPage

class XMSampler;

class XMSamplerMixerPage : public CPropertyPage
{
	DECLARE_DYNAMIC(XMSamplerMixerPage)

public:
	XMSamplerMixerPage();
	virtual ~XMSamplerMixerPage();

// Datos del cuadro de diálogo
	enum { IDD = IDD_XMSAMPLER_MIXER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetActive(void);
	afx_msg void OnNMCustomdrawSlCutoff1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlCutoff2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlCutoff3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlCutoff4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlCutoff5(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlCutoff6(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlCutoff7(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlCutoff8(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes5(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes6(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes7(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlRes8(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan5(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan6(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan7(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlPan8(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedChSurr1();
	afx_msg void OnBnClickedChSurr2();
	afx_msg void OnBnClickedChSurr3();
	afx_msg void OnBnClickedChSurr4();
	afx_msg void OnBnClickedChSurr5();
	afx_msg void OnBnClickedChSurr6();
	afx_msg void OnBnClickedChSurr7();
	afx_msg void OnBnClickedChSurr8();
	afx_msg void OnBnClickedChMute1();
	afx_msg void OnBnClickedChMute2();
	afx_msg void OnBnClickedChMute3();
	afx_msg void OnBnClickedChMute4();
	afx_msg void OnBnClickedChMute5();
	afx_msg void OnBnClickedChMute6();
	afx_msg void OnBnClickedChMute7();
	afx_msg void OnBnClickedChMute8();
	afx_msg void OnNMCustomdrawSlVol1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVol2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVol3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVol4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVol5(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVol6(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVol7(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVol8(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlVolMaster(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlChannels(NMHDR *pNMHDR, LRESULT *pResult);
private:
	void SliderVolume(NMHDR *pNMHDR, LRESULT *pResult, int offset);
	void SliderPanning(NMHDR *pNMHDR, LRESULT *pResult, int offset);
	void SliderCutoff(NMHDR *pNMHDR, LRESULT *pResult, int offset);
	void SliderRessonance(NMHDR *pNMHDR, LRESULT *pResult, int offset);
	void ClickSurround(int offset);
	void ClickMute(int offset);

	static const int dlgName[8];
	static const int dlgVol[8];
	static const int dlgSurr[8];
	static const int dlgPan[8];
	static const int dlgRes[8];
	static const int dlgCut[8];
	static const int dlgMute[8];

	XMSampler *sampler;
	// Indicates the index of the first channel shown (controlled by IDC_SL_CHANNELS)
	int m_ChannelOffset;
	bool m_UpdatingGraphics;
public:
	// Refreshes the values of all the controls of the dialog, except IDC_SL_CHANNELS, IDC_LEFTVU and IDC_RIGHTVU
	void UpdateAllChannels(void);
	// Refreshes the values of the controls of a specific channel. channel 0 is the Master channel.
	void UpdateChannel(int index);
	// Refreshes the values of the controls of the master channel.
	void UpdateMaster(void);

	void pMachine(XMSampler* mac) { sampler = mac; }
private:
};

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
