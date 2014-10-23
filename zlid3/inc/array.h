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


/* Quick array list implementation
-------------------------------------------------------------------------------
*/

template <class T> class ZLArrayList
{
private:
	T** _theArray;
	uint32_t _length;
	uint32_t _iterator;

public:
	ZLArrayList()
	{
		this->_theArray = NULL;
		this->_length = 0;
	}
	~ZLArrayList()
	{
		if (_theArray != NULL)
		{
			free(this->_theArray);
		}
	}
	void push(T* el)
	{
		this->_theArray = (T**)realloc(this->_theArray, sizeof(T*) * (this->_length + 1)); 
		this->_theArray[_length++] = el;
	}
	bool contains(T* el)
	{
		this->reset();
		T* cur = this->next();
		while (cur != NULL)
		{
			if (cur == el) 
			{
				return true;
			}
			cur = this->next();
		}
		return false;
	}

	void reset()
	{
		this->_iterator = 0;
	}
	
	T* next()
	{
		if (this->_iterator >= this->_length)
		{
			return NULL;
		}
		return this->_theArray[this->_iterator++];
	}
	uint32_t length()
	{
		return this->_length;
	}
	size_t size()
	{
		return sizeof(T*) * (this->_length);
	}
	size_t elemSize()
	{
		return sizeof(T*);
	}
};

