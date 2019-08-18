// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "dir.h"
#include <windows.h>
#include <stdio.h>

void dir_enum(void* context, const char* root, const char* wildcard, int flag, void (*enumproc)(void*, const char* path, int flag))
{
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	char path[MAX_PATH];	
	BOOL cont;
  
	_snprintf(path, MAX_PATH, "%s\\%s", root, wildcard);
  
 	
 	if ((hFind = FindFirstFile(path, &wfd)) == INVALID_HANDLE_VALUE)
	{		
		return;
	}
  
	cont = TRUE;
	while(cont == TRUE)
	{
		if ((strncmp(".", wfd.cFileName, 1) !=0) && (strncmp("..", wfd.cFileName, 2) != 0) )
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				_snprintf(path, MAX_PATH, "%s\\%s", root, wfd.cFileName);
				dir_enum(context, path, wildcard, flag, enumproc);
			}
			else
			{
				//do your work here -- mildly klugy comparison
				_snprintf(path, MAX_PATH, "%s\\%s", root, wfd.cFileName);				
				enumproc(context, path, flag);
			}
		}
		cont = FindNextFile(hFind, &wfd);
	}
	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		return;
	}
	if (FindClose(hFind) == FALSE)
	{
		return;
	}
}
