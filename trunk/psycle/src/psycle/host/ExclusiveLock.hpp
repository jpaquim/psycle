#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

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
