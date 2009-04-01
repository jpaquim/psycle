/*
Module : SINSTANCE.H
Purpose: Defines the interface for an MFC wrapper class to do instance checking
Created: PJN / 29-07-1998
History: PJN / 25-03-2000 Neville Franks made the following changes. Contact nevf@getsoft.com, www.getsoft.com
                          1. Changed #pragma error to #pragma message. Former wouldn't compile under VC6
                          2. Replaced above #pragma with #include
                          3. Added TrackFirstInstanceRunning(), MakeMMFFilename()

Copyright (c) 1998 - 2000 by PJ Naughter.  
All rights reserved.
*/

#ifndef __SINSTANCE_H__
#define __SINSTANCE_H__

#include "Psycle.hpp"
#ifndef __AFXMT_H__
#include <afxmt.h>
#endif

class CInstanceChecker : public CObject
{
public:
//Constructors / Destructors
  CInstanceChecker();
  ~CInstanceChecker();

//General functions
  BOOL TrackFirstInstanceRunning();
  BOOL PreviousInstanceRunning();
  HWND ActivatePreviousInstance(); 

protected:
  CString MakeMMFFilename();
	CMutex m_instanceDataMutex;
	HANDLE m_hPrevInstance;
};


#endif //__SINSTANCE_H__
