#if !defined(AFX_DEFAULTVSTGUI_H__44F25597_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
#define AFX_DEFAULTVSTGUI_H__44F25597_ACD9_11D4_937A_BEE48B868538__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefaultVstGui.h : header file
//

#include "VSTHost.h"
#include "PsycleWTLView.h"
#include "IVstGui.h"

/** @file CDefaultVstGui form view definition \n
 *	@brief CDefalutVstGui�_�C�A���O�N���X��`
 *	�����VST�G�f�B�^�_�C�A���O���쐬���܂��B
 *  $Date: 2004/09/19 10:19:38 $
 *  $Revision: 1.2 $
 *******************************************************/

/** @class CDefaultVstGui 
 *  @brief CDefaultVstGui
 */
class CDefaultVstGui : public CDialogImpl<CDefaultVstGui>
{
public:
	CDefaultVstGui();           
	virtual ~CDefaultVstGui();

// Attribute
	/// Set Machine Index \n Machine�̃C���f�b�N�X��ݒ肷��
	void MachineIndex(const int index ){ m_MachineIndex = index;};
	/// Set view pointer \n view �̃|�C���^���Z�b�g����
	void pView(CPsycleWTLView * const pView){m_pView = pView;};
	/// Set Frame Window Pointer \n FrameWindow�̃|�C���^���Z�b�g����
	void pWindow(CWindow * const _pFrameWindow){m_pWindow = _pFrameWindow;};
	/// Set Machine Pointer \n Machine�̃|�C���^���Z�b�g����
	void pMachine(VSTPlugin* const _pVSTPluginMachine){m_pMachine = _pVSTPluginMachine;};
	static const int IDD = IDD_VSTRACK;///< DIalog ID \n �_�C�A���OID;

// Operations
public:
	void Init();///< Initialize Class \n ������
	void UpdateOne();///< Initialize slider and text �X���C�_����уe�L�X�g�̏�����
	void UpdateParList();///< Update parameter list \n �p�����[�^���X�g�̍X�V;
	void UpdateText(const float value);///< Update parameter text \n �p�����[�^�e�L�X�g�̍X�V 
	void UpdateNew(const int par,const float value);///< 
	void InitializePrograms(void);///< Initialize parameter with Program Change \n �p�b�`�`�F���W�ɔ���������
// Message Handlers
	BEGIN_MSG_MAP(CDefaultVstGui)
		COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnCbnSelchangeCombo1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_SLIDER1, NM_CUSTOMDRAW, OnNMCustomdrawSlider1)
		NOTIFY_HANDLER(IDC_SPIN1, UDN_DELTAPOS, OnDeltaposSpin1)
		NOTIFY_HANDLER(IDC_SLIDER1, NM_RELEASEDCAPTURE, OnNMReleasedcaptureSlider1)
		COMMAND_HANDLER(IDC_COMBO1, CBN_CLOSEUP, OnCbnCloseupCombo1)
		COMMAND_HANDLER(IDC_LIST1, LBN_SELCHANGE, OnLbnSelchangeList1)
//		CHAIN_MSG_MAP(CDialogImpl<CDefaultVstGui>)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

private:
	LRESULT OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNMCustomdrawSlider1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled);
	LRESULT OnDeltaposSpin1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMReleasedcaptureSlider1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnCbnCloseupCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLbnSelchangeList1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	CComboBox	m_program;
	CComboBox	m_combo;
	CTrackBarCtrl m_slider;
	CStatic	m_text;
	CListBox	m_parlist;
	
	int m_MachineIndex;
	int m_nPar;
	int m_PreviousProg;
	VSTPlugin* m_pMachine;
	bool m_bUpdatingValue;
	CWindow* m_pWindow;
	CPsycleWTLView* m_pView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFAULTVSTGUI_H__44F25597_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
