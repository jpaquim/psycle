// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"
#include "../../detail/compiler.h"

#include "dir.h"
#include <stdio.h>
#include <string.h>

#if defined DIVERSALIS__COMPILER__GNU || DIVERSALIS__OS__POSIX

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../../detail/portable.h"

static int wildcardmatch(const char *str, const char *match);

char* workdir(char* buffer)
{
    return getcwd(buffer, 4096);
}

const char* pathenv(void)
{		
	return 0;
}

void insertpathenv(const char* path)
{
 	 // todo		
}

void setpathenv(const char* path)
{
 	 // todo
}

void psy_dir_enumerate(void* context, const char* root, const char* wildcard,
	 int flag, void (*enumproc)(void*, const char* path, int flag))
{
 	DIR *dir;
	struct dirent *dir_ptr;
	char path[4096];	
	
	if ((dir=opendir(root)) == NULL) {
	   return;
    }
	while((dir_ptr = readdir(dir)) != NULL) {
	   if (wildcardmatch((*dir_ptr).d_name, wildcard)) {
		  if ((strncmp(".", (*dir_ptr).d_name, 1) != 0) && 
 		  	 (strncmp("..", (*dir_ptr).d_name, 2) != 0)) {
					psy_snprintf(path, MAX_PATH, "%s\\%s", root,
		                (*dir_ptr).d_name);
					enumproc(context, path, flag);										   
          }				   
       }
    }
    if(closedir(dir) == -1) {
      return;
    }
	return;	
}

void psy_dir_enumerate_recursive(void* context, const char* root, const char* wildcard, int flag,
	void (*enumproc)(void*, const char* path, int flag))
{
 	// todo
}

// modified version from Dezhi Zao, codeproject
// a simple wildcard matching function
// modification: replaced TCHAR with char
int wildcardmatch(const char *pszString, const char *pszMatch)
{
	const char *mp;
	const char *cp = NULL;
 
	while (*pszString) 
	{
		if (*pszMatch == '*') 
		{
			if (!*++pszMatch) 
				return 1;
			mp = pszMatch;
			cp = pszString + 1;
		} 
		else if (*pszMatch == '?' || toupper(*pszMatch) == toupper(*pszString)) 
		{
			pszMatch++;
			pszString++;
		}
		else if (!cp)
			return 0;
		else 
		{
			pszMatch = mp;
			pszString = cp++;
		}
	}

	while (*pszMatch == '*')
		pszMatch++;

	return !*pszMatch;
}

#elif defined(DIVERSALIS__OS__MICROSOFT)

#include <direct.h>
#include <windows.h>
#include "../../detail/portable.h"

static const char pathenvvarname[] = { 
	"PATH"
};

void psy_dir_enumerate(void* context, const char* root, const char* wildcard, int flag,
	void (*enumproc)(void*, const char* path, int flag))
{
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	char path[MAX_PATH];	
	BOOL cont;
  
	// First, enumerate all files using the wildcard in the current directory
	psy_snprintf(path, MAX_PATH, "%s\\%s", root, wildcard);
 	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE) {		
		
	} else {
		cont = TRUE;
		do {
			if ((strncmp(".", wfd.cFileName, 1) != 0) && 
					(strncmp("..", wfd.cFileName, 2) != 0) ) {
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {						
					psy_snprintf(path, MAX_PATH, "%s\\%s", root, wfd.cFileName);				
					enumproc(context, path, flag);				
				}
			}		
		} while (FindNextFile(hFind, &wfd));
		if (GetLastError() != ERROR_NO_MORE_FILES) {
			SetLastError(0);		   				   
			return;
		}
		if (FindClose(hFind) == FALSE) {
			SetLastError(0);		   					 
			return;
		}
	}
	// Secondly, find and emumerate all subdirectories with their subdirectories
	psy_snprintf(path, MAX_PATH, "%s\\*", root);
	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE) {		
		SetLastError(0);	   		   
		return;
	}	
	if (GetLastError() != ERROR_NO_MORE_FILES) {
		SetLastError(0);
		return;
	}
	if (FindClose(hFind) == FALSE) {
		SetLastError(0);	   					 
		return;
	}
}

