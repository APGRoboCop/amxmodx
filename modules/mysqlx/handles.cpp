// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// MySQL Module
//

#include <cstring>
#include <sh_stack.h>
#include <amtl/am-vector.h>
#include "mysql2_header.h"

struct QHandle
{
	void *_ptr;
	FREEHANDLE _func;
	HandleType type;
	bool isfree;
};

ke::Vector<QHandle *> g_Handles;
CStack<unsigned int> g_FreeHandles;

unsigned int MakeHandle(void *ptr, HandleType type, FREEHANDLE f)
{
	unsigned int num;
	QHandle *h;

	if (g_FreeHandles.size())
	{
		num = g_FreeHandles.front();
		g_FreeHandles.pop();
		h = g_Handles[num];
	} else {
		h = new QHandle;
		g_Handles.append(h);
		num = static_cast<unsigned int>(g_Handles.length()) - 1;
	}

	h->_ptr = ptr;
	h->type = type;
	h->_func = f;
	h->isfree = false;

	return num + 1;
}

void *GetHandle(unsigned int num, HandleType type)
{
	if (num == 0)
		return nullptr;

	num--;
	if (num >= g_Handles.length())
		return nullptr;

	const QHandle *h = g_Handles[num];
	if (h->isfree || (h->type != type))
		return nullptr;

	return h->_ptr;
}

bool FreeHandle(unsigned int num)
{
	if (num == 0)
		return false;

	const unsigned int _num = num;

	num--;
	if (num >= g_Handles.length())
		return false;

	QHandle *h = g_Handles[num];
	if (h->isfree)
		return false;

	h->_func(h->_ptr, _num);
	h->_ptr = nullptr;
	h->_func = nullptr;
	h->isfree = true;
	
	g_FreeHandles.push(num);

	return true;
}

void FreeAllHandles(HandleType type)
{
	QHandle *q;
	for (size_t i = 0; i < g_Handles.length(); i++)
	{
		q = g_Handles[i];
		if (q && !q->isfree && q->type == type)
		{
			FreeHandle((unsigned int)i);
		}
	}
}

void FreeHandleTable()
{
	QHandle *q;
	for (size_t i = 0; i < g_Handles.length(); i++)
	{
		q = g_Handles[i];
		if (q && !q->isfree)
			FreeHandle((unsigned int)i);
	}
	g_Handles.clear();
	while (!g_FreeHandles.empty())
		g_FreeHandles.pop();
}
