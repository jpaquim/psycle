#pragma once

#include "constants.hpp"       // ***** [bohan] iso-(10)646 encoding only please! *****

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

class XMSampler;
class XMSamplerUISample : public CPropertyPage
{
public:
	DECLARE_DYNCREATE(XMSamplerUISample)
	// Construction
public:
	XMSamplerUISample();
	~XMSamplerUISample();

	// Dialog Data
	//{{AFX_DATA(XMSamplerUISample)
	enum { IDD = IDD_XMSAMPLERUISAMPLE };
	//}}AFX_DATA

	// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(XMSamplerUISample)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(XMSaplerUIGeneral)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void pMachine(XMSampler * const p){m_pMachine = p;};
	XMSampler * const pMachine(){return m_pMachine;};

private:
	XMSampler *m_pMachine;

};

NAMESPACE__END
NAMESPACE__END