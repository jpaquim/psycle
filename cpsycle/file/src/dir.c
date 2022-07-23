/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "../../detail/os.h"
#include "../../detail/compiler.h"

#include "dir.h"
/* std */
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#if defined(DIVERSALIS__OS__MICROSOFT)
/*
** Windows does not define the S_ISREGand S_ISDIR macros in stat.h, so we do.
** We have to define _CRT_INTERNAL_NONSTDC_NAMES 1 before #including sys/stat.h
** in order for Microsoft's stat.h to define names like S_IFMT, S_IFREG, and S_IFDIR,
** rather than just defining  _S_IFMT, _S_IFREG, and _S_IFDIR as it normally does.
*/
#define _CRT_INTERNAL_NONSTDC_NAMES 1
#include <sys/stat.h>
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#endif

/* platform */
#include "../../detail/portable.h"

static void psy_path_extract_path(psy_Path*);
static void psy_path_update(psy_Path*);

#if defined(DIVERSALIS__OS__MICROSOFT)
#include <windows.h> /* MAX_PATH */
#endif

uintptr_t psy_path_max(void)
{
#if defined(DIVERSALIS__OS__MICROSOFT)
	return _MAX_PATH;
#else
	return 4096;
#endif
}

void psy_path_init(psy_Path* self, const char* path)
{
	self->path = strdup(path ? path : "");
	self->name = strdup("");
	self->prefix = strdup("");
	self->ext = strdup("");
	self->filename = strdup("");
	psy_path_extract_path(self);
}

void psy_path_init_all(psy_Path* self, const char* prefix, const char* name,
	const char* ext)
{
	assert(self);
		
	self->path = strdup("");
	self->filename = strdup("");
	self->prefix = strdup("");
	self->name = strdup(name);	
	self->ext = strdup(ext);	
	psy_path_update(self);
}

void psy_path_dispose(psy_Path* self)
{
	free(self->path);
	free(self->name);
	free(self->prefix);
	free(self->ext);
	free(self->filename);
}

void psy_path_setpath(psy_Path* self, const char* path)
{
	free(self->path);
	free(self->name);
	free(self->prefix);
	free(self->ext);
	free(self->filename);
	self->path = strdup(path ? path : "");
	self->name = strdup("");
	self->prefix = strdup("");
	self->ext = strdup("");
	self->filename = strdup("");
	psy_path_extract_path(self);
}

void psy_path_set_name(psy_Path* self, const char* name)
{
	free(self->name);
	self->name = strdup((name) ? name : "");	
	psy_path_update(self);
}

void psy_path_set_prefix(psy_Path* self, const char* prefix)
{
	psy_strreset(&self->prefix, prefix);	
	psy_path_update(self);
}

void psy_path_setext(psy_Path* self, const char* ext)
{
	free(self->ext);
	self->ext = strdup((ext) ? ext : "");
	psy_path_update(self);
}

void psy_path_append_dir(psy_Path* self, const char* dir)
{
	if (psy_strlen(dir) == 0) {
		return;
	}
	if (psy_strlen(self->prefix) == 0) {
		psy_strreset(&self->prefix, dir);
	} else if (psy_strlen(self->prefix) > 0 &&
			self->prefix[psy_strlen(self->prefix) - 1] == psy_SLASH) {
		self->prefix = psy_strcat_realloc(self->prefix, dir);
	} else {	
		if (dir[0] != psy_SLASH) {
			self->prefix = psy_strcat_realloc(self->prefix, psy_SLASHSTR);
		}
		self->prefix = psy_strcat_realloc(self->prefix, dir);
	}
	psy_path_update(self);
}

bool psy_path_remove_dir(psy_Path* self)
{
	const char* str;

	str = psy_path_full(self);
	if (str) {
		char* p;
		uintptr_t pos;

		pos = psy_INDEX_INVALID;
		p = strrchr(str, psy_SLASH);
		if (p) {
			pos = p - str;
		}								
		if (pos != psy_INDEX_INVALID) {
			char* parentdir;

			parentdir = (char*)malloc(pos + 2);
			psy_snprintf(parentdir, pos + 1, str);
			psy_path_dispose(self);
			psy_path_init(self, NULL);
			psy_path_append_dir(self, parentdir);
			free(parentdir);
			return TRUE;
		}							
	}
	return FALSE;
}

