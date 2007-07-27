#pragma once
#include "constants.hpp"
#include "resources/resources.hpp"
#include <afxwin.h>
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

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

	// Dialog Data
	//{{AFX_DATA(XMSamplerUIGeneral)
	enum { IDD = IDD_XM_GENERAL };
	CComboBox	m_interpol;
	CSliderCtrl	m_polyslider;
	CStatic m_polylabel;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(XMSamplerUIGeneral)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:
	XMSampler* _pMachine;
	bool m_bInitialize;
//	CEdit m_GlobalVolume;
public:
	afx_msg void OnCbnSelchangeXminterpol();
	afx_msg void OnNMCustomdrawXmpoly(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	// Generated message map functions
	//{{AFX_MSG(XMSaplerUIGeneral)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_ECommandInfo;
	CButton m_bAmigaSlides;
	CButton m_ckFilter;
	CComboBox m_cbPanningMode;

	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnCbnSelendokXmpanningmode();
};

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
