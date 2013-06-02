#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "XMInstrument.hpp"
#include "EnvelopeEditorDlg.hpp"

namespace psycle { namespace host {

class XMSampler;

class CInstrumentFilDlg : public CDialog
{
public:
	CInstrumentFilDlg();
	virtual ~CInstrumentFilDlg();

	/// Dialog ID
	enum { IDD = IDD_INST_SAMPULSE_INSTFIL };

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void AssignFilterValues(XMInstrument& inst);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelendokFiltertype();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnEnableCutoff();
	afx_msg void OnBnEnableRessonance();
private:
	void SliderCutoff(CSliderCtrl* slid);
	void SliderGlideCut(CSliderCtrl* slid);
	void SliderRessonance(CSliderCtrl* slid);
	void SliderGlideRes(CSliderCtrl* slid);
	void SliderModNote(CSliderCtrl* slid);
	void SliderMod(CSliderCtrl* slid);

	CEnvelopeEditorDlg m_EnvelopeEditorDlg;
	CComboBox m_FilterType;
	CSliderCtrl m_SlVolCutoffPan;
	CButton m_cutoffPan;
	CSliderCtrl m_SlSwing1Glide;
	CButton m_Ressonance;
	CSliderCtrl m_SlFadeoutRes;
	CSliderCtrl m_SlSwing2;
	CSliderCtrl m_SlNoteModNote;
	CSliderCtrl m_SlNoteMod;

	XMInstrument *m_instr;

};

}}