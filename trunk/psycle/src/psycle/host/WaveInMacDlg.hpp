#pragma once
#include <afxwin.h>
#include <afxcmn.h>
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

/// gear rack window.

class AudioRecorder;
class CChildView;

class CWaveInMacDlg : public CDialog
{
public:
	CWaveInMacDlg(CChildView* pParent);
	CWaveInMacDlg(CChildView* pParent, class MachineGui* gui);

	CChildView* m_pParent;
	void RedrawList();
	BOOL Create();
	afx_msg void OnCancel();

	CComboBox m_listbox;
	AudioRecorder *pRecorder;
	enum { IDD = IDD_MACWAVEIN };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void FillCombobox();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelendokCombo1();
	CStatic m_vollabel;
	afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl m_volslider;
private:
	MachineGui* gui_;
};

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
