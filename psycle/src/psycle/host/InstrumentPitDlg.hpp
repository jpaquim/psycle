#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "EnvelopeEditorDlg.hpp"

namespace psycle { namespace host {

class XMInstrument;

class CInstrumentPitDlg : public CDialog
{
public:
	CInstrumentPitDlg();
	virtual ~CInstrumentPitDlg();

	/// Dialog ID
	enum { IDD = IDD_INST_SAMPULSE_INSTPIT };

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void AssignPitchValues(XMInstrument& inst);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

private:
	void SliderTune(CSliderCtrl* slid);
	void SliderGlide(CSliderCtrl* slid);

	CEnvelopeEditorDlg m_EnvelopeEditorDlg;

	CSliderCtrl m_SlVolCutoffPan;
	CSliderCtrl m_SlSwing1Glide;

	XMInstrument *m_instr;
};

}}