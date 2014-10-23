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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../inc/hash.h"
#include "../inc/fastheap.h"
#include "../inc/fastdict.h"

#define BUCKET_INDEX_CHAR_LENGTH 50

struct _Bucket_t {
	const void* pIndex;
	size_t sLength;
	void* pData;
	struct _Bucket_t* pNext;
};

bool eq(const void* p1, size_t len1, const void* p2, size_t len2)
{
	if (len1 == len2)
	{
		if (p1 == p2)
		{
			return true;
		}

		const unsigned char* c1 = (const unsigned char*)p1;
		const unsigned char* c2 = (const unsigned char*)p2;

		while (len1 > 0)
		{
			if (*c1 != *c2)
			{
				return false;
			}
			c1++;
			c2++;
			len1 -= sizeof(unsigned char);
		}
		return true;
	}
	return false;
}

ZLFastDict::ZLFastDict(uint32_t uLength)
{
	this->uHashBits = 32;
	while (   ((1 << (this->uHashBits-1)) & uLength) == 0   )
	{
		this->uHashBits--;
	}
    this->uLength  = hashsize(this->uHashBits);
	this->pBuckets = (_Bucket_t**)malloc(this->uLength*sizeof(_Bucket_t*));
    
	memset(this->pBuckets, 0, this->uLength*sizeof(_Bucket_t*));
	this->uCount = 0;

	this->pDictHeap = new ZLFastHeap(sizeof(_Bucket_t)*this->uLength);
}

ZLFastDict::~ZLFastDict()
{
	free(this->pBuckets);
	delete this->pDictHeap;
}

size_t ZLFastDict::size()
{
	return (this->uLength * sizeof(_Bucket_t*)) + this->pDictHeap->size();
}

void ZLFastDict::add(const void* pIndex, size_t len, void* pData)
{
	uint32_t uIndex = hashlittle(pIndex, len, 0x23);
	uIndex = (uIndex & hashmask(this->uHashBits));

	/* collision test 
	uIndex = 100; */

	if (this->pBuckets[uIndex] == NULL)
	{
		this->pBuckets[uIndex] = (_Bucket_t*)this->pDictHeap->alloc(sizeof(_Bucket_t));
		this->pBuckets[uIndex]->pIndex  = pIndex;
		this->pBuckets[uIndex]->sLength = len;
		this->pBuckets[uIndex]->pData   = pData;
		this->pBuckets[uIndex]->pNext   = NULL;
	}
	else
	{
		_Bucket_t* pBucket = this->pBuckets[uIndex];
		while (pBucket != NULL)
		{
			if (eq(pBucket->pIndex, pBucket->sLength, pIndex, len))
			{
				pBucket->pData = pData;
				break;
			}
			else
			{
				if (pBucket->pNext == NULL)
				{
					/* create new bucket */
					_Bucket_t* pNewBucket = (_Bucket_t*)this->pDictHeap->alloc(sizeof(_Bucket_t));
					pNewBucket->pIndex  = pIndex;
					pNewBucket->sLength = len;
					pNewBucket->pData   = pData;
					pNewBucket->pNext   = NULL;
					pBucket->pNext = pNewBucket;
					pBucket = pNewBucket;
				}
				pBucket = pBucket->pNext;
			}
		}
	}
}

void* ZLFastDict::get(const void* pIndex, size_t len)
{
	uint32_t uIndex = hashlittle(pIndex, len, 0x23);
	uIndex = (uIndex & hashmask(this->uHashBits));

	/* collision test 
	uIndex = 100; */

	_Bucket_t* pBucket = this->pBuckets[uIndex];
	while (pBucket != NULL)
	{
		if (eq(pBucket->pIndex, pBucket->sLength, pIndex, len))
		{
			return pBucket->pData;
		}
		pBucket = pBucket->pNext;
	}

	return NULL;
}

bool ZLFastDict::exists(const void* pIndex, size_t len)
{
	return (this->get(pIndex, len) != NULL);
}

void ZLFastDict::each(FASTDICT_EACH_CALLBACK fpEach, void* pUserData)
{
	uint32_t ui;

	for (ui = 0; ui < this->uLength; ui++)
	{
		_Bucket_t* pBucket = this->pBuckets[ui];
		while (pBucket != NULL)
		{
			fpEach(pBucket->pIndex, pBucket->sLength, pBucket->pData, pUserData);
			pBucket = pBucket->pNext;
		}
	}
}

void ZLFastDict::dump()
{
	uint32_t ui;

	printf("---FastDict dump:\n");

	for (ui = 0; ui < this->uLength; ui++)
	{
		_Bucket_t* pBucket = this->pBuckets[ui];
		while (pBucket != NULL)
		{
			char szBuf[(pBucket->sLength/sizeof(char)) + 1];
			memcpy(szBuf, pBucket->pIndex, pBucket->sLength);
			szBuf[pBucket->sLength/sizeof(char)] = '\0';
			printf("  Bucket [%s] = 0x%lx\n", szBuf, (unsigned long)pBucket->pData);
			pBucket = pBucket->pNext;
		}
	}
}

