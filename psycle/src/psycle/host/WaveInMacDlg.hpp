#pragma once
#include "Psycle.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psycle {
	namespace core {
		class AudioRecorder;
	}
}
using namespace psycle::core;
#endif

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

#if !PSYCLE__CONFIGURATION__USE_PSYCORE
	class AudioRecorder;
#endif

class CChildView;

class CWaveInMacDlg : public CDialog
{
public:
	CWaveInMacDlg(CChildView* pParent);
	CWaveInMacDlg(CChildView* pParent, class MachineGui* gui);

	CChildView* m_pParent;
	void RedrawList();
	BOOL Create();
	void Show(int x, int y);
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
	void centerWindowOnPoint(int x, int y);
};

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
