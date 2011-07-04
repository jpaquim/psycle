#if !defined(AFX_MIDILEARN_H__A28856CD_9197_4DCB_9D3B_C5D8B8A94B5A__INCLUDED_)
#define AFX_MIDILEARN_H__A28856CD_9197_4DCB_9D3B_C5D8B8A94B5A__INCLUDED_
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiLearn.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CMidiLearn dialog

class CMidiLearn : public CDialogImpl<CMidiLearn> {
// Construction
public:
	CMidiLearn();   // standard constructor

	int Message;
	void __cdecl MidiCallback_Test( HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );

// Dialog Data
	enum { IDD = IDD_MIDI_LEARN };
	CStatic		m_Text;

// Implementation
protected:

	void OnCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDILEARN_H__A28856CD_9197_4DCB_9D3B_C5D8B8A94B5A__INCLUDED_)