void psy_path_update(psy_Path* self)
{
	static const char* path_delim = psy_SLASHSTR;
	free(self->path);

	self->path = malloc(psy_strlen(self->prefix) + 1 + psy_strlen(self->name) +
		1 + psy_strlen(self->ext) + 1);
	self->path[0] = '\0';
	if (psy_path_hasprefix(self)) {
		strcat(self->path, self->prefix);
	}
	if (psy_path_hasname(self)) {
		if (psy_path_hasprefix(self)) {
			strcat(self->path, path_delim);
		}
		strcat(self->path, self->name);
	}
	if (psy_path_hasext(self)) {
		strcat(self->path, ".");
		strcat(self->path, self->ext);		
	}
}

bool psy_path_hasprefix(psy_Path* self)
{
	return (psy_strlen(self->prefix) != 0);
}

bool psy_path_hasext(psy_Path* self)
{
	return (psy_strlen(self->ext) != 0);	
}

bool psy_path_hasname(psy_Path* self)
{
	return (psy_strlen(self->name) != 0);	
}

void psy_path_extract_path(psy_Path* self)
{
	char* p;
	
	p = strrchr(self->path, psy_SLASH);
	if (p) {
		free(self->prefix);
		self->prefix = (char*)malloc(p - self->path + 1);
		self->prefix = strncpy(self->prefix, self->path, p - self->path);
		self->prefix[p - self->path] = '\0';
		free(self->name);
		self->name = strdup(p + 1);		
	} else {
		psy_strreset(&self->prefix, "");
		psy_strreset(&self->name, self->path);		
	}
	p = strrchr(self->name, '.');
	if (p) {			
		if (*(p + 1) == '\0' && (p == self->name)) {
			free(self->ext);
			self->ext = strdup("");
			free(self->name);
			self->name = strdup(".");
		} else if (*(p + 1) == '\0' && (*(p - 1) == '.' &&
				(p - 1) == self->name)) {
			free(self->ext);
			self->ext = strdup("");
			free(self->name);
			self->name = strdup("..");
		} else {
			free(self->ext);
			self->ext = malloc(psy_strlen(self->name) - (p - self->name) + 1);
			self->ext = strncpy(self->ext, p + 1, psy_strlen(self->name) -
				(p - self->name));
			self->name[p - self->name] = '\0';
		}
	} else {
		free(self->ext);
		self->ext = strdup("");
	}	
	psy_strreset(&self->filename, self->name);	
	if (psy_strlen(self->ext) != 0) {
		self->filename = psy_strcat_realloc(self->filename, ".");
		self->filename = psy_strcat_realloc(self->filename, self->ext);
	}
}

uintptr_t psy_file_size(const char* path)
{
	struct stat sb;

	assert(path);
	
	if (stat(path, &sb) == -1) {
		return psy_INDEX_INVALID;
	}
	return sb.st_size;
}

bool psy_file_is_directory(const psy_Path* p)
{
	struct stat sb;
	const char* filename;

	assert(p);

	filename = psy_path_full(p);
	if (stat(filename, &sb) == -1) {
		return psy_INDEX_INVALID;
	}
	return S_ISDIR(sb.st_mode) != 0;
}


#if defined DIVERSALIS__COMPILER__GNU || defined DIVERSALIS__OS__POSIX

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../../detail/portable.h"

static int wildcardmatch(const char *str, const char *match);

char* psy_workdir(char* buffer)
{
    return getcwd(buffer, 4096);
}

const char* pathenv(void)
{		
	return 0;
}

void psy_insertpathenv(const char* path)
{
 	 /* todo */
}

void setpathenv(const char* path)
{
 	 /* todo */
}

