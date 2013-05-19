#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "SampleAssignEditor.hpp"

namespace psycle { namespace host {

class XMSampler;
class XMInstrument;

class CInstrumentGenDlg : public CDialog
{
public:
	CInstrumentGenDlg();
	virtual ~CInstrumentGenDlg();

	/// Dialog ID
	enum { IDD = IDD_XMSAMP_INSTGEN };

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
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBtnSetDefaults();
	afx_msg void OnBtnSetSample();
	afx_msg void OnBtnIncreaseOct();
	afx_msg void OnBtnDecreaseOct();

public:
	void AssignGeneralValues(XMInstrument& inst);

private:
	void SetNewNoteAction(const int nna,const int dct,const int dca);

	CSampleAssignEditor m_SampleAssign;

	CEdit m_InstrumentName;
	CComboBox m_NNA;
	CComboBox m_DCT;
	CComboBox m_DCA;
	CEdit m_SampleNumber;
	CScrollBar m_scBar;

	bool m_bInitialized;

	XMInstrument *m_instr;

};

}}