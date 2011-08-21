#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "XMSamplerUIGeneral.hpp"
#include "XMSamplerUIInst.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSamplerMixerPage.hpp"

namespace psycle { namespace host {

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog
class XMSampler;

class XMSamplerUI : public CPropertySheet
{
	DECLARE_DYNAMIC(XMSamplerUI)

	public:
		XMSamplerUI(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		XMSamplerUI(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		virtual ~XMSamplerUI();

		enum { IDD = IDD_XM_SAMPLER };

	private:
		XMSampler* _pMachine;
		XMSamplerUI** windowVar_;
		XMSamplerUIGeneral m_General;
		XMSamplerUIInst m_Instrument;
		XMSamplerUISample m_Sample;
		XMSamplerMixerPage m_Mixer;
		bool init;

	public:
		virtual void PostNcDestroy();
		void Init(XMSampler* pMachine,XMSamplerUI** windowVar);
		void UpdateUI(void);
		XMSampler* GetMachine(){ return _pMachine; }

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnClose();
};

}}
