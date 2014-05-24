#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "SampleAssignEditor.hpp"
#include "EnvelopeEditorDlg.hpp"

namespace psycle { namespace host {

class XMSampler;
class XMInstrument;

class CInstrumentGenDlg : public CDialog
{
public:
	CInstrumentGenDlg();
	virtual ~CInstrumentGenDlg();

	/// Dialog ID
//	enum { IDD = IDD_INST_SAMPULSE_INSTGEN };
	enum { IDD = IDD_INST_SAMPULSE_INSTPIT_NEW };

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeInsName();
	afx_msg void OnCbnSelendokInsNnacombo();
	afx_msg void OnCbnSelendokInsDctcombo();
	afx_msg void OnCbnSelendokInsDcacombo();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBtnSetDefaults();
	afx_msg void OnBtnEditMapping();
	afx_msg void OnBtnPopupSetSample();
	afx_msg void OnSetSample(UINT nID);
	afx_msg void OnBtnIncreaseOct();
	afx_msg void OnBtnDecreaseOct();
	afx_msg void OnBtnIncreaseNote();
	afx_msg void OnBtnDecreaseNote();

public:
	void AssignGeneralValues(XMInstrument& inst, int instno);

private:
	void SetNewNoteAction(const int nna,const int dct,const int dca);
	void ValidateEnabled();

	CSampleAssignEditor m_SampleAssign;

	CEdit m_InstrumentName;
	CComboBox m_NNA;
	CComboBox m_DCT;
	CComboBox m_DCA;
	CScrollBar m_scBar;
	CComboBox m_ShiftMove;
	CEnvelopeEditorDlg m_EnvelopeEditorDlg;

	bool m_bInitialized;

	XMInstrument *m_instr;
	int m_instIdx;

};

}}