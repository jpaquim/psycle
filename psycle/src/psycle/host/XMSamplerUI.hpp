#pragma once
#include "Psycle.hpp"
#include "XMSamplerUIGeneral.hpp"
#include "XMSamplerUIInst.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSamplerMixerPage.hpp"

namespace psycle {
	namespace core {
		class XMSampler;
	}
	namespace host {
		using namespace core;

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog

class XMSamplerUI : public CPropertySheet
	{
	DECLARE_DYNAMIC(XMSamplerUI)

	public:
		XMSamplerUI(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		XMSamplerUI(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		XMSamplerUI(LPCTSTR pszCaption, class MachineGui* gui, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

		enum { IDD = IDD_XM_SAMPLER };

	private:
		XMSampler* _pMachine;
		XMSamplerUIGeneral m_General;
		XMSamplerUIInst m_Instrument;
		XMSamplerUISample m_Sample;
		XMSamplerMixerPage m_Mixer;
		bool init;
		MachineGui* gui_;

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

}   // namespace
}   // namespace
