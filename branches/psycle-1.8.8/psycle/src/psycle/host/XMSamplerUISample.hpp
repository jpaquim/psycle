#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "XMInstrument.hpp"

namespace psycle { namespace host {

class XMSampler;
class CWaveScopeCtrl : public CStatic
{
public:
	CWaveScopeCtrl();
	virtual ~CWaveScopeCtrl();

	void SetWave(XMInstrument::WaveData *pWave) { m_pWave = pWave; }
	XMInstrument::WaveData& rWave() { return *m_pWave; }

	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

protected:
	XMInstrument::WaveData* m_pWave;
	CPen cpen_lo;
	CPen cpen_med;
	CPen cpen_hi;
	CPen cpen_sus;

};




class XMSamplerUISample : public CPropertyPage
{
	DECLARE_DYNAMIC(XMSamplerUISample)

public:
	XMSamplerUISample();
	virtual ~XMSamplerUISample();

	// Datos del cuadro de di�logo
	enum { IDD = IDD_XMSAMPLERUISAMPLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV


public:
	void pMachine(XMSampler *const p){m_pMachine = p;};
	XMSampler * const pMachine(){return m_pMachine;};
	void pWave(XMInstrument::WaveData *const p){m_pWave = p;};
	XMInstrument::WaveData& rWave(){return *m_pWave;};
	void DrawScope(void);

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnSetActive(void);
	afx_msg void OnLbnSelchangeSamplelist();
	afx_msg void OnNMCustomdrawDefvolume(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawGlobvolume(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawPan(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelendokVibratotype();
	afx_msg void OnNMCustomdrawVibratoAttack(NMHDR *pNMHDR, LRESULT *pResult);
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
	afx_msg void OnBnClickedPanenabled();

protected:
	XMSampler *m_pMachine;
	XMInstrument::WaveData *m_pWave;
	bool m_Init;

protected:
	CListBox m_SampleList;
	CWaveScopeCtrl m_WaveScope;
};

}}
