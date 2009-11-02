#pragma once
#include "XMSamplerUIGeneral.hpp"
#include "XMSamplerUIInst.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSamplerMixerPage.hpp"
#include "Constants.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog
class XMSampler;

class XMSamplerUI : public CPropertySheet
	{
	DECLARE_DYNAMIC(XMSamplerUI)

	public:
		XMSamplerUI(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		XMSamplerUI(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

				enum { IDD = IDD_XM_SAMPLER };

	private:
		XMSampler* _pMachine;
		XMSamplerUIGeneral m_General;
		XMSamplerUIInst m_Instrument;
		XMSamplerUISample m_Sample;
		XMSamplerMixerPage m_Mixer;
		bool init;

	public:
		void Init(XMSampler* pMachine);
		void UpdateUI(void);
		XMSampler* GetMachine(){ return _pMachine; }

	protected:
		afx_msg void OnDestroy();

		DECLARE_MESSAGE_MAP()
	};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
