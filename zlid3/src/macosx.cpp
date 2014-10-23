/*
 ------------------------------------------------------------------------------
	zillout - a simple audio library manager for node.js

	Copyright (C) 2014 Pierre Veber
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
------------------------------------------------------------------------------
*/

#include "../inc/platform.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>


/*
 ------------------------------------------------------------------------------
	Trace utils

 ------------------------------------------------------------------------------
*/

void _zl_output_debug(const char* lpszString)
{
	printf("%s", lpszString);
}

/*
 ------------------------------------------------------------------------------
	Path utils

 ------------------------------------------------------------------------------
*/

int _path_exists(const char* path)
{
	struct stat info;

	if (stat(path, &info) == -1)
	{
		return (errno == ENOENT)?0:-1;
	}
	return 1;
}

int _path_is_dir(const char* path)
{
	struct stat info;

	if (stat(path, &info) == 0)
	{
		return S_ISDIR(info.st_mode)?1:0;
	}
	return -1;
}

const char* _path_find_filename(const char* pszPath)
{
	const char* p = strrchr(pszPath, '/');
	return (p==NULL)?pszPath:p+1;
}

char* _path_append(const char* pszBase, const char* pszAppend, MallocProc fpm, void* pMallocProcData)
{
	const char* pszFormat = "%s/%s";
	size_t more = 2;

	if (pszBase[strlen(pszBase)] == '/')
	{
		more = 1;
		pszFormat = "%s%s";
	}

	char* pszNew = (char*)fpm(pMallocProcData, strlen(pszBase) + strlen(pszAppend) + more);
	sprintf(pszNew, pszFormat, pszBase, pszAppend);

	return pszNew;
}

/*
 ------------------------------------------------------------------------------
	Walk into a directory recursively

 ------------------------------------------------------------------------------
*/

int _walk(const char* pszBase, void* pData, FoundFileProc fpf, MallocProc fpm)
{
	struct dirent* pDirent;
	int ret;

	DIR* pDir = opendir(pszBase);
	if (pDir == NULL)
	{
		return errno;
	}
	pDirent = readdir(pDir);
	while (pDirent != NULL)
	{
		if (pDirent->d_type == DT_DIR)
		{
			if (pDirent->d_name[0] != '.')
			{
				char* pszNewBase = (char*)fpm(pData, strlen(pszBase) + strlen(pDirent->d_name) + 2);
				sprintf(pszNewBase, "%s/%s", pszBase, pDirent->d_name);
				ret = _walk(pszNewBase, pData, fpf, fpm);
				if (ret != 0)
				{
					return ret;
				}
			}
		}
		else
		{
			ret = fpf(pData, pszBase, pDirent->d_name);
			if (ret != 0)
			{
				return ret;
			}
		}
		pDirent = readdir(pDir);
	}
	closedir(pDir);
	return 0;
}

