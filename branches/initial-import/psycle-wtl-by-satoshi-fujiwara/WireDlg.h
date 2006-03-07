#if !defined(AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
#define AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_


/** @file
 *  @brief Wire Dialog 
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Machine.h"
#include "constants.h"

// Foward Declarations 
class CPsycleWTLView;
class Song;

/** @class CWireDlg
 *  @brief Wire Dialog Class \n */
class CWireDlg : public CDialogImpl<CWireDlg>
{

// Construction
public:
	static const int MAX_SCOPE_BANDS = 64;
	static const int SCOPE_BUF_SIZE = 4096;
	static const int SCOPE_SPEC_SAMPLES = 256;

	CWireDlg(CPsycleWTLView *pParent);///< standard constructor
	
	UINT this_index;
	int wireIndex;
	int isrcMac;
	bool Inval;
	Machine* _pSrcMachine;
	Machine* _pDstMachine;
	int _dstWireIndex;
	float invol;
	float mult;

	int scope_mode;
	int scope_peak_rate;
	int scope_osc_freq;
	int scope_osc_rate;
	int scope_spec_bands;
	int scope_spec_rate;
	int scope_phase_rate;

	float peakL,peakR;
	float peak2L,peak2R;
	int peakLifeL,peakLifeR;

	float o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;


	float pSamplesL[SCOPE_BUF_SIZE];
	float pSamplesR[SCOPE_BUF_SIZE];
// Dialog Data
	enum { IDD = IDD_WIREDIALOG };
	CTrackBarCtrl	m_slider;
	CTrackBarCtrl	m_slider2;
	CStatic	m_volabel_per;
	CStatic	m_volabel_db;
	CButton m_mode;
	CTrackBarCtrl	m_volslider;

// Implementation
protected:
	inline int GetY(float f);
	void SetMode();
	CPsycleWTLView* m_pParent;
	CBitmap* bufBM;
	CBitmap* clearBM;
	CPen linepenL;
	CPen linepenR;
	CPen linepenbL;
	CPen linepenbR;
	CRect rc;
	CFont font;
	CFont* oldFont;
	BOOL hold;
	BOOL clip;
	int pos;
	int bar_heightsl[MAX_SCOPE_BANDS];
	int bar_heightsr[MAX_SCOPE_BANDS];
public:
	
	BEGIN_MSG_MAP(CWireDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_SLIDER1, NM_CUSTOMDRAW, OnNMCustomdrawSlider1)
		COMMAND_HANDLER(IDC_BUTTON, BN_CLICKED, OnBnClickedButton)
		COMMAND_HANDLER(IDC_BUTTON2, BN_CLICKED, OnBnClickedButton2)
		COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
		COMMAND_HANDLER(IDC_VOLUME_DB, BN_CLICKED, OnBnClickedVolumeDb)
		COMMAND_HANDLER(IDC_VOLUME_PER, BN_CLICKED, OnBnClickedVolumePer)
		NOTIFY_HANDLER(IDC_SLIDER, NM_CUSTOMDRAW, OnNMCustomdrawSlider)
		NOTIFY_HANDLER(IDC_SLIDER2, NM_CUSTOMDRAW, OnNMCustomdrawSlider2)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	void OnCancel();
	virtual void OnFinalMessage(HWND hWnd);
private:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNMCustomdrawSlider1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedVolumeDb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedVolumePer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSlider(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSlider2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WIREDLG_H__078666B6_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
