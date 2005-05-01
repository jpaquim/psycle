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
		void Init(Configuration* pConfig);
		void UpdateUI(void);
		XMSampler* GetMachine(){ return _pMachine; }
		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CConfigDlg)
	public:
		virtual int DoModal();
		//}}AFX_VIRTUAL
		// Implementation
	public:
		virtual ~XMSamplerUI();
		void Init(XMSampler* pMachine);
		// Generated message map functions
	protected:
		//{{AFX_MSG(CConfigDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
NAMESPACE__END
NAMESPACE__END