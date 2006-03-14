#if !defined(AFX_PRESETSDLG_H__03F91AB1_4F4C_4A97_B805_CDA649E629E9__INCLUDED_)
#define AFX_PRESETSDLG_H__03F91AB1_4F4C_4A97_B805_CDA649E629E9__INCLUDED_

/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PresetsDlg.h : header file
//

#define MAX_PRESETS 256

class Machine;
class CFrameMachine;

/////////////////////////////////////////////////////////////////////////////
// CPreset Class

class CPreset  
{
	int numPars;
	int* params;
	long int sizeData;
	byte* data;
	TCHAR name[32];

public:
	void Clear();
	void Init(const int num);
	void Init(const int num,const TCHAR* newname,int* parameters,int size, byte* newdata);
	void Init(const int num,const TCHAR* newname, float* parameters); // for VST .fxb's
	int GetNumPars() { return numPars; }
	void GetParsArray(int* destarray) { if(numPars>0){ memcpy(destarray,params,numPars*sizeof(int)); }}
	void GetDataArray(byte* destarray) {if(sizeData>0){ memcpy(destarray,data,sizeData); }}
	byte* GetData() {return data;}
	long int GetSizeData() {return sizeData;}
	void SetName(TCHAR *setname) { _tcscpy(name,setname); }
	void GetName(TCHAR *nname) { _tcscpy(nname,name); }
	int GetParam(int n);
	void SetParam(int n,int val);
	CPreset();
	virtual ~CPreset();
	void operator=(CPreset& newpreset);
};

/////////////////////////////////////////////////////////////////////////////
// CPresetsDlg dialog

class CPresetsDlg : public CDialogImpl<CPresetsDlg>
{
// Construction
public:
	void AddPreset(CPreset& preset);
	void AddPreset(const TCHAR* name ,float* parameters);
	void AddPreset(const TCHAR* name,int* parameters, byte* newdata);
	CPresetsDlg(Machine* pMachine,CWindow* pWndFrame);   // standard constructor

// Dialog Data
	enum { IDD = IDD_PRESETS };
	CButton	m_exportButton;
	CButton	m_preview;
	CComboBox	m_preslist;
	BEGIN_MSG_MAP(CPresetsDlg)
		COMMAND_HANDLER(IDC_SAVE, BN_CLICKED, OnBnClickedSave)
		COMMAND_HANDLER(IDC_DELETE, BN_CLICKED, OnBnClickedDelete)
		COMMAND_HANDLER(IDC_IMPORT, BN_CLICKED, OnBnClickedImport)
		COMMAND_HANDLER(IDC_EXPORT, BN_CLICKED, OnBnClickedExport)
		COMMAND_HANDLER(IDC_PREVIEW, BN_CLICKED, OnBnClickedPreview)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		COMMAND_HANDLER(IDC_PRESETSLIST, CBN_DBLCLK, OnCbnDblclkPresetslist)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_PRESETSLIST, CBN_SELCHANGE, OnCbnSelchangePresetslist)
	END_MSG_MAP()
private:
	LRESULT OnBnClickedSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnDblclkPresetslist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCbnSelchangePresetslist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	CWindow* m_pWndFrame;
	Machine* m_pMachine;
	int numParameters;
	long int sizeDataStruct;
	int fileversion;
	void TweakMachine(CPreset &preset);
	void SavePresets();
	void ReadPresets();
	void UpdateList();
	CPreset iniPreset;
	CPreset presets[MAX_PRESETS];
	CString fileName;
	bool presetChanged;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESETSDLG_H__03F91AB1_4F4C_4A97_B805_CDA649E629E9__INCLUDED_)
