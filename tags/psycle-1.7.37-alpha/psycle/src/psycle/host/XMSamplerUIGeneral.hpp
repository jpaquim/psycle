#pragma once
#include "constants.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUIGeneral dialog
class XMSampler;
class XMSamplerUIGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(XMSamplerUIGeneral)
// Construction
public:
	XMSamplerUIGeneral();
	~XMSamplerUIGeneral();

	XMSampler * const pMachine() { return _pMachine;};
	void pMachine(XMSampler * const p) { _pMachine = p;};

	// Dialog Data
	//{{AFX_DATA(XMSamplerUIGeneral)
	enum { IDD = IDD_XM_GENERAL };
	CEdit	m_Tempo;
	CEdit	m_Speed;
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
	afx_msg void OnEnChangeXmspeed();
	afx_msg void OnEnChangeXmtempo();
	afx_msg void OnNMCustomdrawXmpoly(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	// Generated message map functions
	//{{AFX_MSG(XMSaplerUIGeneral)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

NAMESPACE__END
NAMESPACE__END