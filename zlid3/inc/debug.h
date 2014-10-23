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

#define	ZL_DBG_BUF_LEN	2048

inline void _zl_log_trace(const char* lpszFileName, unsigned line, const char* lpszMsg)
{
	char szComplete[ZL_DBG_BUF_LEN];
	sprintf(szComplete, "[%s:%u] %s\n", _path_find_filename(lpszFileName), line, lpszMsg);
	_zl_output_debug(szComplete);
}

#ifdef _DEBUG
#define TRACE(m)                 _wp_log_trace(__FILE__,__LINE__,m)
#define TRACE1(m,p1)             { char szMsg[ZL_DBG_BUF_LEN]; sprintf(szMsg, m, p1); _zl_log_trace(__FILE__, __LINE__, szMsg); }
#define TRACE2(m,p1,p2)          { char szMsg[ZL_DBG_BUF_LEN]; sprintf(szMsg, m, p1, p2); _zl_log_trace(__FILE__, __LINE__, szMsg); }
#define TRACE3(m,p1,p2,p3)       { char szMsg[ZL_DBG_BUF_LEN]; sprintf(szMsg, m, p1, p2, p3); _zl_log_trace(__FILE__, __LINE__, szMsg); }
#define TRACE4(m,p1,p2,p3,p4)    { char szMsg[ZL_DBG_BUF_LEN]; sprintf(szMsg, m, p1, p2, p3, p4); _zl_log_trace(__FILE__, __LINE__, szMsg); }
#else
#define TRACE(m)
#define TRACE1(m,p1)
#define TRACE2(m,p1,p2)
#define TRACE3(m,p1,p2,p3)
#define TRACE4(m,p1,p2,p3,p4)
#endif
