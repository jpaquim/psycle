// PCGenTrack.h: interface for the PCGenTrack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCGENTRACK_H__8A8DCA09_5308_4DF6_A420_DC630E5F66BD__INCLUDED_)
#define AFX_PCGENTRACK_H__8A8DCA09_5308_4DF6_A420_DC630E5F66BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCTrack.h"
#include "PCGenProcessor.h"	// Added by ClassView

class PCGenTrack : public PCTrack  
{
public:
	PCGenTrack();
	virtual ~PCGenTrack();

protected:
	CTrack& outputTrack;
	PCSignalProcessor Gen;
};

#endif // !defined(AFX_PCGENTRACK_H__8A8DCA09_5308_4DF6_A420_DC630E5F66BD__INCLUDED_)
