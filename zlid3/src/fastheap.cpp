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

#include <stdlib.h>
#include <string.h>

#include "../inc/fastheap.h"


/* Quick memory management structures
-------------------------------------------------------------------------------
*/

struct _MemPage_t
{
	size_t nMemUsed;
	void* pMemChunk;
	struct _MemPage_t* pNextChunk;
};
typedef struct _MemPage_t MemPage_t;

struct _FastHeap_t
{
	MemPage_t* pHead;
	MemPage_t* pTail;
	size_t nMemPage;
	size_t nMemUsed;
	size_t nMemSize;
	void* pLastCopy;
};

/* Quick memory management low level routines
-------------------------------------------------------------------------------
*/

MemPage_t* _new_chunk(size_t nChunkSize)
{
	MemPage_t* out = (MemPage_t*)malloc(sizeof(MemPage_t));
	if (out != NULL)
	{
		out->nMemUsed = 0;
		out->pNextChunk = NULL;
		out->pMemChunk = malloc(nChunkSize);
		if (out->pMemChunk == NULL)
		{
			free(out);
			out = NULL;
		}
	}
	return out;
}

void _free_chunk(MemPage_t* pChunk)
{
	if (pChunk->pNextChunk != NULL)
	{
		_free_chunk(pChunk->pNextChunk);
	}
	free(pChunk->pMemChunk);
	free(pChunk);
}

/* Quick memory management C API
-------------------------------------------------------------------------------
*/

FastHeap_t* zl_fast_heap_new(size_t nPageSize)
{
	FastHeap_t* out = (FastHeap_t*)malloc(sizeof(FastHeap_t));
	if (out != NULL)
	{
		out->pHead = NULL;
		out->pTail = NULL;
		out->nMemPage = nPageSize;
		out->nMemUsed = 0;
		out->nMemSize = 0;
		out->pLastCopy = NULL;
	}
	return out;
}

void* zl_fast_heap_alloc(FastHeap_t* pHeap, size_t nDesired)
{
	void* out = NULL;

	if (nDesired > pHeap->nMemPage)
	{
		return NULL;
	}

	if (pHeap->pHead == NULL)
	{
		pHeap->pHead = _new_chunk(pHeap->nMemPage);
		pHeap->pTail = pHeap->pHead;
		pHeap->nMemSize += pHeap->nMemPage;
	}
	else if ( (pHeap->pTail->nMemUsed + nDesired) > pHeap->nMemPage )
	{
		pHeap->pTail->pNextChunk = _new_chunk(pHeap->nMemPage);
		pHeap->pTail = pHeap->pTail->pNextChunk;
		pHeap->nMemSize += pHeap->nMemPage;
	}

	if (pHeap->pTail != NULL)
	{
		out = (unsigned char*)pHeap->pTail->pMemChunk + pHeap->pTail->nMemUsed;
		pHeap->pTail->nMemUsed += nDesired;
		pHeap->nMemUsed += nDesired;
	}
	
	return out;
}

void* zl_fast_heap_dup(FastHeap_t* pHeap, const void* pSource, size_t nSize)
{
	if (pHeap->pLastCopy != pSource)
	{
		pHeap->pLastCopy = zl_fast_heap_alloc(pHeap, nSize);
		if (pHeap->pLastCopy != NULL)
		{
			memcpy(pHeap->pLastCopy, pSource, nSize);
		}
	}
	return pHeap->pLastCopy;
}

void zl_fast_heap_free(FastHeap_t* pHeap)
{
	if (pHeap != NULL)
	{
		if (pHeap->pHead != NULL)
		{
			_free_chunk(pHeap->pHead);
		}
		free(pHeap);
	}
}

/* Quick string memory management C++ API (wrapper around C API)
-------------------------------------------------------------------------------
*/

ZLFastHeap::ZLFastHeap(size_t nDesired)
{ 
	this->pHeap = zl_fast_heap_new(nDesired);
}

ZLFastHeap::~ZLFastHeap()
{ 
	zl_fast_heap_free(this->pHeap);
}

void* ZLFastHeap::alloc(size_t nDesired)
{
	return zl_fast_heap_alloc(this->pHeap, nDesired);
}

void* ZLFastHeap::duplicate(const void* pSource, size_t nSize)
{
	return zl_fast_heap_dup(this->pHeap, pSource, nSize);
}

size_t ZLFastHeap::size()
{
	return this->pHeap->nMemSize;
}


