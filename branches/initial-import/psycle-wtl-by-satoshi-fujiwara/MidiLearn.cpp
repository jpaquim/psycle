/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */

#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "OutputDlg.h"
#include "MidiInput.h"
#include "configuration.h"
#include "MidiLearn.h"




/////////////////////////////////////////////////////////////////////////////
// CMidiLearn dialog


CMidiLearn::CMidiLearn()
{
	//{{AFX_DATA_INIT(CMidiLearn)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Message = -1;
//	Global::pConfig->_pMidiInput->fnMidiCallback_Test = (MidiCallback_Test);
}


/////////////////////////////////////////////////////////////////////////////
// CMidiLearn message handlers

void CMidiLearn::OnCancel() 
{
	// TODO: Add extra cleanup here
	Message = -1;
	//CDialog::OnCancel();
}

