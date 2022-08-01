/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_THREAD_H
#define psy_THREAD_H

#include "../../detail/psydef.h"
#include "../../detail/os.h"

#if defined DIVERSALIS__OS__POSIX
	#include <pthread.h>
#elif defined DIVERSALIS__OS__MICROSOFT
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#error "unsupported operating system"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* native handle type */
typedef
#if defined DIVERSALIS__OS__POSIX
	pthread_t
#elif defined DIVERSALIS__OS__MICROSOFT
	HANDLE
#else
	#error "unsupported operating system"
#endif
psy_native_handle_type;

#if defined DIVERSALIS__OS__MICROSOFT
typedef unsigned int (__stdcall *psy_fp_thread_callback)(void* context);
#else
typedef unsigned int (*psy_fp_thread_callback)(void* context);
#endif

typedef struct psy_Thread {
	psy_native_handle_type native_handle_;
} psy_Thread;

/* initialises an instance */
void psy_thread_init(psy_Thread*);
/* initialises an instance that represents the given thread */
void psy_thread_init_all(psy_Thread*, psy_native_handle_type);
/* starts a new thread with the the given thread function */
void psy_thread_init_start(psy_Thread*, void* context, psy_fp_thread_callback);
/* initialises an instance that represents the given thread */
void psy_thread_dispose(psy_Thread*);

void psy_thread_start(psy_Thread*, void* context, psy_fp_thread_callback);
void psy_thread_join(psy_Thread*);

void psy_sleep_for(uintptr_t ns);
uintptr_t psy_thread_hardware_concurrency(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_THREAD_H */
