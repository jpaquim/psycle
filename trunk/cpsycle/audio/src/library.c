// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "library.h"
#include <stdlib.h>
#include <stdio.h>
#include <portable.h>


psy_Library* psy_library_alloc(void)
{
	return (psy_Library*) malloc(sizeof(psy_Library));
}

psy_Library* psy_library_allocinit(void)
{
	psy_Library* rv;

	rv = psy_library_alloc();
	if (rv) {
		psy_library_init(rv);
	}
	return rv;
}

void psy_library_deallocate(psy_Library* self)
{
	psy_library_dispose(self);
	free(self);
}

int psy_library_empty(psy_Library* self)
{
	return self->module == 0;
}

#if defined(DIVERSALIS__OS__MICROSOFT)

#include <windows.h>
#include <excpt.h>

static int FilterException(const char* path, int code, struct _EXCEPTION_POINTERS *ep) 
{	
	char txt[512];	
		
	psy_snprintf(txt, 512, "Error Load Module %s", path);	
	MessageBox(0, txt, "Psycle Host Exception", MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

void psy_library_init(psy_Library* self)
{
	self->module = 0;
	self->err = 0;
	self->path = _strdup("");
}

void psy_library_load(psy_Library* self, const char* path)
{	
	__try {						
		free(self->path);
		self->path = _strdup(path);
		self->module = LoadLibrary(path);			
		if (self->module == NULL) {
			// LPVOID lpMsgBuf;
			self->err = GetLastError();
			
			/*FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			// Process any inserts in lpMsgBuf.
			// ...
			// Display the string.
			MessageBox( NULL, (LPCTSTR)lpMsgBuf, path, MB_OK | MB_ICONINFORMATION );
			// Free the buffer.
			LocalFree( lpMsgBuf ); */
		}
	} __except(FilterException(path, GetExceptionCode(), GetExceptionInformation())) {
		self->module = 0;
		self->err = 1;
	}
}

void psy_library_unload(psy_Library* self)
{
	psy_library_dispose(self);
	psy_library_init(self);
}

void* psy_library_functionpointer(psy_Library* self, const char* name)
{
	return GetProcAddress(self->module, name);
}

void psy_library_dispose(psy_Library* self)
{
	if (self->module) {
		FreeLibrary(self->module);
		self->err = GetLastError();
		self->module = 0;		
	}
	free(self->path);
	self->path = 0;
}

#elif defined(DIVERSALIS_OS_APPLE)

#include <CoreFoundation/CoreFoundation.h>

void psy_library_init(psy_Library* self)
{
	self->module = 0;
	self->err = 0;
}

void psy_library_load(psy_Library* self, const char* path)
{		
	self->module = CFBundleCreate (NULL, path);	
}

void* psy_library_functionpointerd(psy_Library* self, const char* name)
{
	return (void*)
		CFBundleGetFunctionPointerForName((CFBundleRef)self->module, name);
}

void psy_library_dispose(psy_Library* self)
{
	if (self->module) {
		CFBundleUnloadExecutable ((CFBundleRef)self->module);
		CFRelease ((CFBundleRef) self->module);
		self->module = 0;
	}
}

#elif defined(DIVERSALIS_OS_LINUX)

#include <dlfcn.h>

void psy_library_init(psy_Library* self)
{
	self->module = 0;
	self->err = 0;
}

void psy_library_load(psy_Library* self, const char* path)
{	
	self->module = dlopen(path, RTLD_LAZY /*RTLD_NOW*/);
	
	if (self->module == 0) {
		self->err = dlerror();
	}	
}

void* psy_library_functionpointer(psy_Library* self, const char* name)
{
	return dlsym(self->module, name);
}

void psy_library_dispose(psy_Library* self)
{
	if (self->module) {
		dlclose(self->module);
		self->module = 0;
	}
}			

#endif
