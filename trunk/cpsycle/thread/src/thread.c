/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "../../detail/os.h"
/* local */
#include "thread.h"

#if defined DIVERSALIS__OS__POSIX
	#include <unistd.h>
#elif defined DIVERSALIS__OS__MICROSOFT
	#include <process.h>
#else
	#error "unsupported operating system"
#endif


/* implementation */
void psy_thread_init(psy_Thread* self)
{	
	self->native_handle_ = 0;	
	/*self->native_handle_ = 
	#if defined DIVERSALIS__OS__POSIX
		self->native_handle_ = pthread_self();
	#elif defined DIVERSALIS__OS__MICROSOFT		
		self->native_handle_ = GetCurrentThread();
	#else
		#error "unsupported operating system"
	#endif */
}

void psy_thread_init_all(psy_Thread* self, psy_native_handle_type native_handle)
{
	self->native_handle_ = native_handle;
}

void psy_thread_init_start(psy_Thread* self, void* context, psy_fp_thread_callback callback)
{	
#if defined DIVERSALIS__OS__POSIX
	pthread_create(&self->native_handle_, NULL, (void* (*)(void*))callback,
		(void*)context);
#elif defined DIVERSALIS__OS__MICROSOFT		
	self->native_handle_ = (HANDLE)_beginthreadex(0, 0, callback, context, 0, 0);		
#else
	#error "unsupported operating system"
#endif
}

void psy_thread_dispose(psy_Thread* self)
{	
	if (self->native_handle_) {
#if defined DIVERSALIS__OS__MICROSOFT
		psy_thread_join(self);
		CloseHandle(self->native_handle_);
#endif
	}
}

void psy_thread_start(psy_Thread* self, void* context, psy_fp_thread_callback callback)
{	
	#if defined DIVERSALIS__OS__POSIX
		pthread_create(&self->native_handle_, NULL, (void* (*)(void*))callback,
			(void*)context);
	#elif defined DIVERSALIS__OS__MICROSOFT		
		self->native_handle_ = (HANDLE)_beginthreadex(0, 0, callback, context, 0, 0);		
	#else
		#error "unsupported operating system"
	#endif
}


void psy_thread_join(psy_Thread* self)
{
	if (self->native_handle_ != 0) {
		#if defined DIVERSALIS__OS__POSIX
			int ret;
			void *ret_join;

			ret = pthread_join(self->native_handle_, &ret_join);			
		#elif defined DIVERSALIS__OS__MICROSOFT		
			WaitForSingleObject(self->native_handle_, INFINITE);
		#else
			#error "unsupported operating system"
		#endif
	}
}

void psy_sleep_for(uintptr_t ns)
{	
	#if defined DIVERSALIS__OS__POSIX		
		usleep(ns);
	#elif defined DIVERSALIS__OS__MICROSOFT		
		Sleep(ns / 1000 /* microseconds */);
	#else
		#error "unsupported operating system"
	#endif
}

uintptr_t psy_thread_hardware_concurrency(void)
{
	#if defined DIVERSALIS__OS__POSIX		
		return sysconf(_SC_NPROCESSORS_ONLN);
	#elif defined DIVERSALIS__OS__MICROSOFT		
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		return sysinfo.dwNumberOfProcessors;
	#else
		#error "unsupported operating system"
		return 0;
	#endif
}
