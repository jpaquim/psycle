#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "XMInstrument.hpp"
#include "InstrumentGenDlg.hpp"
#include "InstrumentAmpDlg.hpp"
#include "InstrumentPanDlg.hpp"
#include "InstrumentFilDlg.hpp"
#include "InstrumentPitDlg.hpp"

namespace psycle { namespace host {

class XMSampler;

class XMSamplerUIInst : public CPropertyPage
{
public:
	DECLARE_DYNAMIC(XMSamplerUIInst)

public:
	XMSamplerUIInst();
	virtual ~XMSamplerUIInst();

	/// Dialog ID
	enum { IDD = IDD_XMSAMPLERUIINST };

	void pMachine(XMSampler * const p){m_pMachine = p;};
	XMSampler * const pMachine(){return m_pMachine;};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV
	virtual BOOL OnInitDialog();


protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnSetActive(void);

	afx_msg void OnLbnSelchangeInstrumentlist();
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedLoadins();
	afx_msg void OnBnClickedSaveins();
	afx_msg void OnBnClickedDupeins();
	afx_msg void OnBnClickedDeleteins();

private:

	void SetInstrumentData(const int instno);


	XMSampler *m_pMachine;
	bool m_bInitialized;
	int		m_iCurrentSelected;

protected:
	CListBox m_InstrumentList;

	CTabCtrl m_tabMain;
	CInstrumentGenDlg m_genTab;
	CInstrumentAmpDlg m_ampTab;
	CInstrumentPanDlg m_panTab;
	CInstrumentFilDlg m_filTab;
	CInstrumentPitDlg m_pitTab;
};

}}
