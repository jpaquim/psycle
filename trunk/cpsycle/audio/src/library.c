// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "library.h"

#include <dir.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../detail/portable.h"


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
	self->module = NULL;
	self->err = 0;
	self->path = strdup("");
	self->env = NULL;
	self->root = NULL;
}

void psy_library_setenv(psy_Library* self, const char* path, const char* root)
{	
	char* temp;
	const char* p;
	char* env = NULL;	

	temp = self->root;
	self->root = strdup(root);
	free(temp);
	free(self->env);
	self->env = strdup(getenv("PATH"));
	p = strstr(path, root);
	if (p) {
		uintptr_t dest;

		p = path + psy_strlen(root);
		dest = 0;
		while (*p != '\0') {
			if (*p == '\\') {
				uintptr_t len;
				
				len = p - path;
				if (dest == 0) {
					env = malloc(len + 1);
					if (!env) {
						// error
						break;
					}
				} else {
					char* temp;
					temp = realloc(env, strlen(env) + len + 2);
					if (temp) {
						env = temp;
					} else {
						free(env);
						env = NULL;
						// error
						break;
					}
				}
				if (dest == 0) {
					psy_snprintf(env + dest, len + 1, "%s", path);
					dest += len;
				} else {
					psy_snprintf(env + dest, len + 2, ";%s", path);
					dest += len + 1;
				}
			}
			++p;
		}
		if (env != NULL) {
			if (!SetEnvironmentVariable("PATH", env)) {
				//int const e(::GetLastError());
				//throw exceptions::library_errors::loading_error("Could not alter PATH env var.");
				// error
			}			
		}
	}
}

void psy_library_restoreenv(psy_Library* self)
{
	if (!SetEnvironmentVariable("PATH", self->env)) {
		// error
	}
	free(self->env);
	self->env = NULL;
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
		self->module = NULL;
	}
	free(self->path);
	self->path = NULL;
	free(self->env);
	self->env = NULL;
	free(self->root);
	self->root = NULL;
}

#elif defined(DIVERSALIS__OS__APPLE)

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

void psy_library_unload(psy_Library* self)
{
	psy_library_dispose(self);
	psy_library_init(self);
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

void psy_library_setenv(psy_Library* self, const char* path, const char* root)
{
}

void psy_library_restoreenv(psy_Library* self)
{
}

#elif defined(DIVERSALIS__OS__LINUX)

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
		self->err = (dlerror())
			? 1
			: 0;
        perror("library load ");
	}	
}

void psy_library_unload(psy_Library* self)
{
	psy_library_dispose(self);
	psy_library_init(self);
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

void psy_library_setenv(psy_Library* self, const char* path, const char* root)
{
}

void psy_library_restoreenv(psy_Library* self)
{
}

#endif
