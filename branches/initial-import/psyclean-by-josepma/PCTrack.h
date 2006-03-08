// PCTrack.h: interface for the PCTrack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCTRACK_H__CFA0BFC6_C0B1_4909_AF4C_6EC723AB14AE__INCLUDED_)
#define AFX_PCTRACK_H__CFA0BFC6_C0B1_4909_AF4C_6EC723AB14AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class PCTrack  
{
public:
	PCTrack();
	virtual ~PCTrack();

protected:
	CList<PCSignalProcessor,PCSignalProcessor&> fxlist(1);

};

#endif // !defined(AFX_PCTRACK_H__CFA0BFC6_C0B1_4909_AF4C_6EC723AB14AE__INCLUDED_)
