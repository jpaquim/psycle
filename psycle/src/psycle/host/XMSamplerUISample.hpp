#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "WaveScopeCtrl.hpp"
#include <psycle/host/XMInstrument.hpp>

namespace psycle { namespace host {

class XMSampler;

class XMSamplerUISample : public CPropertyPage
{
public:
	DECLARE_DYNAMIC(XMSamplerUISample)

public:
	XMSamplerUISample();
	virtual ~XMSamplerUISample();

	// Datos del cuadro de di�logo
	enum { IDD = IDD_XMSAMPLERUISAMPLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV
	virtual BOOL OnInitDialog();


public:
	void pMachine(XMSampler *const p){m_pMachine = p;};
	XMSampler * const pMachine(){return m_pMachine;};
	void pWave(XMInstrument::WaveData *const p){m_pWave = p;};
	XMInstrument::WaveData& rWave(){return *m_pWave;};
	void DrawScope(void);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnSetActive(void);
	afx_msg void OnLbnSelchangeSamplelist();
	afx_msg void OnCbnSelendokVibratotype();
	afx_msg void OnCbnSelendokLoop();
	afx_msg void OnCbnSelendokSustainloop();
	afx_msg void OnEnChangeLoopstart();
	afx_msg void OnEnChangeLoopend();
	afx_msg void OnEnChangeSustainstart();
	afx_msg void OnEnChangeSustainend();
	afx_msg void OnEnChangeWavename();
	afx_msg void OnEnChangeSamplerate();
	afx_msg void OnDeltaposSpinsamplerate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOpenwaveeditor();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedDupe();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedPanenabled();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCustomdrawSliderm(UINT idx, NMHDR* pNMHDR, LRESULT* pResult);
protected:

	void SliderDefvolume(CSliderCtrl* slid);
	void SliderGlobvolume(CSliderCtrl* slid);
	void SliderPan(CSliderCtrl* slid);
	void SliderVibratoAttack(CSliderCtrl* slid);
	void SliderVibratospeed(CSliderCtrl* slid);
	void SliderVibratodepth(CSliderCtrl* slid);
	void SliderSamplenote(CSliderCtrl* slid);
	void SliderFinetune(CSliderCtrl* slid);
	void FillPanDescription(int val);
	void RefreshSampleList();

	XMSampler *m_pMachine;
	XMInstrument::WaveData *m_pWave;
	bool m_Init;

protected:
	CListBox m_SampleList;
	CWaveScopeCtrl m_WaveScope;
};

}}
