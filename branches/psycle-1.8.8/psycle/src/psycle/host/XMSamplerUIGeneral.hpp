#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUIGeneral dialog
class XMSampler;
class XMSamplerUIGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(XMSamplerUIGeneral)
// Construction
public:
	XMSamplerUIGeneral();
	virtual ~XMSamplerUIGeneral();

	XMSampler * const pMachine() { return _pMachine;};
	void pMachine(XMSampler * const p) { _pMachine = p;};

	enum { IDD = IDD_XM_GENERAL };
	CComboBox	m_interpol;
	CSliderCtrl	m_polyslider;
	CStatic m_polylabel;
	CEdit m_ECommandInfo;
	CButton m_bAmigaSlides;
	CButton m_ckFilter;
	CComboBox m_cbPanningMode;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
private:
	XMSampler* _pMachine;
	bool m_bInitialize;
//	CEdit m_GlobalVolume;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelchangeXminterpol();
	afx_msg void OnNMCustomdrawXmpoly(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnCbnSelendokXmpanningmode();
};

}}
