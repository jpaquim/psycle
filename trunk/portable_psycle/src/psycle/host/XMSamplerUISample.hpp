#pragma once
#include "afxwin.h"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

class XMSampler;

class XMSamplerUISample : public CPropertyPage
{
	DECLARE_DYNAMIC(XMSamplerUISample)

public:
	XMSamplerUISample();
	virtual ~XMSamplerUISample();

	// Datos del cuadro de diálogo
	enum { IDD = IDD_XMSAMPLERUISAMPLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV

	DECLARE_MESSAGE_MAP()

public:
	void pMachine(XMSampler *const p){m_pMachine = p;};
	XMSampler * const pMachine(){return m_pMachine;};

	afx_msg BOOL OnSetActive(void);
	afx_msg void OnLbnSelchangeSamplelist();
	afx_msg void OnNMCustomdrawDefvolume(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawGlobvolume(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawPan(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelendokVibratotype();
	afx_msg void OnNMCustomdrawVibratorate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawVibratospeed(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawVibratodepth(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelendokLoop();
	afx_msg void OnCbnSelendokSustainloop();
	afx_msg void OnEnChangeLoopstart();
	afx_msg void OnEnChangeLoopend();
	afx_msg void OnEnChangeSustainstart();
	afx_msg void OnEnChangeSustainend();
	afx_msg void OnEnChangeWavename();
	afx_msg void OnEnChangeSamplerate();
	afx_msg void OnDeltaposSpinsamplerate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSamplenote(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawFinetune(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOpenwaveeditor();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedDupe();
	afx_msg void OnBnClickedDelete();

protected:
	XMSampler *m_pMachine;
	bool m_Init;

	CListBox m_SampleList;

};
NAMESPACE__END
NAMESPACE__END