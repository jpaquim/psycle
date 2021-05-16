/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "library.h"
/* file */
#include <dir.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"


#if defined(DIVERSALIS__OS__MICROSOFT)

#include <windows.h>
#include <excpt.h>

void* module_fp(void* handle, const char* name)
{
	return GetProcAddress((HMODULE)handle, name);
}

void* module_load(const char* path, int* err)
{
	HMODULE module;

	module = LoadLibrary(path);
	*err = GetLastError();
	return module;
}

int module_release(void* handle)
{
	FreeLibrary((HMODULE)handle);
	return GetLastError();
}

#elif defined(DIVERSALIS__OS__APPLE)

#include <CoreFoundation/CoreFoundation.h>

void* module_fp(void* handle, const char* name)
{
	return (void*)CFBundleGetFunctionPointerForName(
		(CFBundleRef)handle, name);
}

void* module_load(const char* path, int* err)
{
	*err = 0;
	return CFBundleCreate(NULL, path);
}

int module_release(void* handle)
{
	CFBundleUnloadExecutable((CFBundleRef)handle);
	CFRelease((CFBundleRef)handle);
	return 0;
}

#elif defined(DIVERSALIS__OS__LINUX)
#include <dlfcn.h>

void* module_fp(void* handle, const char* name)
{
	return dlsym(handle, name);
}

void* module_load(const char* path, int* err)
{
	void* rv;

	rv = dlopen(path, RTLD_LAZY /*RTLD_NOW*/);
	if (rv == 0) {
		*err = (dlerror()) ? 1 : 0;
		perror("library load ");
	}
	return rv;
}

int module_release(void* handle)
{
	dlclose(handle);
	return 0;
}
#else
/* unknown platform */
void* module_fp(void* handle, const char* name) { return NULL; }
void* module_load(const char* path, int* err) { *err = 0; return NULL; }
int module_release(void* handle) { return 0; }
#endif

/* psy_Library */
void psy_library_init(psy_Library* self)
{
	self->module = NULL;
	self->err = 0;
	self->path = strdup("");
	self->env = NULL;
	self->root = NULL;
}

void psy_library_dispose(psy_Library* self)
{
	if (self->module) {
		self->err = module_release(self->module);
		self->module = NULL;
	}
	free(self->path);
	self->path = NULL;
	free(self->env);
	self->env = NULL;
	free(self->root);
	self->root = NULL;
}

psy_Library* psy_library_alloc(void)
{
	return (psy_Library*)malloc(sizeof(psy_Library));
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

void psy_library_unload(psy_Library* self)
{
	psy_library_dispose(self);
	psy_library_init(self);
}

void psy_library_load(psy_Library* self, const char* path)
{
	int err;

	err = 0;
	free(self->path);
	self->path = psy_strdup(path);
	self->module = module_load(path, &err);
	if (self->module == NULL) {			
		self->err = err;
	}
}

void* psy_library_functionpointer(psy_Library* self, const char* name)
{
	return module_fp(self->module, name);
}

void psy_library_setenv(psy_Library* self, const char* path, const char* root)
{
#if defined(DIVERSALIS__OS__MICROSOFT)
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
						/* error */
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
						/* error */
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
				/*  int const e(::GetLastError());
				  throw exceptions::library_errors::loading_error("Could not alter PATH env var.");
				  error */
			}
		}
	}
#endif
}

void psy_library_restoreenv(psy_Library* self)
{
#if defined(DIVERSALIS__OS__MICROSOFT)
	if (!SetEnvironmentVariable("PATH", self->env)) {
		/* error */
	}
	free(self->env);
	self->env = NULL;
#endif
}
