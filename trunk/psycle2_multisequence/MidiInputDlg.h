#if !defined(AFX_MIDIINPUTDLG_H__48F02BAA_A9CC_48DF_9F60_D4E2122D46CC__INCLUDED_)
#define AFX_MIDIINPUTDLG_H__48F02BAA_A9CC_48DF_9F60_D4E2122D46CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiInputDlg.h : header file
//

#include "AudioDriver.h"

/////////////////////////////////////////////////////////////////////////////
// CMidiInputDlg dialog

class CMidiInputDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CMidiInputDlg)

// Construction
public:
	CMidiInputDlg();
	~CMidiInputDlg();

// Dialog Data
	//{{AFX_DATA(CMidiInputDlg)
	enum { IDD = IDD_MIDI_INPUT };
	CButton		m_midiRecordVelButton;
	CComboBox	m_midiTypeVelComboBox;
	CEdit		m_midiCommandVelEdit;
	CEdit		m_midiFromVelEdit;
	CEdit		m_midiToVelEdit;
	CButton		m_midiRecordPitButton;
	CComboBox	m_midiTypePitComboBox;
	CEdit		m_midiCommandPitEdit;
	CEdit		m_midiFromPitEdit;
	CEdit		m_midiToPitEdit;
	CButton		m_midiRecord0Button;
	CEdit		m_midiMessage0Edit;
	CComboBox	m_midiType0ComboBox;
	CEdit		m_midiCommand0Edit;
	CEdit		m_midiFrom0Edit;
	CEdit		m_midiTo0Edit;
	CButton		m_midiRecord1Button;
	CEdit		m_midiMessage1Edit;
	CComboBox	m_midiType1ComboBox;
	CEdit		m_midiCommand1Edit;
	CEdit		m_midiFrom1Edit;
	CEdit		m_midiTo1Edit;
	CButton		m_midiRecord2Button;
	CEdit		m_midiMessage2Edit;
	CComboBox	m_midiType2ComboBox;
	CEdit		m_midiCommand2Edit;
	CEdit		m_midiFrom2Edit;
	CEdit		m_midiTo2Edit;
	CButton		m_midiRecord3Button;
	CEdit		m_midiMessage3Edit;
	CComboBox	m_midiType3ComboBox;
	CEdit		m_midiCommand3Edit;
	CEdit		m_midiFrom3Edit;
	CEdit		m_midiTo3Edit;
	CButton		m_midiRecord4Button;
	CEdit		m_midiMessage4Edit;
	CComboBox	m_midiType4ComboBox;
	CEdit		m_midiCommand4Edit;
	CEdit		m_midiFrom4Edit;
	CEdit		m_midiTo4Edit;
	CButton		m_midiRecord5Button;
	CEdit		m_midiMessage5Edit;
	CComboBox	m_midiType5ComboBox;
	CEdit		m_midiCommand5Edit;
	CEdit		m_midiFrom5Edit;
	CEdit		m_midiTo5Edit;
	CButton		m_midiRecord6Button;
	CEdit		m_midiMessage6Edit;
	CComboBox	m_midiType6ComboBox;
	CEdit		m_midiCommand6Edit;
	CEdit		m_midiFrom6Edit;
	CEdit		m_midiTo6Edit;
	CButton		m_midiRecord7Button;
	CEdit		m_midiMessage7Edit;
	CComboBox	m_midiType7ComboBox;
	CEdit		m_midiCommand7Edit;
	CEdit		m_midiFrom7Edit;
	CEdit		m_midiTo7Edit;
	CButton		m_midiRecord8Button;
	CEdit		m_midiMessage8Edit;
	CComboBox	m_midiType8ComboBox;
	CEdit		m_midiCommand8Edit;
	CEdit		m_midiFrom8Edit;
	CEdit		m_midiTo8Edit;
	CButton		m_midiRecord9Button;
	CEdit		m_midiMessage9Edit;
	CComboBox	m_midiType9ComboBox;
	CEdit		m_midiCommand9Edit;
	CEdit		m_midiFrom9Edit;
	CEdit		m_midiTo9Edit;
	CButton		m_midiRecord10Button;
	CEdit		m_midiMessage10Edit;
	CComboBox	m_midiType10ComboBox;
	CEdit		m_midiCommand10Edit;
	CEdit		m_midiFrom10Edit;
	CEdit		m_midiTo10Edit;
	CButton		m_midiRecord11Button;
	CEdit		m_midiMessage11Edit;
	CComboBox	m_midiType11ComboBox;
	CEdit		m_midiCommand11Edit;
	CEdit		m_midiFrom11Edit;
	CEdit		m_midiTo11Edit;
	CButton		m_midiRecord12Button;
	CEdit		m_midiMessage12Edit;
	CComboBox	m_midiType12ComboBox;
	CEdit		m_midiCommand12Edit;
	CEdit		m_midiFrom12Edit;
	CEdit		m_midiTo12Edit;
	CButton		m_midiRecord13Button;
	CEdit		m_midiMessage13Edit;
	CComboBox	m_midiType13ComboBox;
	CEdit		m_midiCommand13Edit;
	CEdit		m_midiFrom13Edit;
	CEdit		m_midiTo13Edit;
	CButton		m_midiRecord14Button;
	CEdit		m_midiMessage14Edit;
	CComboBox	m_midiType14ComboBox;
	CEdit		m_midiCommand14Edit;
	CEdit		m_midiFrom14Edit;
	CEdit		m_midiTo14Edit;
	CButton		m_midiRecord15Button;
	CEdit		m_midiMessage15Edit;
	CComboBox	m_midiType15ComboBox;
	CEdit		m_midiCommand15Edit;
	CEdit		m_midiFrom15Edit;
	CEdit		m_midiTo15Edit;
	CButton		m_midiRawMcm;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMidiInputDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMidiInputDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDIINPUTDLG_H__48F02BAA_A9CC_48DF_9F60_D4E2122D46CC__INCLUDED_)
