// PCMixer.h: interface for the PCMixer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCMIXER_H__5CFB0C3A_6F6C_4391_BEFA_EC7462F5F3F8__INCLUDED_)
#define AFX_PCMIXER_H__5CFB0C3A_6F6C_4391_BEFA_EC7462F5F3F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCSignalProcessor.h"


class PCMixer  
{
public:
	void GenerateSound(void);
	PCMixer();
	virtual ~PCMixer();

};

#endif // !defined(AFX_PCMIXER_H__5CFB0C3A_6F6C_4391_BEFA_EC7462F5F3F8__INCLUDED_)
