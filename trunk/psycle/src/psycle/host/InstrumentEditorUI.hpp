#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "InstrumentEditor.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSamplerUIInst.hpp"

namespace psycle { namespace host {

/////////////////////////////////////////////////////////////////////////////
// InstrumentEditorUI dialog

class InstrumentEditorUI : public CPropertySheet
{
	DECLARE_DYNAMIC(InstrumentEditorUI)

	public:
		InstrumentEditorUI(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		InstrumentEditorUI(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		virtual ~InstrumentEditorUI();

		enum { IDD = IDD_INSTRUMENTUI };

	private:
		InstrumentEditorUI** windowVar_;
		CInstrumentEditor m_InstrBasic;
		XMSamplerUIInst m_InstrSampulse;
		XMSamplerUISample m_SampleBank;
		bool init;

	public:
		virtual void PostNcDestroy();
		void Init(InstrumentEditorUI** windowVar);
		void UpdateUI(void);
		BOOL PreTranslateChildMessage(MSG* pMsg, HWND focusWin);

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnClose();
};

}}
