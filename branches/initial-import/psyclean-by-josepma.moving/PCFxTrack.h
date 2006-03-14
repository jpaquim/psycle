// PCFxTrack.h: interface for the PCFxTrack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCFXTRACK_H__640DCD1A_7648_4F1C_927F_E62A9048D9B6__INCLUDED_)
#define AFX_PCFXTRACK_H__640DCD1A_7648_4F1C_927F_E62A9048D9B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCTrack.h"

class PCFxTrack : public PCTrack  
{
public:
	PCFxTrack();
	virtual ~PCFxTrack();

protected:
		CTrack& outputTrack;

};

#endif // !defined(AFX_PCFXTRACK_H__640DCD1A_7648_4F1C_927F_E62A9048D9B6__INCLUDED_)
