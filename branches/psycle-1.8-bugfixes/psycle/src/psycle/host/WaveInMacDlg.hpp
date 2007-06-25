#pragma once
NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)
/// gear rack window.

class AudioRecorder;
class CChildView;

class CWaveInMacDlg : public CDialog
{
public:
	CWaveInMacDlg(CChildView* pParent);
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
};

NAMESPACE__END
NAMESPACE__END