void psy_dir_enumerate(void* context, const char* root, const char* wildcard,
	int flag, psy_fp_findfile enumproc)
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
					psy_snprintf(path, 4096, "%s/%s", root,
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

int psy_dir_enumerate_recursive(void* context, const char* root, const char* wildcard, int flag,
	psy_fp_findfile enumproc)
{
	DIR *dir;
	struct dirent *dir_ptr;
	char path[4096];
	
	/* 
	** First, enumerate all files using the wildcard in the current
	** directory
	**/
	if ((dir=opendir(root)) == NULL) {
	   return 0;
    }    
	while((dir_ptr = readdir(dir)) != NULL) {
		if (wildcardmatch((*dir_ptr).d_name, wildcard)) {
			if ((strncmp(".", (*dir_ptr).d_name, 1) != 0) && 
					(strncmp("..", (*dir_ptr).d_name, 2) != 0)) {
				psy_snprintf(path, 4096, "%s/%s", root,
					(*dir_ptr).d_name);
				enumproc(context, path, flag);										   
			}				   
		}
    }
    if(closedir(dir) == -1) {
      return 0;
    }
    /* 
    ** Secondly, find and emumerate all subdirectories with their
    ** subdirectories
    */
    if ((dir=opendir(root)) == NULL) {
	   return 0;
    }        
	while((dir_ptr = readdir(dir)) != NULL) {				   
		if ((strncmp(".", (*dir_ptr).d_name, 1) != 0) && 
				(strncmp("..", (*dir_ptr).d_name, 2) != 0) &&
				dir_ptr->d_type == DT_DIR) {
			/* enumerate subdirectory with its subdirectories */
			psy_snprintf(path, 4096, "%s" psy_SLASHSTR "%s", root,
				(*dir_ptr).d_name);			
			if (psy_dir_enumerate_recursive(context, path, wildcard,
					flag, enumproc) == 0) {
				closedir(dir);				
				return 0;
			}									   
		 }       
    }
    if(closedir(dir) == -1) {
      return 0;
    }
	return 1;	
}

/*
** modified version from Dezhi Zao, codeproject
** a simple wildcard matching function
** modification: replaced TCHAR with char
*/
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

const char* psy_dir_config(void)
{
	const char *rv;
		
	if ((rv = getenv("XDG_CONFIG_HOME")) == NULL) {
		static char config[4096];
		
		if (psy_dir_home()) {
			psy_snprintf(config, 4096, "%s/.config", psy_dir_home());
			rv = config;
		}
	}	
	return rv;    
}

const char* psy_dir_home(void)
{
	const char *rv;
	
	if ((rv = getenv("HOME")) == NULL) {
		rv = getpwuid(getuid())->pw_dir;
	}	
	return rv;
}

psy_List* psy_drives(void)
{
	psy_List* rv;
	
	rv = NULL;
	psy_list_append(&rv, strdup("/"));
	/*if (psy_dir_home()) {
		char tmp[4096];
				
		psy_snprintf(tmp, 4096, "%s/", psy_dir_home());
		psy_list_append(&rv, strdup(tmp));
	}*/
	return rv;	
}

psy_List* psy_directories(const char* path)
{
	DIR *d;
	psy_List* rv = NULL;
	struct dirent *dir;

	d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {			
			if (dir->d_type == DT_DIR) {
				if (dir->d_name) {
					psy_list_append(&rv, strdup(dir->d_name));
				}
			}
		}
		closedir(d);
	}
	return rv;
}

#elif defined(DIVERSALIS__OS__MICROSOFT)

#include <direct.h>
#include <windows.h>
#include "Shlobj.h"
#include "../../detail/portable.h"

static const char pathenvvarname[] = { 
	"PATH"
};

