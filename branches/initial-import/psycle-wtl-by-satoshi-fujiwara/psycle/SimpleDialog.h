#pragma once
/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */


namespace SF {

class CSimpleDialog :
	public CDialogImpl<CSimpleDialog>
{
public:
	//enum { IDD = IDD_PSYBAR2};
	const int IDD;
	CSimpleDialog(int id ) : IDD(id) { ;};
	~CSimpleDialog(void){;};
	BEGIN_MSG_MAP(CTestDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		FORWARD_NOTIFICATIONS() 
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		ModifyStyle(0, WS_CLIPSIBLINGS);
		SetDlgCtrlID(IDD);
		ExecuteDlgInit(IDD);
		return 0;
	};
};
}
