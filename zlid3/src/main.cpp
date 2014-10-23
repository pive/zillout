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

#include "../inc/precomp.h"

ZLFastHeap* pHeap;

int _on_file_found(void* pData, const char* pszBase, const char* pszFile)
{
	printf("Found %s under %s.\n", pszFile, pszBase);
	return 0;
}

void* _heap_alloc(void* pData, size_t desired)
{
	return pHeap->alloc(desired);
}

int main(void)
{
	/*pHeap = new ZLFastHeap(2048);
	int res = _walk("/Users/pveber/Documents/dev/zillout", NULL, _on_file_found, _heap_alloc);
	printf("Result: %d\n", res);
	delete pHeap;*/

	/*const char* ind1 = "zob";
	const char* ind2 = "cul";
	const char* ind3 = "vulve";
	const char* ind4 = "chatte";
	const char* ind5 = "tu pues du cul sale enculé";
	const char* ind6 = "chatte à ta mère";

	ZLFastDict* pDict = new ZLFastDict(4000);
	pDict->add(ind1, strlen(ind1), (void*)0x123);
	pDict->add(ind2, strlen(ind2), (void*)0x234);
	pDict->add(ind3, strlen(ind3), (void*)0x345);
	pDict->add(ind4, strlen(ind4), (void*)0x456);
	pDict->add(ind5, strlen(ind5), (void*)0x567);
	pDict->add(ind6, strlen(ind6), (void*)0x678);

	unsigned long uval = (unsigned long)pDict->get(ind6, strlen(ind6));
	printf("Value returned from get: %lx\n", uval);
	pDict->dump();
	delete pDict;*/

	ZLID3Library* pLib = new ZLID3Library("/Users/pveber/Music");
	//pLib->loadImages(true);
	pLib->load(NULL, NULL);

	pLib->dump();
	pLib->dumpImage("Wavering Radiant", "out.jpg");

	delete pLib;
}