void psy_dir_enumerate(void* context, const char* root, const char* wildcard, int flag,
	psy_fp_findfile enumproc)
{
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	char path[MAX_PATH];	
	BOOL cont;
  
	/* First, enumerate all files using the wildcard in the current directory */
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
	/*
	** Secondly, find and emumerate all subdirectories with their subdirectories
	*/
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

int psy_dir_enumerate_recursive(void* context, const char* root, const char* wildcard, int flag,
	psy_fp_findfile enumproc)
{
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	char path[MAX_PATH];	
	BOOL cont;
  
	/*
	** First, enumerate all files using the wildcard in the current directory
	*/
	psy_snprintf(path, MAX_PATH, "%s\\%s", root, wildcard);
 	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE) {		
		
	} else {
		cont = TRUE;
		do {
			if ((strncmp(".", wfd.cFileName, 1) != 0) && 
					(strncmp("..", wfd.cFileName, 2) != 0) ) {
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {						
					psy_snprintf(path, MAX_PATH, "%s\\%s", root, wfd.cFileName);				
					if (enumproc(context, path, flag) == 0) {
						if (FindClose(hFind) == FALSE) {
							SetLastError(0);							
						}
						return 0;
					}
				}
			}		
		} while (FindNextFile(hFind, &wfd));
		if (GetLastError() != ERROR_NO_MORE_FILES) {
			SetLastError(0);		   				   
			return 1;
		}
		if (FindClose(hFind) == FALSE) {
			SetLastError(0);		   					 
			return 1;
		}
	}
	/*
	** Secondly, find and emumerate all subdirectories with their subdirectories
	*/
	psy_snprintf(path, MAX_PATH, "%s\\*", root);
	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE) {		
		SetLastError(0);	   		   
		return 1;
	}	
	do {
		if ((strncmp(".", wfd.cFileName, 1) != 0) && 
				(strncmp("..", wfd.cFileName, 2) != 0) ) {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				/* enumerate subdirectory with its subdirectories */
				psy_snprintf(path, MAX_PATH, "%s\\%s", root, wfd.cFileName);				
				if (psy_dir_enumerate_recursive(context, path, wildcard, flag,
						enumproc) == 0) {
					if (FindClose(hFind) == FALSE) {
						SetLastError(0);						
					}					
					return 0;
				}
			}			
		}		
	} while (FindNextFile(hFind, &wfd));
	if (GetLastError() != ERROR_NO_MORE_FILES) {
		SetLastError(0);
		return 1;
	}
	if (FindClose(hFind) == FALSE) {
	 	SetLastError(0);
		return 1;
	}
	SetLastError(0);
	return 1;
}

char* psy_workdir(char* buffer)
{
	return _getcwd(buffer, _MAX_PATH);
}

const char* pathenv(void)
{		
	return getenv(pathenvvarname);	
}

void psy_insertpathenv(const char* path)
{	
	const char* envpath;		
	
	envpath = pathenv();
	if (envpath && path) {			
		char* newenvpath;
		size_t size;

		size = psy_strlen(path) + psy_strlen(envpath) + 3;
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

		size = psy_strlen(path) + psy_strlen(pathenvvarname) + 3;
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

const char* psy_dir_config(void)
{
	static TCHAR achDevice[MAX_PATH];	
#if WINVER >= 0x600
	HRESULT  hr;
	/*
	** include file ShlObj.h contains list of CSIDL defines however only a subset
	** are supported with Windows 7 and later.
	** for the 3rd argument, hToken, can be a specified Access Token or SSID for
	** a user other than the current user. Using NULL gives us the current user.
	*/

	if (SUCCEEDED(hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, achDevice))) {
		/*
		** append a folder name to the user's Documents directory.
		** the Path Handling functions are pretty handy.
		** PathAppend(achDevice, L"xxx");
		*/
	}
#else	
	strcpy(achDevice, PSYCLE_APP_DIR);
#endif
	return achDevice;
}

const char* psy_dir_home(void)
{
	static TCHAR achDevice[MAX_PATH];
#if WINVER >= 0x600
	HRESULT  hr;
	/*
	** include file ShlObj.h contains list of CSIDL defines however only a
	** subset are supported with Windows 7 and later.
	** for the 3rd argument, hToken, can be a specified Access Token or SSID for
	** a user other than the current user. Using NULL gives us the current user.
	*/

	if (SUCCEEDED(hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0,
		achDevice))) {
		/*
		** append a folder name to the user's Documents directory.
		** the Path Handling functions are pretty handy.
		** PathAppend(achDevice, L"xxx");
		*/
	}
#else	
	strcpy(achDevice, PSYCLE_APP_DIR);
#endif
	return achDevice;
}

