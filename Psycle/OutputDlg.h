#if !defined(AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_)
#define AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OutputDlg.h : header file
//

#include "AudioDriver.h"

/////////////////////////////////////////////////////////////////////////////
// Defines

#define	MIN_HEADROOM	0
#define	MAX_HEADROOM	9999

/////////////////////////////////////////////////////////////////////////////
// COutputDlg dialog

class COutputDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(COutputDlg)

// Construction
public:
	int m_driverIndex;
	int m_midiDriverIndex;		// MIDI IMPLEMENTATION
	int m_syncDriverIndex;
	int _numDrivers;
	int _numMidiDrivers;		// MIDI IMPLEMENTATION
	int m_midiHeadroom;
	AudioDriver** m_ppDrivers;

	COutputDlg();   // standard constructor

// Dialog Data
	//{{AFX_DATA(COutputDlg)
	enum { IDD = IDD_OUTPUT };
	CComboBox	m_midiDriverComboBox;
	CComboBox	m_midiSyncComboBox;
	CComboBox	m_driverComboBox;
	CEdit		m_midiHeadroomEdit;
	CSpinButtonCtrl m_midiHeadroomSpin;
	CButton		m_midiMachineViewSeqMode;
	CButton		m_midiRecordVelocityButton;
	CButton		m_midiRecordPitchButton;
	CButton		m_midiRecordModwheelButton;
	CComboBox	m_midiVelocityTypeComboBox;
	CComboBox	m_midiPitchTypeComboBox;
	CComboBox	m_midiModWheelTypeComboBox;
	CEdit		m_midiVelocityCommandEdit;
	CEdit		m_midiVelocityFromEdit;
	CEdit		m_midiVelocityToEdit;
	CEdit		m_midiPitchCommandEdit;
	CEdit		m_midiPitchFromEdit;
	CEdit		m_midiPitchToEdit;
	CEdit		m_midiModWheelCommandEdit;
	CEdit		m_midiModWheelFromEdit;
	CEdit		m_midiModWheelToEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_oldDriverIndex;
	int m_oldMidiDriverIndex;		// MIDI IMPLEMENTATION
	int m_oldSyncDriverIndex;

	// Generated message map functions
	//{{AFX_MSG(COutputDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnConfig();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_)
