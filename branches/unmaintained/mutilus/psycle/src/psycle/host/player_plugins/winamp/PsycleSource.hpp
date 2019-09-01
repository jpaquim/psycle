#pragma once

#include <Wasabi_SDK/studio/studio/wac.h>

class PsycleSourceWnd;

#define WACNAME WACPsycleSource
#define WACPARENT WAComponentClient

class WACNAME : public WACPARENT {
public:
	WACNAME();
	virtual ~WACNAME();
	
	virtual GUID getGUID();
	
	// Put your public methods here.
	
private:
	// Put your private data here.
};

extern WACPARENT *the;