void psy_dir_enumerate_recursive(void* context, const char* root, const char* wildcard, int flag,
	void (*enumproc)(void*, const char* path, int flag))
{
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	char path[MAX_PATH];	
	BOOL cont;
  
	// First, enumerate all files using the wildcard in the current directory
	psy_snprintf(path, MAX_PATH, "%s\\%s", root, wildcard);
 	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE) {		
		
	} else {
		cont = TRUE;
		do {
			if ((strncmp(".", wfd.cFileName, 1) != 0) && 
					(strncmp("..", wfd.cFileName, 2) != 0) ) {
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {						
					psy_snprintf(path, MAX_PATH, "%s\\%s", root, wfd.cFileName);				
					enumproc(context, path, flag);				
				}
			}		
		} while (FindNextFile(hFind, &wfd));
		if (GetLastError() != ERROR_NO_MORE_FILES) {
			SetLastError(0);		   				   
			return;
		}
		if (FindClose(hFind) == FALSE) {
			SetLastError(0);		   					 
			return;
		}
	}
	// Secondly, find and emumerate all subdirectories with their subdirectories
	psy_snprintf(path, MAX_PATH, "%s\\*", root);
	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE) {		
		SetLastError(0);	   		   
		return;
	}	
	do {
		if ((strncmp(".", wfd.cFileName, 1) != 0) && 
				(strncmp("..", wfd.cFileName, 2) != 0) ) {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// enumerate subdirectory with its subdirectories
				psy_snprintf(path, MAX_PATH, "%s\\%s", root, wfd.cFileName);				
				psy_dir_enumerate_recursive(context, path, wildcard, flag, enumproc);
			}			
		}		
	} while (FindNextFile(hFind, &wfd));
	if (GetLastError() != ERROR_NO_MORE_FILES) {
		SetLastError(0);
		return;
	}
	if (FindClose(hFind) == FALSE) {
	 	SetLastError(0);
		return;
	}
	SetLastError(0);
}

char* workdir(char* buffer)
{
	return _getcwd(buffer, _MAX_PATH);
}

const char* pathenv(void)
{		
	return getenv(pathenvvarname);	
}

void insertpathenv(const char* path)
{	
	const char* envpath;		
	
	envpath = pathenv();
	if (envpath && path) {			
		char* newenvpath;
		size_t size;

		size = strlen(path) + strlen(envpath) + 3;
		newenvpath = (char*)malloc(size);
		newenvpath[0] = '\0';

		strcpy(newenvpath, path);
		strcat(newenvpath, ";");
		strcat(newenvpath, envpath);
		setpathenv(newenvpath);
		free(newenvpath);
	}	
}

void setpathenv(const char* path)
{
	if (path) {
		char* newenv;
		size_t size;

		size = strlen(path) + strlen(pathenvvarname) + 3;
		newenv = (char*)malloc(size);
		if (newenv) {
			newenv[0] = '\0';

			strcpy(newenv, pathenvvarname);
			strcat(newenv, "=");
			strcat(newenv, path);
			putenv(newenv);
			free(newenv);
		}
	}
}

#else

#include <stdio.h>

static const char pathenvvarname[] = { 
	"PATH"
};

void psy_dir_enumerate(void* context, const char* root, const char* wildcard, int flag,
	void (*enumproc)(void*, const char* path, int flag))
{
}

void psy_dir_enumerate_recursive(void* context, const char* root, const char* wildcard, int flag, void (*enumproc)(void*, const char* path, int flag))
{	
}

char* workdir(char* buffer)
{
	return 0;
}

const char* pathenv(void)
{		
	return 0;
}

void insertpathenv(const char* path)
{		
}

void setpathenv(const char* path)
{
}
#endif

struct FileSearch {
	char* filepath;
};

static int onenumfindfile(struct FileSearch* self, const char* path, int flag);

void psy_dir_findfile(const char* searchpath, const char* wildcard,
	char* filepath)
{
	struct FileSearch filesearch;

	filepath[0] = '\0';
	filesearch.filepath = filepath;
	psy_dir_enumerate_recursive(&filesearch, searchpath, wildcard, 0, onenumfindfile);
}

int onenumfindfile(struct FileSearch* self, const char* path, int flag)
{
	strcpy(self->filepath, path);
	return 1;
}

void psy_dir_extract_path(const char* path, char* prefix, char* name, char* ext)
{
	char* p;

	prefix[0] = '\0';
	p = strrchr(path, '\\');
	if (p) {
		prefix = strncpy(prefix, path, p - path);
		prefix[p - path] = '\0';
		name = strcpy(name, p + 1);
	} else {
		prefix[0] = '\0';
		name = strcpy(name, path);
	}
	p = strrchr(name, '.');
	if (p) {
		ext = strncpy(ext, p + 1, strlen(name) - (p - name));
		name[p - name] = '\0';
	} else {
		ext = '\0';
	}
}
