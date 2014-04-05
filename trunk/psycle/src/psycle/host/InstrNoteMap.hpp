#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include <psycle/host/XMInstrument.hpp>

namespace psycle { namespace host {

class CInstrNoteMap: public CDialog
{
public:
	CInstrNoteMap();
	virtual ~CInstrNoteMap();

	enum { IDD = IDD_INST_NOTEMAP };

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	afx_msg void OnSelchangeSampleList();
	afx_msg void OnSelchangeNoteList();
	afx_msg void OnSelchangeNoteMapList();
	afx_msg void OnRadioEdit();
	afx_msg void OnRadioShow();

private:
	void RefreshSampleList(int sample);
	void RefreshNoteList();
	void RefreshNoteMapList(bool reset=true);

	static const char* c_Key_name[12];

	CListBox m_SampleList;
	CListBox m_NoteList;
	CListBox m_NoteMapList;
	CButton	m_radio_edit;

	/// Table of mapped notes to samples
	/// (note number=first, sample number=second)
	///\todo Could it be interesting to map other things like volume,panning, cutoff...?
	XMInstrument::NotePair m_AssignNoteToSample[XMInstrument::NOTE_MAP_SIZE];
public:
	XMInstrument *m_instr;
};

}}