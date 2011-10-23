#include <psycle/host/detail/project.private.hpp>
#include "ExclusiveLock.hpp"

///////////////////////////////// Implementation //////////////////////////////

CExclusiveLock::CExclusiveLock(CSemaphore* in_semaphore, int places, bool lock_now)
:semaphore(in_semaphore), num_places(places), locked(false) {
	if(lock_now) Lock();
}

CExclusiveLock::~CExclusiveLock()
{
	if (locked) { UnLock(); }
}
void CExclusiveLock::Lock() {
	if (locked) return;
	for(int i=0;i<num_places;i++) semaphore->Lock();
	locked=true;
}

void CExclusiveLock::UnLock() {
	if(!locked) return;
	for(int i=0;i<num_places;i++) semaphore->Unlock();
	locked=false;
}
