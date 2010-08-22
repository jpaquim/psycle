#ifndef PSYCLE__HOST__EXCLUSIVELOCK__INCLUDED
#define PSYCLE__HOST__EXCLUSIVELOCK__INCLUDED
#pragma once

#include "Psycle.hpp"

#ifndef __AFXMT_H__
	#include <afxmt.h>
#endif

class CExclusiveLock
{
public:
//Constructors / Destructors
  CExclusiveLock(CSemaphore* in_semaphore,int places, bool lock_now=true);
  ~CExclusiveLock();

  void Lock();
  void UnLock();
  CSemaphore & operator=(const CSemaphore& other);
protected:
	CSemaphore* semaphore;
	int		num_places;
	bool	locked;
};

#endif // PSYCLE__HOST__EXCLUSIVELOCK_INCLUDED
