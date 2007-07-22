#pragma once
#include "XMSamplerUIGeneral.hpp"
#include "XMSamplerUIInst.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSamplerMixerPage.hpp"
#include <psycle/engine/constants.hpp>

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

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

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
