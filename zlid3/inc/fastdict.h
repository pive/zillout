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

#include <stdint.h>

typedef void (*FASTDICT_EACH_CALLBACK)(const void*, size_t, void*, void*);

struct _Bucket_t;

class ZLFastDict
{
private:

	ZLFastHeap* pDictHeap;

	uint32_t uLength;
	uint32_t uHashBits;
	uint32_t uCount;

	struct _Bucket_t** pBuckets;

public:
	ZLFastDict(uint32_t);
	~ZLFastDict();

	void  add(const void* pIndex, size_t len, void* pData);
	void  add(const char* pszIndex, void* pData) { this->add((const void*)pszIndex, strlen(pszIndex), pData); };
	void* get(const void* pIndex, size_t len);
	void* get(const char* pszIndex) { return this->get((const void*)pszIndex, strlen(pszIndex)+1); };
	bool  exists(const void* pIndex, size_t len);
	bool  exists(const char* pszIndex) { return this->exists((const void*)pszIndex, strlen(pszIndex)); };
	void  each(FASTDICT_EACH_CALLBACK fpEach, void* pUserData=NULL);
	void  dump();

	uint32_t count()  { return this->uCount; };
	uint32_t length() { return this->uLength; };
	size_t size();
};
