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

struct _FastHeap_t;
typedef struct _FastHeap_t FastHeap_t;


/* Quick string memory management C++ API (wrapper around C API)
-------------------------------------------------------------------------------
*/

class ZLFastHeap
{
	private:
		FastHeap_t* pHeap;

	public:
		ZLFastHeap(size_t nDesired);
		~ZLFastHeap();

		void* alloc(size_t nDesired);
		void* duplicate(const void* pSource, size_t nSize);

		size_t size();
};
