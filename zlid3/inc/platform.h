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

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define usleep(x)	Sleep((DWORD)(x/1000))
#include "win32/targetver.h"
#include <windows.h>
#else
#define _strdup strdup
#define _strndup strndup
#define _stricmp stricmp
#endif

#include <stddef.h>

/*
 ----------------------------------------------------------------------------
	Debug
 ----------------------------------------------------------------------------
*/
void _zl_output_debug(const char* lpszString);

/*
 ----------------------------------------------------------------------------
	Path utilities
 ----------------------------------------------------------------------------
*/

typedef void* (*MallocProc)(void*, size_t);

int _path_is_dir(const char* pszPath);
int _path_exists(const char* path);
const char* _path_find_filename(const char* pszPath);
const char* _path_file_extension(const char* pszPath);
char* _path_append(const char* pszBase, const char* pszAppend, MallocProc fpm, void* pData);

/* walk into a directory recursively */
typedef int (*FoundFileProc)(void*, const char*, const char*);
int _walk(const char* pszBase, void* pData, FoundFileProc fpf, MallocProc fpm);
