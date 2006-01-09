#pragma once
#include "constants.hpp"
#include "XMSamplerUIGeneral.hpp"
#include "XMSamplerUIInst.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSamplerMixerPage.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)
/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog
class XMSampler;

class XMSamplerUI : public CPropertySheet
	{
	DECLARE_DYNAMIC(XMSamplerUI)
	public:
		XMSamplerUI(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		XMSamplerUI(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

		// Datos del cuadro de diálogo
		enum { IDD = IDD_XM_SAMPLER };

		// Attributes
	private:
		XMSampler* _pMachine;
		XMSamplerUIGeneral m_General;
		XMSamplerUIInst m_Instrument;
		XMSamplerUISample m_Sample;
		XMSamplerMixerPage m_Mixer;
		bool init;
		// Operations
	public:
		void Init(XMSampler* pMachine);
		void UpdateUI(void);
		XMSampler* GetMachine(){ return _pMachine; }

		// Implementation
		// Generated message map functions
	protected:
		//{{AFX_MSG(XMSamplerUI)
		afx_msg void OnDestroy();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
NAMESPACE__END
NAMESPACE__END