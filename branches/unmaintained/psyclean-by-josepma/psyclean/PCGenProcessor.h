// PCGenProcessor.h: interface for the PCGenProcessor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCGENPROCESSOR_H__C09D50CC_FB17_49A0_83FB_A32A044AEE78__INCLUDED_)
#define AFX_PCGENPROCESSOR_H__C09D50CC_FB17_49A0_83FB_A32A044AEE78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCSignalProcessor.h"

class PCGenProcessor : public PCSignalProcessor  
{
public:
	PCGenProcessor();
	virtual ~PCGenProcessor();

};

#endif // !defined(AFX_PCGENPROCESSOR_H__C09D50CC_FB17_49A0_83FB_A32A044AEE78__INCLUDED_)