psy_List* psy_drives(void)
{
	psy_List* rv = NULL;
	DWORD dwSize = MAX_PATH;
	char szLogicalDrives[MAX_PATH] = { 0 };
	DWORD dwResult = GetLogicalDriveStrings(dwSize, szLogicalDrives);

	if (dwResult > 0 && dwResult <= MAX_PATH) {
		char* szSingleDrive = szLogicalDrives;
		while (*szSingleDrive) {
			char* drive;
			char* slash;			

			drive = strdup(szSingleDrive);
			slash = strrchr(drive, psy_SLASH);
			if (slash) {
				uintptr_t pos;

				pos = slash - drive;
				drive[pos] = '\0';
			}
			psy_list_append(&rv, drive);
			szSingleDrive += psy_strlen(szSingleDrive) + 1;			
		}
	}
	return rv;
}

psy_List* psy_directories(const char* root)
{
	psy_List* rv = NULL;
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	char path[MAX_PATH];
	BOOL cont;

	/* First, enumerate all files using the wildcard in the current directory */
	psy_snprintf(path, MAX_PATH, "%s\\*", root);
	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE) {

	} else {
		cont = TRUE;
		do {
			/* if ((strncmp(".", wfd.cFileName, 1) != 0) &&
				  (strncmp("..", wfd.cFileName, 2) != 0)) { */
				if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					psy_snprintf(path, MAX_PATH, "\\%s", wfd.cFileName);
					psy_list_append(&rv, strdup(path));
				}
			/* } */
		} while (FindNextFile(hFind, &wfd));
		if (GetLastError() != ERROR_NO_MORE_FILES) {
			SetLastError(0);
		}
		if (FindClose(hFind) == FALSE) {
			SetLastError(0);
		}
	}
	return rv;
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

char* psy_workdir(char* buffer)
{
	return 0;
}

const char* pathenv(void)
{		
	return 0;
}

void psy_insertpathenv(const char* path)
{		
}

void setpathenv(const char* path)
{
}

psy_List* psy_drives(void)
{
	return NULL;
}

psy_List* psy_directories(const char* path)
{
	psy_List* rv = NULL;

	return NULL;
}

#endif

struct FileSearch {
	char* filepath;
};

static int onenumfindfile(struct FileSearch* self, const char* path, int flag);

void psy_dir_findfile(const char* searchpath, const char* wildcard,
	char* filepath)
{
	filepath[0] = '\0';
	if (psy_strlen(wildcard) > 0) {
		struct FileSearch filesearch;
		
		filesearch.filepath = filepath;
		psy_dir_enumerate_recursive(&filesearch, searchpath, wildcard, 0,
			(psy_fp_findfile)onenumfindfile);
	}
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
		ext = strncpy(ext, p + 1, psy_strlen(name) - (p - name));
		name[p - name] = '\0';
	} else {
		ext = '\0';
	}
}

void psy_mkdir(const char* path)
{
#if defined _MSC_VER
	_mkdir(path);
#elif defined __GNUC__
	mkdir(path, 0777);
#endif
}

bool psy_direxists(const char* path)
{
#if defined _MSC_VER
	DWORD dwAttrib = GetFileAttributes(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#elif defined __GNUC__
	DIR* dir = opendir(path);
	if (dir) {
		/* Directory exists. */
		closedir(dir);
		return TRUE;
	} else if (ENOENT == errno) {
		/* Directory does not exist. */
		return FALSE;
	} else {
		/* opendir() failed for some other reason. */
		return FALSE;
	}
#endif
}

bool psy_filereadable(const char* fname)
{
	FILE* file;
	if ((file = fopen(fname, "r")))
	{
		fclose(file);
		return TRUE;
	}
	return FALSE;
}

static int on_enum_files(psy_List** rv, const char* path, int flag);

psy_List* psy_files(const char* path, const char* wildcard, int recursive)
{
	psy_List* rv;

	rv = NULL;
	if (recursive) {
		psy_dir_enumerate_recursive(&rv, path, wildcard, 0,
			(psy_fp_findfile)on_enum_files);
	} else {
		psy_dir_enumerate(&rv, path, wildcard, 0,
			(psy_fp_findfile)on_enum_files);
	}
	return rv;
}

int on_enum_files(psy_List** rv, const char* path, int flag)
{
	psy_list_append(rv, psy_strdup(path));	
	return 1;
}
