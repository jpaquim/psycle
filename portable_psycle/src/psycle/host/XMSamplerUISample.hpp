#pragma once

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
	afx_msg BOOL OnSetActive(void);

private:

	XMSampler *m_pMachine;
public:
	void pMachine(XMSampler *const p){m_pMachine = p;};
	XMSampler * const pMachine(){return m_pMachine;};
	afx_msg void OnLbnSelchangeSamplelist();
};
NAMESPACE__END
NAMESPACE__END