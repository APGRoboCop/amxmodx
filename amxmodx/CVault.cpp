// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#include <cctype>
#include <cstdlib>
#include <cstring>
#include "amxmodx.h"
#include "CVault.h"
#include "CFileSystem.h"

// *****************************************************
// class Vault
// *****************************************************

bool Vault::exists(const char* k)
{
	if (*k == 0) return false;

	return *find(k) != nullptr;
}

void Vault::put(const char* k, const char* v)
{
	if (*k == 0) return;

	if (*v == 0)
	{
		remove(k);
		return;
	}

	Obj** a = find(k);

	if (*a)
	{
		(*a)->value = v;
		(*a)->number = atoi(v);
	}
	else
		*a = new Obj(k, v);
}

Vault::Obj::Obj(const char* k, const char* v): key(k), value(v), next(nullptr)
{
	number = atoi(v);
}

Vault::Obj** Vault::find(const char* n)
{
	Obj** a = &head;

	while (*a)
	{
		if (strcmp((*a)->key.chars(), n) == 0)
			return a;

		a = &(*a)->next;
	}

	return a;
}


int Vault::get_number(const char* n)
{
	if (*n == 0) return 0;

	const Obj* b = *find(n);

	if (b == nullptr) return 0;

	return b->number;
}

const char* Vault::get(const char* n)
{
	if (*n == 0) return "";

	const Obj* b = *find(n);

	if (b == nullptr) return "";

	return b->value.chars();
}

void Vault::clear()
{
	while (head)
	{
		Obj* a = head->next;
		delete head;
		head = a;
	}
}

void Vault::remove(const char* n)
{
	Obj** b = find(n);

	if (*b == nullptr) return;

	Obj* a = (*b)->next;
	delete *b;
	*b = a;
}

void Vault::setSource(const char* n)
{
	path = n;
}

bool Vault::loadVault()
{
	if (!path.length())
	{
		return false;
	}

	clear();

	FILE *fp = fopen(path.chars(), "r");

	if (!fp)
	{
		return false;
	}

	char lineRead[512];

	while (fgets(lineRead, sizeof(lineRead), fp))
	{
		char key[sizeof(lineRead) + 1];
		char value[sizeof(lineRead) + 1];
		UTIL_TrimLeft(lineRead);

		if (!*lineRead || *lineRead == ';')
		{
			continue;
		}

		sscanf(lineRead, "%s%*[ \t]%[^\n]", key, value);

		if (isalpha(*key))
		{
			put(key, value);
		}
	}

	fclose(fp);

	return true;

}

bool Vault::saveVault()
{
	if (!path.length())
	{
		return false;
	}

	FILE *fp = fopen(path.chars(), "w");

	if (!fp)
	{
		return false;
	}

	fputs("; Don't modify!\n", fp);

	for (const Obj* b = head; b; b = b->next)
	{
		fprintf(fp, "%s\t%s\n", b->key.chars(), b->value.chars());
	}

	fclose(fp);

	return true;
}
