// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "library.h"
#include <stdlib.h>

#define __cplusplus
#include <diversalis/os.hpp>
#undef __cplusplus

Library* library_alloc(void)
{
	return (Library*) malloc(sizeof(Library));
}

Library* library_allocinit(void)
{
	Library* rv;

	rv = library_alloc();
	if (rv) {
		library_init(rv);
	}
	return rv;
}

#if defined(DIVERSALIS__OS__MICROSOFT)

#include <windows.h>

void library_init(Library* self)
{
	self->module = 0;
	self->err = 0;
	self->path = _strdup("");
}

void library_load(Library* self, const char* path)
{	
	self->module = LoadLibrary(path);	
	free(self->path);
	self->path = _strdup(path);	
	if (self->module == NULL) {
		self->err = GetLastError();			
	}	
}

void library_unload(Library* self)
{
	library_dispose(self);
	library_init(self);
}

void* library_functionpointer(Library* self, const char* name)
{
	return GetProcAddress(self->module, name);
}

void library_dispose(Library* self)
{
	if (self->module) {
		FreeLibrary(self->module);
		self->err = GetLastError();
		self->module = 0;
		free(self->path);
	}
}

#elif defined(DIVERSALIS_OS_APPLE)

#include <CoreFoundation/CoreFoundation.h>

void library_init(Library* self)
{
	self->module = 0;
	self->err = 0;
}

void library_load(Library* self, const char* path)
{		
	self->module = CFBundleCreate (NULL, path);	
}

void* library_functionpointerd(Library* self, const char* name)
{
	return (void*)
		CFBundleGetFunctionPointerForName((CFBundleRef)self->module, name);
}

void library_dispose(Library* self)
{
	if (self->module) {
		CFBundleUnloadExecutable ((CFBundleRef)self->module);
		CFRelease ((CFBundleRef) self->module);
		self->module = 0;
	}
}

#elif defined(DIVERSALIS_OS_LINUX)

#include <dlfcn.h>

void library_init(Library* self)
{
	self->module = 0;
	self->err = 0;
}

void library_load(Library* self, const char* path)
{	
	self->module = dlopen(path, RTLD_LAZY /*RTLD_NOW*/);
	
	if (self->module == 0) {
		self->err = dlerror();
	}	
}

void* library_functionpointer(Library* self, const char* name)
{
	return dlsym(self->module, name);
}

void library_dispose(Library* self)
{
	if (self->module) {
		dlclose(self->module);
		self->module = 0;
	}
}			

#endif
