#pragma once
#include "resource.h"

class FS_SF2_UI :
	public CDialog
{
	DECLARE_DYNAMIC(FS_SF2_UI)
public:
	FS_SF2_UI(CWnd* pParent = NULL);
	virtual ~FS_SF2_UI();
	// Dialog Data
	enum { IDD = IDD_FS_SF2_DIALOG };

	CDialog m_wndFSUI;
protected:
	DECLARE_MESSAGE_MAP()
};
